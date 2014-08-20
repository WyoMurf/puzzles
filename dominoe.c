#include <stdio.h>

/*
  Copyright (C) 2014, Steve Murphy, Cody, Wyoming, All Rights Reserved.
  Licensed via the GNU GPLv2.
  No Warranties!
*/

/* problem: 

A board, 9 in x 7 in, is marked out into 63 one-inch squares. To
make the number of squares even, the middle square is clocked out by placing 
a chessman on it. John attempts to cover the board, other than the middle
square, with dominoes which are 2inches by 1 inch.

Since 31 pieces will be needed in all, more than one set will be required. Can you
completely cover the board, other than the middle square, without having any domino
stick out over the edge of the board, or can you prove it cannot be done?

*/
/* proof by exhaustive permutation of placements: */

#define BWID 9
#define BHGT 7

int move_count = 0;
int LIMIT = 0;
int successes = 0;

typedef char board[BWID][BHGT];

void init_board(board x, int col, int row)
{
	int i,j;
	for(i=0;i<BWID;i++)
	{
		for(j=0;j<BHGT;j++)
		{
			x[i][j] = 0;
		}
	}
	x[col][row] = 99;
}


void print_board(board x)
{
	int i,j;
	printf("+=========+\n");
	for(j=0;j<BHGT;j++)
	{
		printf("|");
		
		for(i=0;i<BWID;i++)
		{
			if( x[i][j] == 99 )
				printf("*");
			else if (x[i][j] == 0 )
				printf(" ");
			else
				printf("%c",x[i][j]+'@');
		}
		printf("|\n");
	}
	
	printf("+=========+\n");
}

int place_board(board x, char orient, int pno)
{
	int i,j;

	move_count++;
	
	for(j=0;j<BHGT;j++)
	{
		for(i=0;i<BWID;i++)
		{
			if( orient == 'H' )
			{
				if( i < (BWID-1) && x[i][j] == 0 && x[i+1][j] == 0 )
				{
					x[i][j] = x[i+1][j] = pno;
					return 1;
				}
				
			}
			else
			{
				if( j < (BHGT-1) && x[i][j] == 0 && x[i][j+1] == 0 )
				{
					x[i][j] = x[i][j+1] = pno;
					return 1;
				}
			}
		}
	}
	return 0;
}

void copy_board(board from, board to)
{
	int i,j;
	for(j=0;j<BHGT;j++)
	{
		for(i=0;i<BWID;i++)
		{
			to[i][j] = from[i][j];
		}
	}
}

int make_moves(board x, int nummoves)
{
	/* add a peice horizontally, and proceed with more pieces,
	   when finished, add a peice vertically, and proceed with more pieces */
	/* if success encountered, print the board. */
	board y;
	
	if( LIMIT > 0 && move_count > LIMIT )
		return 0;
	if( nummoves == 31 )
		return 0;
	if( !LIMIT && nummoves == 5 )
	{
		printf("Level 5 move made, move count = %d\n", move_count);
		print_board(x);
	}
	
	copy_board(x, y);
	if( place_board(y, 'H', nummoves+1) )
	{/* success */
		if( nummoves == 30 )
		{
			successes++;
			if( !LIMIT )
			{
				printf("Success!\n");
				print_board(y);
			}
			return 1;
		}
	}
	else 
		return 0; /* if I can't make a move, then why dive deeper? */
	
	make_moves(y, nummoves+1); /* dive down */
	
	/* when we finish with that subtree, try the next */

	copy_board(x, y);
	if( place_board(y, 'V', nummoves+1) )
	{/* success */
		if( nummoves == 30 )
		{
			successes++;
			if( !LIMIT )
			{
				printf("Success!\n");
				print_board(y);
			}
			return 1;
		}
	}
	else 
		return 0; /* if I can't make a move, then why dive deeper? */
	
	make_moves(y, nummoves+1); /* dive down */
	return 1;
}



main(int argc,char **argv)
{
	int i,j;
	board x,y;
	int succ[20][20];
	
	/* init_board(x, 4, 3); we are going to play ALL possible games with all possible blocked out squares */

	LIMIT=0;
	
	/* make_moves(x,0); */

	LIMIT=100000;
	
	printf("Move count = %d\n", move_count);
	printf("Successes = %d\n", successes);

	for(j=0;j<BHGT;j++)
	{
		if( j == 1 )
			LIMIT = 4000000;
		else
			LIMIT = 100000;
		
		for(i=0;i<BWID;i++)
		{
			successes = 0;
			move_count = 0;
			init_board(x,i,j);
			make_moves(x,0);
			succ[i][j] = successes;
			if( i == 0 ) printf("\n");
			printf("%d.",succ[i][j]);
			fflush(stdout);
		}
	}
	/* print results */
	printf("\n+------------------------------------------------------------------------+\n");
	for(j=0;j<BHGT;j++)
	{
		printf("|");
		for(i=0;i<BWID;i++)
		{
			printf("%7d ", succ[i][j]);
		}
		printf("|\n");
	}
	printf("+------------------------------------------------------------------------+\n");
	for(j=0;j<BHGT;j++)
	{
		for(i=0;i<BWID;i++)
		{
			if( succ[i][j] > 0 )
				y[i][j] = 1;
			else
				y[i][j] = 0;
		}
	}
	print_board(y);
}
