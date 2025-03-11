all: main.c arguments.c data_nodes.c errors.c first_pass.c preprocessor.c second_pass.c sentences.c utils.c
	gcc main.c arguments.c data_nodes.c errors.c first_pass.c preprocessor.c second_pass.c sentences.c utils.c -Wall -ansi -pedantic -o all

