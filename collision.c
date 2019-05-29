#include <stdio.h>
#include "openssl/evp.h"
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>

//This section declares the functions used in the main program

// generates random alphanumeric string of length len
void gen_random (char *s, const int len)
{
	/* Define an array with all the provided characters */
	char alphanum[] = "0123456789!\"#$%&'()*+-/. ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

	int alphanumLen = sizeof(alphanum) -1;

	/* Generate random string using the characters of length 'len'*/

	for (int n = 0; n < len; n++)
	{
		//Limit random number to length of alphanum NOT GOOD PRACTICE
		int random = rand() % alphanumLen;

		//Set s character to that num in alphanum
		s[n] = alphanum[random];
		
	}
}

// compares first len bytes of two hash values hash1 and hash2
int match_digest (char *hash1, char *hash2, int len)
{

	/* Judge whether 'hash1' and 'hash2' are same or not. If match found, return '1' else '0'. Use the variables passed as parameters in the function*/ 

	for(int n = 0; n <len; n++)
	{
		if(hash1[n] != hash2[n])
		{
			return 0;
		}
	}
	return 1;

}

// generates the digest for the message mess using hash algorithm digest_method and
// saves the first len bytes in mess_hash
void generate_digest(char *mess, char *mess_hash, int len, char *digest_method)
{
	EVP_MD_CTX mdctx;
	const EVP_MD *md;
	
	unsigned char md_value [EVP_MAX_MD_SIZE];
	int md_len, i;
	OpenSSL_add_all_digests();
	
	md = EVP_get_digestbyname(digest_method);

	if(!md)
	{
		printf("Unkown message digest %s\n", digest_method);
		exit(1);
	}

	EVP_MD_CTX_init(&mdctx);
	EVP_DigestInit_ex(&mdctx ,md, NULL);
	EVP_DigestUpdate(&mdctx, mess, strlen(mess));
	EVP_DigestFinal_ex(&mdctx, md_value, &md_len);
	EVP_MD_CTX_cleanup(&mdctx);

	for(i = 0; i < len; i++)
	{
		mess_hash[i] = md_value[i];
	}
}

void save_to_repo(unsigned char repo[] [3], char repo_msg[] [7], int *repo_index, char *msg, unsigned char * hash)
{
	// increment repository index
	*repo_index = (*repo_index) + 1;
	
	// save hash
	repo[*repo_index][0] = hash[0];
	repo[*repo_index][1] = hash[1];
	repo[*repo_index][2] = hash[2];

	// save message
	strcpy(repo_msg[*repo_index], msg);
}

// returns -1 for no match
// returns index if match

int match_repo (unsigned char repo[] [3], const int repo_index , unsigned char *hash)
{
	int i;
	
	for(i = 0; i <= repo_index; i++)
	{
		if((repo[i][0] == hash[0]) && (repo[i][1] == hash[1]) && (repo[i][2] == hash[2]))
		{
			return i;
		}
	}
	return -1;
}

int main(int argc, char *argv[])
{
	int i, match, c, length, j;
	char ch;
	int curr_length = 0, inc_len=1;

	// hash algorithm
	char *hash_algo = argv[1];

	char *mess = NULL;
	unsigned char mess_hash[3];

	char *rand_str = NULL;
	unsigned char rand_hash[3];

	//Approach used in program
	//initial text - initial hash
	//generate text - hash compare with initial hash and hash repository -> true exit
	//false -> save text and hash

	// generated 24 bit hash values will be added to this repository
	unsigned char repo[10000][3];

	// index to keep track of number of hash values in repository
	int repo_index = -1;

	// message correposding to each hash value in repo is kept here
	char repo_msg[10000][7];

	srand(time(NULL));

	printf("\nBreak collision free property\n");
	printf("\nFind M1 and M2 s.t. 24 bits H(M1) = 24 bits H(M2) \n");

	//generate random string of random length
	length = rand() %20 + 1; // length varies 1-20
	mess = malloc(length + 1);
	gen_random (mess, length);//Stores the generated string in mess

	// calculate digest for mess in mess hash
	generate_digest(mess, mess_hash, 3, hash_algo );// a 24 bit (3 byte)hash is generated here and stored in variable mess_hash using a particular hash algrithm hash_algo

	printf("\n");

	rand_str = malloc(7);

	for(j = 0; j < 10000; j++)
	{

		// generate random string
		gen_random(rand_str, 6);

		// generate digest for random string
		generate_digest(rand_str, rand_hash, 3, hash_algo);

		//compare first 3 bytes of the hash values
		match = match_digest(rand_hash, mess_hash, 3);

		if(match == 1) //check the match found. 
		{

			/* When match found print the two strings i.e. the initial string 
			and the randomly generated string which have the same hash values*/
			printf ("\nMATCH IN GENERATED\n");

			printf ("\nInitial String: %s has same first 24 bits hash value as the Random String: %s \n", mess, rand_str);

			/* Print the hash value of the randomly generated string in hexadecimal format */
			printf ("The Matching Hash text is: ");

			for(int n = 0; n < 3; n++) 
			{
				printf("%x", rand_hash[n]);
			}
			printf ("\n\n");
			
			/* Print the number of iterations used to find the match. */
			printf ("\nIterations = %d\n\n", j);
			break;
		}
		match = -1, // match_repo may return actual index
		match = match_repo (repo, repo_index, rand_hash);// This statement compares the random String's hash with the repository storing all the generated strings' hash 

//If the random string's hash matches with the ones in the repository, match becomes positive
		if(match >= 0)
		{


			/* When match found from the repositary, print the two strings i.e. the random string and the repositary string which have the same hash values*/
			printf ("\nMATCH IN REPOSITORY\n");
			printf ("\nRepository String: %s has same first 24 bits hash value as the Random String: %s \n\n", repo_msg[match], rand_str);

			printf ("The Matching Hash text is: ");

			for(int n = 0; n < 3; n++) 
			{
				printf("%x", rand_hash[n]);
			}
			printf ("\n\n");

			
			/* Print the number of iterations used to find the match. */

			printf ("\nIterations = %d\n\n", j);
			break;
		}
		else
		{
		save_to_repo(repo, repo_msg, &repo_index, rand_str, rand_hash);
		}
	}
	if(rand_str != NULL)
	{
		free(rand_str);
	}
	if(mess != NULL)
	{
		free(mess);
	}
	printf("\n\n");
}
