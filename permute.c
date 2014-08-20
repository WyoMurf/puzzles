
/* 

Copyright (C) 2014 Steve Murphy, Cody, WY, All Rights Reserved.

Users may use, copy, and redistribute this code via the
GNU GPLv2 license. No warranties!


*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int permutation_count = 0;

copy_set(int *from, int* to)
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

void permute( int *set, int len, char **vals, int lev, int *currset, int (*func)(int *, int, char **))
{
	int i;
	
	if( lev == len )
	{
		(*func)(currset, len, vals);
		return;
	}
	for(i=0;i<len;i++)
	{
		if( !in_set(currset, lev, i) )
		{
			currset[lev] = i;
			permute( set, len, vals, lev+1, currset, func );
		}
	}
}

int print_result(int *currset, int len, char **vals)
{
		/* print out the set */
 	int i;
	for(i=0;i<len;i++)
	{
		printf("%s", vals[currset[i]]);
	}
	printf("\n"); 
	permutation_count++;
	
}

main(int argc,char **argv)
{
	int len;
	int set[20], currset[20];
	char *vals[20];
	char *vals2[20][20];
	
	int i;
	
	if( argc < 2 )
	{
		printf("permute <word>\n");
		exit(10);
	}
	
	len = strlen(argv[1]);
	if( len > 19 ) /* really 10 or 11 is getting impractically big! 40 million possibilities at 11! */
	{
		printf("String too long\n");
		exit(10);
	}
	
	/* find all the n-digit numbers that have each digit mentioned only once. */
	/* all the numbers including only the numbers 1-n; */

	for(i=0;i<len;i++)
	{
		vals[i] = vals2[i];
		vals[i][0] = argv[1][i];
		vals[i][1] = 0;
	}
	
	/* the initial set */
	for(i=0;i<len;i++)
	{
		set[i] = i;
	}
	
	permute(set, len, (char **)vals, 0, currset, print_result);

	printf("Done. That's been %d permutations.\n", permutation_count);
	
}
