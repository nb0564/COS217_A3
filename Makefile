# Author: Nicholas Budny

# Makefile for COS217 Assignment 3
# Symbol Table implementation with linked list and hash table

CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -pedantic -g

all: testsymtablelist testsymtablehash

testsymtablelist: testsymtable.o symtablelist.o
	$(CC) $(CFLAGS) -o testsymtablelist testsymtable.o symtablelist.o

testsymtablehash: testsymtable.o symtablehash.o
	$(CC) $(CFLAGS) -o testsymtablehash testsymtable.o symtablehash.o

testsymtable.o: testsymtable.c symtable.h
	$(CC) $(CFLAGS) -c testsymtable.c

symtablelist.o: symtablelist.c symtable.h
	$(CC) $(CFLAGS) -c symtablelist.c

symtablehash.o: symtablehash.c symtable.h
	$(CC) $(CFLAGS) -c symtablehash.c

clean:
	rm -f *.o testsymtablelist testsymtablehash
