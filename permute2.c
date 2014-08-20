
/* permutation generator written by Steve Murphy *
/* Copyright 2008 Steve Murphy */
/* All rights reserved */
/* No Warranties! */

/* this programs finds (by brute force permutations)
   all the solutions to the puzzle, where a star of
   David is formed by 6 lines, and every intersection
   of the star of 2 lines has a number, making 12 numbers.
   All 4 numbers in each line must add to 26.
   The 12 positions can filled with the numbers 1 to 12,
   and each number can only be used once, and every number
   has to be used.

   I number the positions so:

                   1
                 /   \
            2---3-----4---5
             \ /       \ /
              6         7
             / \       / \
            8---9----10---11
                  \  /
                   12

    When you take all the possible arrangements of 
    12 numbers, you get a lot of permutations--
    over 479 million, but computers are fast these
    days, and indeed, this program can run thru
    all the permutations on a 3 GHz machine in 
    about 16 or so minutes, which isn't that bad.

    Of interest, is the fact that every solution
    has 5 rotations, so each solution will be found
    6 times. So, finding the set of UNIQUE solutions
    is a little bit more challenging. I do this
    by filing each solution's rotations in a hash 
    table, and checking each solution against that
    table before declaring it a solution.

*/
    

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* we get our hashtab data structures from the work I did on the Asterisk
   PBX software. Since I wrote it, I can do this! */
#include "hashtab.h"

struct ast_hashtab *solutions;

int permutation_count = 0;
int solution_count = 0;
int non_unique_solutions_count = 0;


/* Code to do permutations */

/*          utility functions */

/*      in_set is used to make sure that the number we
               have chosen isn't already in the set */

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

/*        the workhorse. note that it calls itself.
           args:

              int *set  == an array of ints, indexes into the permutable item list
              int len  == the number of items to permute.
              char **vals  == an array of the strings that are being permuted.
              int lev  == which item we are going to be selecting. At each level
                          of recursion, we will be selecting the next permuted item.
                          The deeper we go, the more items we have selected.
              int *currset  == while set points to the list of items, the 'currset'
                           is the working list of permuted items. when we reach the bottom
                           (when the lev equals len), we have a complete permuted set.
              int (*func)(int *, in, char **) == a func to call each time we have
                           a complete permuted list; It returns an int. It takes 3 args,
                           and array of ints, a len, and an array of char pointers.
*/
void permute( int *set, int len, char **vals, int lev, int *currset, int (*func)(int *, int, char **))
{
	int i;
	
	if( lev == len )
	{
		(*func)(currset, len, vals); /* at the end of the recursion, call the func we passed in */
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
	char solutionstring[1000];
	
		/* print out the set */
/* 	int i;
	for(i=0;i<len;i++)
	{
		printf("%s", vals[currset[i]]);
	}
	printf("\n"); */

	/* test to see if each of the 6 lines of numbers adds to 26 */	
	if(  (char)vals[currset[0]][0] + (char)vals[currset[2]][0] + (char)vals[currset[5]][0] + (char)vals[currset[7]][0] == 26
	  && (char)vals[currset[0]][0] + (char)vals[currset[3]][0] + (char)vals[currset[6]][0] + (char)vals[currset[10]][0] == 26
	  && (char)vals[currset[1]][0] + (char)vals[currset[2]][0] + (char)vals[currset[3]][0] + (char)vals[currset[4]][0] == 26
	  && (char)vals[currset[1]][0] + (char)vals[currset[5]][0] + (char)vals[currset[8]][0] + (char)vals[currset[11]][0] == 26
	  && (char)vals[currset[7]][0] + (char)vals[currset[8]][0] + (char)vals[currset[9]][0] + (char)vals[currset[10]][0] == 26
	  && (char)vals[currset[4]][0] + (char)vals[currset[6]][0] + (char)vals[currset[9]][0] + (char)vals[currset[11]][0] == 26)
	{
		char *lookup_result;
		 /* yay! we found a solution! Check in the hashtab for it--
		    forming a string of the list of numbers should make for
                    fairly quick way of checking */
		sprintf(solutionstring,"%d-%d-%d-%d-%d-%d-%d-%d-%d-%d-%d-%d", 
				(char)vals[currset[0]][0],
				(char)vals[currset[1]][0],
				(char)vals[currset[2]][0],
				(char)vals[currset[3]][0],
				(char)vals[currset[4]][0],
				(char)vals[currset[5]][0],
				(char)vals[currset[6]][0],
				(char)vals[currset[7]][0],
				(char)vals[currset[8]][0],
				(char)vals[currset[9]][0],
				(char)vals[currset[10]][0],
				(char)vals[currset[11]][0]);
		lookup_result = ast_hashtab_lookup(solutions, solutionstring);
		if (!lookup_result)
		{
			printf("\n Solution: 1=%d; 2=%d; 3=%d; 4=%d; 5=%d; 6=%d, 7=%d; 8=%d; 9=%d; 10=%d; 11=%d; 12=%d\n",
				   (char)vals[currset[0]][0],
				   (char)vals[currset[1]][0],
				   (char)vals[currset[2]][0],
				   (char)vals[currset[3]][0],
				   (char)vals[currset[4]][0],
				   (char)vals[currset[5]][0],
				   (char)vals[currset[6]][0],
				   (char)vals[currset[7]][0],
				   (char)vals[currset[8]][0],
				   (char)vals[currset[9]][0],
				   (char)vals[currset[10]][0],
				   (char)vals[currset[11]][0]);
			solution_count++;
			/* really, we don't need to insert the unrotated solution;
                           it will never come up again. But, oh, well.... */
			ast_hashtab_insert_immediate(solutions, strdup(solutionstring));

			/* How do I come up the index numbers? just take the drawing, rotate it to the
                           right one step, and now read off the position numbers on the rotated set... */

			/* rotate 1/6 to the right */
			sprintf(solutionstring,"%d-%d-%d-%d-%d-%d-%d-%d-%d-%d-%d-%d", 
					(char)vals[currset[1]][0],
					(char)vals[currset[7]][0],
					(char)vals[currset[5]][0],
					(char)vals[currset[2]][0],
					(char)vals[currset[0]][0],
					(char)vals[currset[8]][0],
					(char)vals[currset[3]][0],
					(char)vals[currset[11]][0],
					(char)vals[currset[9]][0],
					(char)vals[currset[6]][0],
					(char)vals[currset[4]][0],
					(char)vals[currset[10]][0]);
			ast_hashtab_insert_immediate(solutions, strdup(solutionstring));
			/* rotate 2/6 to the right */
			sprintf(solutionstring,"%d-%d-%d-%d-%d-%d-%d-%d-%d-%d-%d-%d", 
					(char)vals[currset[7]][0],
					(char)vals[currset[11]][0],
					(char)vals[currset[8]][0],
					(char)vals[currset[5]][0],
					(char)vals[currset[1]][0],
					(char)vals[currset[9]][0],
					(char)vals[currset[2]][0],
					(char)vals[currset[10]][0],
					(char)vals[currset[6]][0],
					(char)vals[currset[3]][0],
					(char)vals[currset[0]][0],
					(char)vals[currset[4]][0]);
			ast_hashtab_insert_immediate(solutions, strdup(solutionstring));
			/* rotate 3/6 to the right */
			sprintf(solutionstring,"%d-%d-%d-%d-%d-%d-%d-%d-%d-%d-%d-%d", 
					(char)vals[currset[11]][0],
					(char)vals[currset[10]][0],
					(char)vals[currset[9]][0],
					(char)vals[currset[8]][0],
					(char)vals[currset[7]][0],
					(char)vals[currset[6]][0],
					(char)vals[currset[5]][0],
					(char)vals[currset[4]][0],
					(char)vals[currset[3]][0],
					(char)vals[currset[2]][0],
					(char)vals[currset[1]][0],
					(char)vals[currset[0]][0]);
			ast_hashtab_insert_immediate(solutions, strdup(solutionstring));
			/* rotate 4/6 to the right */
			sprintf(solutionstring,"%d-%d-%d-%d-%d-%d-%d-%d-%d-%d-%d-%d", 
					(char)vals[currset[10]][0],
					(char)vals[currset[4]][0],
					(char)vals[currset[6]][0],
					(char)vals[currset[9]][0],
					(char)vals[currset[11]][0],
					(char)vals[currset[3]][0],
					(char)vals[currset[8]][0],
					(char)vals[currset[0]][0],
					(char)vals[currset[2]][0],
					(char)vals[currset[5]][0],
					(char)vals[currset[7]][0],
					(char)vals[currset[1]][0]);
			ast_hashtab_insert_immediate(solutions, strdup(solutionstring));
			/* rotate 5/6 to the right */
			sprintf(solutionstring,"%d-%d-%d-%d-%d-%d-%d-%d-%d-%d-%d-%d", 
					(char)vals[currset[4]][0],
					(char)vals[currset[0]][0],
					(char)vals[currset[3]][0],
					(char)vals[currset[6]][0],
					(char)vals[currset[10]][0],
					(char)vals[currset[2]][0],
					(char)vals[currset[9]][0],
					(char)vals[currset[1]][0],
					(char)vals[currset[5]][0],
					(char)vals[currset[8]][0],
					(char)vals[currset[11]][0],
					(char)vals[currset[7]][0]);
			ast_hashtab_insert_immediate(solutions, strdup(solutionstring));
		}
		else
			non_unique_solutions_count++;
	}
	
	permutation_count++;
	if( ((permutation_count+1) % 1000000) == 0 )
	{
		printf("."); fflush(stdout); /* show progress to keep the impatient calm -- you should get around 470 of these, each standing for a million generated permutations */
	}
}

main(int argc,char **argv)
{
	int len = 12; /* this val says we are permuting 12 items */
	int set[20], currset[20]; /* set is a list of the 12 items to permute. currset is a work area where we generate all the permutations */
	char *vals[20];  /* vals is a set of up to 20 string pointers */
	char *vals2[20][20]; /* and here is where we can store those strings of up to 20 chars each */
	int i; /* a temp working var */
	
	/* find all the n-digit numbers that have each digit mentioned only once. */
	/* all the numbers including only the numbers 1-n; */

	/* set up the arrays */
	for(i=0;i<len;i++)
	{
		/* now, this code was adapted from a previous program that permuted letters in a word.
		   The idea was to take a scrambled word, and generate all the permutations of that
                   set of letters, and look each one up in the dictionary, and output just the permutations
                   that resulted in a valid word. Well, this code can handle not just individual letters,
                   but whole strings to permute. For instance, you could permut apples, oranges, and grapefruit;
                    you would get these sets of 3:
                      apples, oranges, grapefruit
                      apples, grapefruit, oranges
                      oranges, apples, grapefruit
                      oranges, grapefruit, apples
                      grapefruit, apples, oranges
                      grapefruit, oranges, apples
                   Well, that's fine and good, but this time all we want to permute is the numbers 1 thru 12.
		   so, we put one letter in each string, the letters with values 1 thru 12. We won't be
                   printing them as letters, thank goodness, because they'd just be control characters
                   and not very visible. But a char is 8 bits, and we can store the numbers 0-255 in those
                   8 bits, which is more than enough to handle 1-12. */
		vals[i] = (char *)vals2[i];  /* now remember, vals is a set of char pointers. So make them
                                                point to the real storage area, vals2 */
		vals[i][0] = (char)i+1;      /* here is where we stick the vals 1 to 12 into vals2 via the pointers in vals */
		vals[i][1] = 0;              /* not really necessary, but done out of habit: strings are null terminated */
	}
	
	/* the initial set */
	/* now, we don't play around with the vals arrays. Instead, we play around with the indexes into the
           val array. */
	for(i=0;i<len;i++)
	{
		set[i] = i;
	}

	/* set up the hash table of the solutions found, and the 5 rotations of each solution, to find just a set of 'unique' solutions */
	solutions = ast_hashtab_create(7109, ast_hashtab_compare_strings, ast_hashtab_resize_none, ast_hashtab_newsize_none, ast_hashtab_hash_string, 0);

	/* now, everything is all set up. Fire up the permute engine */	
	permute(set, len, (char **)vals, 0, currset, print_result);

	/* approximately 16 minutes later on my gutless 3 Ghz machine,
           permute completes, and we can output the totals kept in global vars */

	/* now, in this case, 12 items will result in just over 479 million permutations. This puzzle yields 960 solutions,
           of which only 160 are really unique. The remaining 800 solutions are simply rotations of previously found
           solutions. This is due to the symmetry of the puzzle. I don't look at reflections (mirror images). Hadn't
           thought of that till now. Might be, that the rotations will cover the reflections. This is a further
           puzzle left to the reader. */

	printf("\nDone. That's been %d permutations; %d unique solutions, %d non-unique solutions.\n", permutation_count, solution_count, non_unique_solutions_count);

	/* we can free up that hashtab now, it's done. Yeah, I know, it's like rearranging deck chairs on the sinking Titanic... */
	ast_hashtab_destroy(solutions, free);
}
