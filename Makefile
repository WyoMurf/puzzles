


exes : wordscram dominoe permute rock permute2 sudoku_solver twowords threewords

wordscram : wordscram.o hashtab.o
	gcc -g -o wordscram wordscram.o hashtab.o -lpthread

twowords : twowords.o hashtab.o
	gcc -g -o twowords twowords.o hashtab.o -lpthread

twowords.o : twowords.c
	gcc -g -c twowords.c

threewords : threewords.o hashtab.o
	gcc -g -o threewords threewords.o hashtab.o -lpthread

threewords.o : threewords.c
	gcc -g -c threewords.c

dominoe : dominoe.o
	gcc -g -o dominoe dominoe.o

dominoe.o : dominoe.c
	gcc -g -c dominoe.c

hashtab.o : hashtab.c hashtab.h
	gcc -g -c hashtab.c

wordscram.o : wordscram.c
	gcc -g -c wordscram.c

permute : permute.o
	gcc -g -o permute permute.o

permute2 : permute2.c hashtab.o
	gcc permute2.c -g -o permute2 hashtab.o -lpthread

permute.o : permute.c
	gcc -g -c permute.c

rock : rock.o
	gcc -g -o rock rock.o

rock.o : rock.c
	gcc -g -c rock.c

sudoku_solver : sudoku_solver.c
	gcc -g -o sudoku_solver sudoku_solver.c
