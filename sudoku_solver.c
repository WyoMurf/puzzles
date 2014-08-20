/* 
  Copyright (C) 2014 Steve Murphy, Cody, WY
  All Rights reserved

  Permission to use this code via the GNU GPLv2.

  No warranties!

*/


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>

struct timeval start_time;
long long tries = 0;
int high_water = 0;
long long allocs = 0;
long long frees =0;
struct sd_state 
{
        short currpos_x; /* column number 1-9 */
        short currpos_y; /* row number 1-9 */
	int count_filled;
	int state[9][9];
};

int allsols = 0;
int sols = 0;

struct sd_state *read_init_state(char *fname)
{
	struct sd_state *x = calloc(sizeof(struct sd_state), 1);
	FILE *f = fopen(fname,"r");
	if (f && x)
	{
		int i,j;
		for(i=0; i<9; i++)
		{
			char buf1[90];
			fgets(buf1, sizeof(buf1), f);
			for(j=0; j<9; j++)
			{
				if(buf1[j] != ' ' && buf1[j] != '0')
				{
					char xx[2];
					xx[0] = buf1[j];
					xx[1] = 0;
					x->state[i][j] = atoi(xx);
					if( x->state[i][j] < 1 || x->state[i][j] > 9 )
					{
						printf("Illegal char in the soduko puzzle at pos (row,col) = (%d,%d)\n", i, j);
						exit(10);
					}
					else
					{
						x->count_filled++;
					}
				}
			}
		}
		fclose(f);
		return x;
	}
	else
	{
		return NULL;
	}
}

int legal(struct sd_state *state, int num)
{
	int i,j;
	if (num < 1 || num > 9) return 0;
	if (state->currpos_x < 0 || state->currpos_x > 8) return 0;
	if (state->currpos_y < 0 || state->currpos_y > 8) return 0;
	for(j=0; j<9; j++)
	{
		if (j==state->currpos_x) continue;
		if (num == state->state[state->currpos_y][j]) return 0;
	}
	for(i=0; i<9; i++)
	{
		if (i==state->currpos_y) continue;
		if (num == state->state[i][state->currpos_x]) return 0;
	}
	int ilim = (state->currpos_y/3)*3+3, jlim = (state->currpos_x/3)*3+3;
	for(i=(state->currpos_y/3)*3; i<ilim; i++)
	{
		for(j=(state->currpos_x/3)*3; j<jlim; j++)
		{
			if(i==state->currpos_y && j==state->currpos_x)
				continue;
			if( num == state->state[i][j] )
				return 0;
		}
	}
	return 1;
}

struct sd_state *copy_state(struct sd_state *x)
{
	struct sd_state *y = calloc(sizeof(struct sd_state),1);
	allocs++;
	*y = *x;
	return y;
}

void print_solution(struct sd_state *x)
{
	int i, j;

	for(i = 0; i<9; i++)
	{
		for(j = 0; j<9; j++)
		{
			if (x->state[i][j] != 0) 
			{
				printf(" %d", x->state[i][j]);
			}
			else
			{
				printf("  ");
			}
			if (j%3 == 2 && j!=8)
			{
				printf(" |");
			}
		}
		printf("\n");
		if(i%3 == 2 && i != 8)
			printf("-------+-------+-------\n");
	}
}

int solve(struct sd_state *x, int level)
{
	int i, j, num;
	struct sd_state *y = copy_state(x);

	for(i = y->currpos_y; i<9; i++)
	{
		for(j = 0; j<9; j++)
		{
			if (y->state[i][j] == 0) 
			{
				y->currpos_x = j;
				y->currpos_y = i;
				int one_legal =0;
				for(num=1; num<10; num++)
				{
					if (legal(y, num))
					{
						one_legal++;
						tries++;
						// if( level < 4)
						//	printf("Level %d: at (%d,%d) try %d\n", level, y->currpos_x, y->currpos_y, num);
                                          
						if (tries%10000000 == 0 ) {printf("Progress: tries:%lld, allocs=%lld, frees=%lld\n", tries, allocs, frees); }
						y->state[i][j] = num;
						y->count_filled = x->count_filled+1;
						if(y->count_filled > high_water)
						{
							// printf("Ha high water is: %d\n", high_water);
							high_water = y->count_filled;
						}
						if( y->count_filled == 81 )
						{
							struct timeval end_time;
							gettimeofday(&end_time, NULL);
							long mtime, secs, usecs;
							secs = end_time.tv_sec - start_time.tv_sec;
							usecs = end_time.tv_usec - start_time.tv_usec;
							mtime = secs * 1000000 + usecs;
							secs = mtime/1000000;
							usecs = mtime % 1000000;
							printf("YAY! We have a solution after %lld tries! elapsed time = %ld.%06ld\n", tries, secs, usecs);
							print_solution(y);
							sols++;
							if (allsols)
							{
								frees++;
								free(y);
								return 0;
							}
							else
							{
								exit(0);
							}
						}
						solve(y, level+1);
					}
					else
					{
						// printf("can't put %d at (%d,%d)\n", num, i, j);
					}
				}
				frees++; 
				free(y); 
				return 0;
			}
		}
	}
	free (y);
	frees++;
	return 0;
	
}

int main(int argc, char **argv)
{
	struct timeval end_time;
        sols = 0;
	if( argc < 2 || argc > 3)
	{
		printf("Hey-- this program requires 1 argument-- \n\
the name of a file that has 9 rows of 9 columns, \neach containing 1 thru 9, with a blank or zero for an empty position/");
                printf("\n  Usage:\n     sudoku-solver [-a] <puzzle-file>\n\n");
		exit(10);
	}
	else
	{
                int i;
		
		for (i=1; i< argc; i++)
		{
			if( !strcmp(argv[i], "-a") == 0)
			{
				allsols = 1;
				break;
			}
		}
		struct sd_state *x = read_init_state(argv[argc-1]);
		if (!x)
		{
			printf("Hey-- couldn't parse file %s\n", argv[1]);
		}
		else
		{
			int i, j;
			print_solution(x); // Show starting state
			printf("OK, we see that %d positions are already filled.\n", x->count_filled);
                        gettimeofday(&start_time, NULL);
			solve(x, 0);
			if (allsols) printf("Number of solutions=%d\n", sols);
		}
	}
}

