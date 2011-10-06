CC=cc
SRC= main.c add.c add_s.c mult.c utils.c div.c bitwise.c sub.c sub_s.c mult_kar_s.c mult_kar.c test_utils.c test.c
FLAGS = -O0 -Wall -fopenmp -g
LIBS = -lgmp -lrt
OBJ=domath

compile: $(SRC)
	$(CC) $(FLAGS) $(SRC) -o $(OBJ) $(LIBS)

debug: $(SRC)
	$(CC) $(FLAGS) $(SRC) -o $(OBJ) $(LIBS)
	gdb $(OBJ)

clean:
	rm -f $(OBJ)

realclean:
	rm -f $(OBJ) *.csv core.*
