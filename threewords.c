
/* 
   Copyright (C) 2018 Steve Murphy, Cody, WY
   All Rights Reserved.

   Permission to use this code via GNU GPLv2
   No warranties!

*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>

#include "hashtab.h"
#define DICT "/usr/share/dict/words"

struct ast_hashtab *dict;

char *dict_file_buffer;
int dict_file_size;

/* since the object in the hash table will be just a string pointer,
   we don't need to get fancy with the hash table representation.
   let's just suck in the entire dict file, and replace the linefeeds 
   with a null as we go thru it, entering each word into the table.
*/

/* give me the jumbled letters of two words, and give me the length of the longest word.
I'll see if I can find all the words that long that use those characters. */

void read_dict(void)
{
	struct stat dict_stat;
	FILE *dict_file;
	char *cptr, *ncptr;

	/* since the hashtab package provides all the funcs we'll need to implement this common and simple
	   sort of hash table, let's use them... */
	dict = ast_hashtab_create(600000, ast_hashtab_compare_strings, ast_hashtab_resize_none, ast_hashtab_newsize_none, ast_hashtab_hash_string, 0);

	/* get the size of the dict file, allocate that much mem, read it in */
	stat(DICT, &dict_stat);
	dict_file_size = dict_stat.st_size;
	dict_file_buffer = malloc(dict_file_size+1);
	dict_file = fopen(DICT,"r");
	if( !dict_file )
	{
		printf("uh-oh! can't open the dictionary (%s) for reading. Check out the proper path and perhaps fix and recompile this source\n", DICT);
		exit(13);
	}
	fread(dict_file_buffer, 1, dict_file_size, dict_file);
	fclose(dict_file); /* we are now done with this file pointer */
	
	/* now, people, we go thru the dictionary, and turn each linefeed into a null, and enter each word into the 
	   hash table */

	cptr = dict_file_buffer; /* start it out */
	
	while( (ncptr=strchr(cptr,'\n')) )
	{
		*ncptr = 0;
		if (!ast_hashtab_insert_safe(dict, cptr))
			printf("Could not insert %s into the hash table; ignoring it.\n", cptr);
		cptr = ncptr+1; /* next line! */
	}
	printf("Entered %d entries from the dictionary into the hash table.\n", ast_hashtab_size(dict));
}


void copy_set(int *from, int* to)
{
	int i;
	for(i=0;i<20;i++)
	{
		to[i] = from[i];
	}
}

int in_set(int *set, int ind, int val)
{
	int i,j;
	for(i=0;i<ind;i++)
	{
		if( set[i] == val )
			return 1;
	}
	return 0;
}

int match_count = 0;

int permute( int *set, int len, char *vals, int lev, int *currset, int print_it)
{
	int i;
	int retval=0;
	
	if( lev == len )
	{
		if(print_it)
		{
			/* print out the set */
			for(i=0;i<len;i++)
			{
				printf("%c", vals[currset[i]]);
			}
			printf("\n");
		}
		else
		{
			char buf1[20];
			for(i=0;i<len;i++)
			{
				buf1[i] = vals[currset[i]];
			}
			buf1[i] = 0;
			
			if( ast_hashtab_lookup(dict, buf1) )
			{
				match_count++;
				printf("   Found:    %s\n", buf1);
				retval=1;
			}
		}
		return retval;
	}
	for(i=0;i<len;i++)
	{
		if( !in_set(currset, lev, i) )
		{
			currset[lev] = i;
			permute( set, len, vals, lev+1, currset, print_it );
		}
	}
	return retval;
}

void rmcharfromset(char *vals, char x) {
	char *p = strchr(vals,x);
	if(!p) {
		printf("What the--- ?? How do remove %c from %s\n", x, vals);
		return;
	} else {
		while(*p) {
			*p = *(p+1);
			p++;
		}
		*p = 0;
	}
}

int twowords(char *vals, int len)
{
	int set[20], currset[20];
	struct ast_hashtab_iter *it;
	char *sptr;
	int i;
	int matched =0;
	int unmatched = 0;
	int wronglen = 0;

	/* the initial set */
	for(i=0;i<len;i++)
	{
		set[i] = i;
	}

	it = ast_hashtab_start_traversal(dict);
        while( (sptr = ast_hashtab_next(it)) ) {
		char buf3[40];
		char *p;
		int tlen = strlen(sptr);
		int i;
		if(tlen != len) {
			wronglen++;
			continue; // skip all the words not exactly "len" chars long!
		}
		strcpy(buf3, vals);
		for(i=0; i<tlen; i++) {
			char y;
			y=sptr[i];
			p = strchr(buf3,y);
			if(p) {
				rmcharfromset(buf3,y);
			} else {
				break; // this ain't the word
			}

		}
		if( p ) {
			char buf4[200];
			int len7;
			int pret;
			printf("    Match: Word=%s remainder=%s\n", sptr, buf3);
			/* the initial set */
			len7 = strlen(buf3);
			for(i=0;i<len7;i++)
			{
				set[i] = i;
			}
			pret = permute(set, len7, buf3, 0, currset, 0);
			if (pret) {
			}
			matched++;
		} else {
			unmatched++;
		}
	}
}


int main(int argc,char **argv)
{
	int len;
	int wlen2;
	int set[20], currset[20];
	char vals[40];
	struct ast_hashtab_iter *it;
	char *sptr;
	int i;
	int matched =0;
	int unmatched = 0;
	int wronglen = 0;
	
	if( argc < 4 )
	{
		printf("threeword <scrambleOfTwoWords> <longestwordlen> <nextlongestwordlen>\n");
		exit(10);
	}

		
	
	/* find all the n-digit numbers that have each digit mentioned only once. */
	/* all the numbers including only the numbers 1-n; */
	printf("Reading %s...\n", DICT);
	read_dict();
	printf("Done, wasn't that quick?\n");

	strcpy(vals,argv[1]);
	len = atoi(argv[2]);
	wlen2 = atoi(argv[3]);

	/* the initial set */
	for(i=0;i<len;i++)
	{
		set[i] = i;
	}

	printf("Searching... (Note: if you repeat the same letter more than once in a word, expect multiple matches!)\n");

	it = ast_hashtab_start_traversal(dict);
        while( (sptr = ast_hashtab_next(it)) ) {
		char buf3[40];
		char *p;
		int tlen = strlen(sptr);
		int i;
		if(tlen != len) {
			wronglen++;
			continue; // skip all the words not exactly "len" chars long!
		}
		strcpy(buf3, vals);
		for(i=0; i<tlen; i++) {
			char y;
			y=sptr[i];
			p = strchr(buf3,y);
			if(p) { 
				rmcharfromset(buf3,y);
			} else {
				break; // this ain't the word 
			}

		}
		if( p ) {
			char buf4[200];
			char buf5[200];
			int len7;
			printf("============================================================\n");
			printf("Match: Word=%s remainder=%s\n", sptr, buf3); 
			twowords(buf3, wlen2);
			/* the initial set */
			len7 = strlen(buf3);
			for(i=0;i<len7;i++)
			{
				set[i] = i;
			}
			permute(set, len7, buf3, 0, currset, 0);
			matched++;
		} else {
			unmatched++;
		}
	}
	printf ("Matched: %d\nUnmatched: %d\nWrong Length: %d\n", matched, unmatched, wronglen);
	
}
