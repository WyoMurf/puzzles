/*

Copyright (C) 2014 Steve Murphy, Cody, WY
All Rights Reserved

Folks can use this code via the GNU GPLv2 license.

No warranties. This code isn't working yet. As a meta-puzzle,
I challenge readers to solve this puzzle by making the code
work.

*/


#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int set6[8];
int set4[6];
int currset6[8];
int currset4[6];
	char *val6[7];
	char *val4[6];
	char vals24[8][8]; /* not bothering to set these up right now */
	char vals26[8][8]; /* not bothering to set these up right now */
int total_solutions = 0;

/* another combinatorial puzzle */
/* this one is available as a puzzle. It a 14-sided polyhdedron, a "Tetradecahedra",
   and particularly, a "Truncated octahedron", with 6 square facets, and 8 regular hexagons.
   In each plastic facet, which attaches to the polyhedron via a central peg, and can be rotated,
   are a set of either 2 or 3 colored lines, of red, blue, and yellow. Each line goes from
   the center of one side of the facet, to the center of another side. The challenge is to arrange
   the facets, so the red, blue and yellow lines form an unbroken path around the cube.
   This puzzle is called the "Rock" and can be seen at:
   http://twistypuzzles.com/cgi-bin/puzzle.cgi?pkey=673
   The puzzle I have was produced by Twisted Minds Ltd, of NZ, and is labeled "Hot Rocks".

/* the data for the face of the polyhedron */

#define Y 'y' /* yellow */
#define B 'b' /* blue */
#define R 'r' /* red */

struct facet
{
	int facetno;        /* each facet has a number */
	int sides;          /* each facet has either 4 or 6 sides */
	int zero_side;     /* the zero-side of this facet is toward this facet number */
	int connections[6]; /* up to 6 connections per facet, the facet number of the corresponding facet  */
};


struct tile
{
	int tileno;
	int sides;
	char colors[6]; /* one color per side */
};

/* question: what exactly IS the '0' position of a tile? */
/* answer1:  the '0' side faces the lowest facet number of the adjoining 4 or 6 facets. */

/* random thoughts: we don't have to go thru all the permutations of all the tiles.
   all we have to do is fix the first tile, and then do all the permutations of the
   remaining tiles in the other spots. Each tile will need to be spun in all its 4 or 6
   ways to see if a fit can be made. There will be 0, or 2 positions where a tile can
   mesh with a previously placed tile.

   This can be done in two stages, first, to generate all the permutations of the tiles
   on the polyhedron. Then, spin all the pieces until either a complete match is
   made, or a rejection of the current arrangement, based on the fact that a tile 
   cannot be spun into a working arrangement.

   The spinning of the pieces is also a combinatorial sort of thing. we start with
   the #2 piece in the in the facet_order list, and know that every peice has 2 rotations
   where it can match the color on the edge with any other tile. We find first one of 
   the two matches on tile 2, then try 14. We test against all other facets that
   have been 'placed' or spun to a set value. At some point, we will not find a
   position that matches all the so-far connected pieces. So, we back up to the
   first tile that has not yet been turned all the way, and turn it cw one click, and
   repeat. When we back up, we move all the turned tiles 'behind' it back to their
   starting positions, and re-work our way thru them until we have rotated the first
   tile thru all 6 positions; Then we try the next combination of tiles on the polyhedron.

   When a match is made, the arrangement and spins on each tile are published, and a 
   catalog kept. There should be, methinks, 6 occurrences for every unique pattern, 
   because of the symmetry of the tile set.

*/


int facet_order[14] =  /* evaluate the facets in this order */
{
	0,1,13,4,8,7,11,5,3,6,10,9,12,2
};

/* no special reason for the above order, but it does spiral around the 
   facets nicely */

struct facet polyhedron[14] = {
       /* num, sides, 0 side, each side is next to the indicated facet number */
	{ 0,  6, 1, {4,8,7,11,1,13}},
	{ 1,  6, 0, {0,11,6,10,5,13}},
	{ 2,  6, 3, {9,5,10,6,12,3}},
	{ 3,  6, 2, {7,8,4,9,2,12}},
	{ 4,  6, 0, {5,9,3,8,0,13}},
	{ 5,  6, 1, {4,13,1,10,2,9}},
	{ 6,  6, 1, {12,2,10,1,11,7}},
	{ 7,  6, 0, {3,12,6,11,0,8}},
	{ 8,  4, 0, {3,7,0,4}},
	{ 9,  4, 2, {2,3,4,5}},
	{ 10, 4, 1, {2,5,1,6}},
	{ 11, 4, 0, {7,6,1,0}},
	{ 12, 4, 2, {2,6,7,3}},
	{ 13, 4, 0, {4,0,1,5}}	
};

struct tile tiles[14] = {  /* 
      tile num, sides, color of line at center of each side of the tile */
	{ 0, 4,  {Y,Y,B,B}},
	{ 1, 6,  {Y,B,R,R,B,Y}},
	{ 2, 6,  {B,B,R,Y,R,Y}},
	{ 3, 4,  {Y,B,Y,B}},
	{ 4, 4,  {B,R,B,R}},
	{ 5, 4,  {R,Y,R,Y}},
	{ 6, 6,  {B,Y,R,Y,B,R}},
	{ 7, 4,  {R,R,B,B}},
	{ 8, 4,  {Y,Y,R,R}},
	{ 9, 6,  {R,Y,B,Y,R,B}},
	{ 10, 6, {B,B,Y,R,Y,R}},
	{ 11, 6, {R,Y,Y,B,R,B}},
	{ 12, 6, {Y,Y,R,B,B,R}},
	{ 13, 6, {B,B,R,R,Y,Y}},
};

struct state
{
	int set[14]; /* the set of which tiles are on which facets */
	int rot[14]; /* The number of cw steps each tile is turned. 0 - starting position; sides-1 = last position When rot[0] == 6, it's time to try a new tile placement */
	int lev; /* a number that varies from 0 to 13. a full match on tile <facet_order[13]> should get published as a success. */
};

/* here is one solution:
 Facet   Tile
   0      11
   1      13
   2      10
   3      6
   4      2
   5      1
   6      9
   7      12
   8      7
   9      0
  10      4
  11      3
  12      8
  13      5

Uh, no rotations yet. I guess you could spin pieces all day... */

int ready_for_check(int facet_from, int facet_to)
{
	switch(facet_from)
	{
	case 0: /* first facet */
		return 0; /* nobody has been placed yet */
	case 1:
		if(facet_to == 0)
			return 1;
		else
			return 0;
	case 2:
		/* last facet */
		return 1;
	case 3:
		switch(facet_to)
		{
		case 6:
		case 10:
		case 9:
		case 12:
		case 2:
			return 0;
		default:
			return 1;
		}
		
	case 4:
		switch(facet_to)
		{
		case 0:
		case 1:
		case 13:
			return 1;
		default:
			return 0;
		}
		
	case 5:
		switch(facet_to)
		{
		case 3:
		case 6:
		case 10:
		case 9:
		case 12:
		case 2:
			return 0;
		default:
			return 1;
		}

	case 6:
		switch(facet_to)
		{
		case 10:
		case 9:
		case 12:
		case 2:
			return 0;
		default:
			return 1;
		}
	case 7:
		switch(facet_to)
		{
		case 0:
		case 1:
		case 13:
		case 4:
		case 8:
			return 1;
		default:
			return 0;
		}
	case 8:
		switch(facet_to)
		{
		case 0:
		case 1:
		case 13:
		case 4:
			return 1;
		default:
			return 0;
		}
	case 9:
		switch(facet_to)
		{
		case 12:
		case 2:
			return 0;
		default:
			return 1;
		}
	case 10:
		switch(facet_to)
		{
		case 9:
		case 12:
		case 2:
			return 0;
		default:
			return 1;
		}
	case 11:
		switch(facet_to)
		{
		case 0:
		case 1:
		case 13:
		case 4:
		case 8:
		case 7:
			return 1;
		default:
			return 0;
		}
	case 12:
		if( facet_to == 2 )
			return 0;
		else 
			return 1;
	case 13:
		if( facet_to < 2 )
			return 1;
		else 
			return 0;
	}
}

/* this is a pretty key function-- */

char get_color(struct state *state, int facet_num, int spin, int connection_index)
{
	int sides = polyhedron[facet_num].sides;
	int tile = state->set[facet_num];
	
	/* interesting routine. think about it. the first color faces the connection_index=0;
	   the spin, and the index can add up to twice the number of sides-2; 
	   so, for 6 sides
        0 1 2 3 4 5 6 7 8 9 10 = spin + connection_index (both vary from 0 to 5)
		0 1 2 3 4 5 0 1 2 3 4  = desired side ; or (spin+index)%6

		and for 4 sides
        0 1 2 3 4 5 6 = spin + index (both vary from 0 to 3)
		0 1 2 3 0 1 2 = desired side; or (spin+index)%4
	*/
	/* quick sanity check-- you should be able to remove this from 'production' code */
	if(tiles[tile].sides != sides)
	{
		printf("ERROR: disagreement between sides between facet #%d, and tile #%d; see the state->set info!\n",
			   facet_num, tile);
	}
	
	return tiles[tile].colors[(spin+connection_index)%sides];
}


int itfits(struct state *state) 
{
	int facet_from = facet_order[state->lev];
	int facet_from_spin = state->rot[facet_from];
	
	int sides = polyhedron[facet_from].sides;
	int i;
	
	if( state->lev == 0 ) /* the first tile is always OK. */
		return 1;
	
	/* tile_to will come from the connection list of the facet */
	for(i=0; i< sides; i++)
	{
		int facet_to;
		int facet_to_spin;
		
		facet_to = polyhedron[facet_from].connections[i];

		if (ready_for_check(facet_from, facet_to)) /* has the connected tile been actually 'set' yet? */
		{
			int j;
			int facet_to_connection_index;
			char color_to, color_from;
			
			facet_to_spin = state->rot[facet_to];

			for(j=0;j<polyhedron[facet_to].sides;j++)
			{
				if(polyhedron[facet_to].connections[j] == facet_from)
				{
					/* this particular connection points back to 'from' */
					break;
				}
			}
			if(j>= polyhedron[facet_to].sides)
			{
				/* we should never see this message-- but, if I messed up a connection somewhere, it'll be best to know it! */
				printf("ERROR: from facet %d to facet %d, could not find on facet %d a connection back to %d!!!\n",
					   facet_from, facet_to, facet_to, facet_from);
				break;
			}
			/* the zero side for a tile is always the one facing the first connected facet */
			/* now j points to which side on the 'to' facet we are connected to, and i points to the side on the 'from' side */
			/* so, now it should be 'simple' to see if the color on 'from' matches the color on 'to' */
			color_from = get_color(state, facet_from, facet_from_spin, i);
			color_to = get_color(state, facet_to, facet_to_spin, j);
			if( color_from != color_to )
				return 0; /* look for an "active" disagreement; and return false in that case */
		}
	}
	return 1; /* if no disagreements, then it fits, so far */
}

/* this is the next 'key' routine, the algorithm to step thru the tiles in the order presented by
   facet_order[], and using itfits() and the incrementing state->spin until all the rotations
   have been disqualified. If we make it all the way to the end, then we can print out the match */

void spin_tiles(struct state *state, int lev)
{
	int i, ret;
	int curr_facet = facet_order[lev];
	int curr_sides = polyhedron[lev].sides;
	state->lev = lev;
	for(i=0;i<curr_sides;i++)
	{
		state->rot[lev] = i;
		
		if( itfits(state) )
		{
			if(lev == 13)
			{
				/* report here the success== we found a solution */
				int j;
				
				total_solutions++;
				printf("============================= solution #%d\n", total_solutions);
				for(j=0;j<13;j++)
				{
					printf("Facet: %d  Tile: %d; 0spin = %d; spin = %d\n", j, state->set[j], polyhedron[j].connections[0], state->rot[j]);
				}
				printf("=============================\n");
				return;
			}
			else
				spin_tiles(state, lev+1);
		}
		/* else spin this tile and try again */
	}
	/* spun all the way, so: */
	state->rot[lev] = 0;
	state->lev--;
	return;
}

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

/* just an example func from an earlier implementation */
int print_result(int *currset, int len, char **vals)
{
		/* print out the set */
	int i;
	for(i=0;i<len;i++)
	{
		printf("%s", vals[currset[i]]);
	}
	printf("\n");
}


int countup = 0;
struct state mystate;

permute4(int *currset, int len, char **vals)
{
	
	/* all the real work gets done here. At this point, we have a generated
       permutation for the 4-sided tiles, and this is based on a single 
	   permutation of the 6-sided tiles. */
	/* the .set index is the facet number, and the value is the tile number! */
	int i;
	
	mystate.set[8] = vals[currset4[0]][0];
	mystate.set[9] = vals[currset4[1]][0];
	mystate.set[10] = vals[currset4[2]][0];
	mystate.set[11] = vals[currset4[3]][0];
	mystate.set[12] = vals[currset4[4]][0];
	mystate.set[13] = vals[currset4[5]][0];
	countup++;
	for(i=0;i<14;i++)
	{
		mystate.rot[i] = 0;
	}
	spin_tiles(&mystate, 0);
	
}

permute8(int *currset, int len, char **vals)
{
	
	/* this function calls permute for the final set of 4-sided tiles */
	mystate.set[0] = 1; /* forever fixed. Otherwise, it's the same thing over and over */
	mystate.set[1] = vals[currset6[0]][0];
	mystate.set[2] = vals[currset6[1]][0];
	mystate.set[3] = vals[currset6[2]][0];
	mystate.set[4] = vals[currset6[3]][0];
	mystate.set[5] = vals[currset6[4]][0];
	mystate.set[6] = vals[currset6[5]][0];
	mystate.set[7] = vals[currset6[6]][0];
	
	permute(set4, 6, (char **)val4, 0, currset4, permute4);
}

main(int argc,char **argv)
{
	/* set up the field for the 8 tiles and the 4 tiles */
	int i;
	
	for(i=0;i<7;i++)
	{
		set6[i] = i;
	}
	for(i=0;i<6;i++)
	{
		set4[i] = i;
	}
	for(i=0;i<7;i++)
	{
		val6[i] = (char *)vals26[i];
	}
	for(i=0;i<6;i++)
	{
		val4[i] = (char *)vals24[i];
	}
	
	vals26[0][0] = 2; /* skip 1 */
	vals26[1][0] = 6;
	vals26[2][0] = 9;
	vals26[3][0] = 10;
	vals26[4][0] = 11;
	vals26[5][0] = 12;
	vals26[6][0] = 13;

	vals24[0][0] = 0;
	vals24[1][0] = 3;
	vals24[2][0] = 4;
	vals24[3][0] = 5;
	vals24[4][0] = 7;
	vals24[5][0] = 8;

	permute(set6, 7, (char **)val6, 0, currset6, permute8);

	printf("count of %d permutations of tiles on the facets!\n", countup);
	
}

/* note: there are 40320 permutations of the 8 6-sided tiles
         and there are 720 permutations of the 6 4-sided tiles,
		 for a total of 29,030,400 total combinations (not the awesome 14 factorial...)
		 ..AND.. you can fix the first tile, and let the others permute around it, so,
		 really, you only have 5040 (7!) times 720 or 3,628,800 permutations,
		 which really, is not that bad. Only a few seconds (literally) on my machine, to 
		 step thru!
*/
