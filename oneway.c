#include <stdio.h>
#include "openssl/evp.h"
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>

// generates random alphanumeric string of length len
void gen_random(char *s, const int len)
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

char alphanum[] = "0123456789!\"#$%&'()*+-/. ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

#define SETLENGTH 77  

// check whether last string for length len
int check_last(char *str, int len)
{
	int i;
	char *last = malloc (len + 1);
	for(i = 0; i < len; i++)
	{
		last[i] = alphanum [SETLENGTH - 1];
	}
	last[len] = 0;
	if(!strcmp(str, last))
	    return 1;
	return 0;
}

// *ch will never be 'z'
void raise (char *ch)
{
	int i; 
	for(i = 0; i < SETLENGTH; i++)
	{
		if(alphanum[i] == *ch)
		{
			*ch = alphanum[i + 1];
	                 return;
		}
	}
}

// generate next string of length len int str
// return 0 ok, return 1 - end of string set for length len
int gen_str (int len, char *str)
{ 
	int i,j;
	static int alpha_index = 0;
	static int curr_index;
	static int raise_condition = 0;
	static int first_call = 1;
	static int count;

	// initial state
	if(first_call == 1)
	{
		first_call = 0;
		for(i = 0; i < len; i++)
		{
			str[i] = alphanum[0];
		}
		alpha_index = 1;
		curr_index = len-1;
		count = 0;
		return 0;
	}
	
	// prev call assigned 'z' to last index in string
	if(raise_condition == 1)
	{
		raise_condition = 0;
		
		for(i = curr_index-1 ; i >= 0 ; i--)
		{
			if (str[i] != alphanum[SETLENGTH-1])
				break;

		}
		if(i >= 0)  
		{
			raise(&str[i]);
			for(j = i+1 ; j < len; j++)
			{
				str[j] = alphanum [0];
			}
		}
	}

	// find next string
	str[curr_index] = alphanum[alpha_index];

	// loop char set index if required
	alpha_index++;

	if(alpha_index > (SETLENGTH - 1))
	{
		alpha_index = 0;
		raise_condition = 1;
	}

	// check whether the last string has been reached
	if(check_last(str, len) == 1)
	{
		alpha_index = 0;
		first_call = 1;
		raise_condition = 0;
		return 1;
	}
	return 0;
}

// compares first len bytes of two hash values hash1 and hash2
int match_digest (char *hash1, char *hash2, int len)
{

	for(int n = 0; n <len; n++)
	{
		if(hash1[n] != hash2[n])
		{
			return 0;
		}
	}
	return 1;

	/* Judge whether 'hash1' and 'hash2' are same or not. If match found, return '1' else '0'. Use the variables passed as parameters in the function*/ 

}

// generates the digest for the message mess using hash algorithm digest_method and saves the first len bytes in mess_hash
void generate_digest (char *mess, char *mess_hash, int len, char *digest_method)
{
	EVP_MD_CTX mdctx;
	const EVP_MD *md;
	
	unsigned char md_value [EVP_MAX_MD_SIZE];
	int md_len, i;
	OpenSSL_add_all_digests();
	
	md = EVP_get_digestbyname (digest_method);
	
	if (!md)
	{
		printf("Unknown message digest %s\n", digest_method);
		exit(1);
	}
	EVP_MD_CTX_init(&mdctx);
	EVP_DigestInit_ex(&mdctx, md, NULL);
	EVP_DigestUpdate(&mdctx, mess, strlen(mess));
	EVP_DigestFinal_ex(&mdctx, md_value, &md_len);
	EVP_MD_CTX_cleanup(&mdctx) ;

	for (i = 0; i < len; i++)
	{
		mess_hash[i] = md_value[i];
	}
}

int main (int argc, char *argv[])
{
	int i, count, match, c;
	char ch;
	int curr_length = 0, inc_len = 1;
	
	char *hash_algo = NULL;
	char *mess = NULL ;
	unsigned char mess_hash [3];
	char *rand_str = NULL;
	unsigned char rand_hash [3];

	// hash algorithm from command line argument
	hash_algo = argv[1]; 
	printf("\nBreak one way property\n");
	printf("\nGiven 24 bits H(M), find M' with same 24 bits H(M')\n");
	
	srand(time(NULL));
	// random string of length 6 will be generated, which requires a 7 byte allocation

	mess = malloc(7);

	//Use the generate random function you made
	gen_random(mess, 6);

	//generate hash for random string
	generate_digest(mess, mess_hash, 3, hash_algo);

	printf("\nSelecting random string as M: %s", mess);
	printf("\tDigest: ");
	for (i = 0; i < 3; i++)
	{
		printf("%02x ", mess_hash[i]);
	}

	printf("\n");

	count = 0;

	// now generate strings using brute force approach starting with string length 1

	while(1)
	{
		count++;

		if (inc_len == 1)
		{
			inc_len = 0;
			curr_length++;
		
			if (rand_str != NULL ) 
			{
				free(rand_str);
			}
			
			rand_str = malloc(curr_length + 1);
			rand_str[curr_length] = 0;
		}

		// get the next string of length curr_length
		inc_len = gen_str(curr_length, rand_str);

		// find hash of generated string
		generate_digest(rand_str, rand_hash, 3, hash_algo);

		//compare fir_st 3 bytes of the hash values
		match = match_digest (rand_hash, mess_hash, 3);

		if(match == 1)
		{
			printf("\n Match found !\n ");
                        break;
		}
	}
		
	printf ("\nString: %s has same first 24 bits hash value. \n", rand_str);
	printf ("\nIterations = %d\n\n", count);
	
	if(rand_str != NULL)
	{
		free(rand_str);
	}
	if(mess != NULL)
	{
		free(mess);
	}
}

