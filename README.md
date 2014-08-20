puzzles
=======

various puzzle solvers

dominoe.c : Can you lay dominoe pieces down to completely
cover a 9x7 inch board, with one square blocked out? (Each piece
is 1x2 in.).  This program traverses all possible solutions
for each blanked square, and shows you how many solutions
are possible for each blank position.

hashtab.[ch], hashtest.c: written by me, Digium has the copyright,
and licenses it under GNU GPLv2. Sometimes, a game solver requires
a generic hashtable to make things faster.

permute.c: give it a word, it will show you all the permutations
possible by scrambling the letters.

permute2.c: Solves (completely) a number puzzle in the shape
of the Star of David, where all intersecting lines of 4 numbers
add to 26. Brute force, with equivalent rotations eliminated.

rock.c: This one isn't complete, so it forms its own little
puzzle for the reader.  Well, it is "complete", sort of. 
It's fully written, but it just doesn't work.
Can you fix the bugs and make it work? The source code comments
give you a link to picture of the puzzle on the web.

sudoku_solver.c, sudoku_test_*: Solve those pesky 9x9 puzzles.
The sudoku_test_* files serve as examples. It's quick and 
simple.

uniqueletter.c: Fairly worthless. 

wordscram.c: Ever see those word scramble puzzles in the newspaper?
  Don't want to wait until tomorrow for the solution? You 
  feed it the scrambled word as a command line argument, and
  it will give you a list of words that are formed by those
  letters.

wordsolve was obsolesced as wordscram.c was added to.


