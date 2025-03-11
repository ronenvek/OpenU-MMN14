#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "data_nodes.h"
#include "sentences.h"
#include "utils.h"
#include "arguments.h"
#include "first_pass.h"
#include "errors.h"


/* an array containing all of the conserved words */
char* conserved_words[28] = {
    ".string",
    ".data",
    ".define",
    ".extern",
    ".entry",
    "mov",
    "cmp",
    "add",
    "sub",
    "not",
    "clr",
    "lea",
    "inc",
    "dec",
    "jmp",
    "bne",
    "red",
    "prn",
    "jst",
    "rts",
    "hlt",
    "r1",
    "r2",
    "r3",
    "r4",
    "r5",
    "r6",
    "r7"
};

/* an array containing all of the valid operations */
char* valid_operations[21] = {
    ".string",
    ".data",
    ".define",
    ".extern",
    ".entry",
    "mov",
    "cmp",
    "add",
    "sub",
    "not",
    "clr",
    "lea",
    "inc",
    "dec",
    "jmp",
    "bne",
    "red",
    "prn",
    "jst",
    "rts",
    "hlt"
};


/* an array containing the valid number of arguments for each operation (except .data which can take an unlimited number of variables and .define which is handled in the second pass) */
struct valid_argc_struct valid_argc[19] = {
    {"mov", 2},
    {"cmp", 2},
    {"add", 2},
    {"sub", 2},
    {"not", 1},
    {"clr", 1},
    {"lea", 2},
    {"inc", 1},
    {"dec", 1},
    {"jmp", 1},
    {"bne", 1},
    {"red", 1},
    {"prn", 1},
    {"jst", 1},
    {"rts", 0},
    {"hlt", 0},
    {".string", 1},
    {".entry", 1},
    {".extern", 1}
};



/* returns weather the given operation is valid */
int is_valid_operation(char* op) {
    int i;
    for (i=0; i<21; i++) /* go through every valid operation */
        if (strcmp(op, valid_operations[i])==0) /* return true if the operation is found */
            return TRUE;
    return FALSE; /* return false if the operation is missing */
}


/* returns weather the given word is conserved */
int is_conserved_word(char* word) {
    int i;
    for (i=0; i<28; i++) /* go through every conserved word */
        if (strcmp(word, conserved_words[i])==0) /* if the word is found, return true */
            return TRUE;
    return FALSE; /* if it is missing return false */
}


/* returns weather the given sentence contains the right amount of arguments for its operation */
int is_valid_argc(sentence s) {
    int i;
    
    /* .data can have an unlimited amount of arguments (but not 0) */
    if (strcmp(s.operation, ".data")==0)
        return s.argc>0;
    
    for(i=0; i<19; i++) /* go through every operation and it's valid argc */
        if (strcmp(s.operation, valid_argc[i].name)==0) /* if the operation is found, return weather it has the right argc */
            return s.argc == valid_argc[i].argc;
    return FALSE; /* return false if the operation is not found */
}


/* returns weather the given sentence has the correct types of arguments for its operation */
int valid_arg_types(sentence s) {
    char* op;
    op = s.operation;
	
	/* if the operation is - mov, add or sub */
    if (strcmp(op,"mov")==0 || strcmp(op,"add")==0 || strcmp(op,"sub")==0) {
        arg_type origin = get_arg_type(s.argv[0]);
        arg_type destination = get_arg_type(s.argv[1]);
        /* the origin operand can be 1, 2, 3 or 4 and the destination type can be 1, 2 or 3*/
        return (origin==0 || origin==1 || origin==2 || origin==3) && 
               (destination==1 || destination==2 || destination==3);
    }
	
	/* if the operation is - cmp */
    if (strcmp(op,"cmp")==0) {
        arg_type origin = get_arg_type(s.argv[0]);
        arg_type destination = get_arg_type(s.argv[1]);
        /* both operands can be 1, 2, 3 or 4*/
        return (origin==0 || origin==1 || origin==2 || origin==3) && 
               (destination==0 || destination==1 || destination==2 || destination==3);
    }
	
	/* if the operation is - not, clr, inc, dec or red */
    if (strcmp(op,"not")==0 || strcmp(op,"clr")==0 || strcmp(op,"inc")==0 || strcmp(op,"dec")==0 || strcmp(op,"red")==0) {
        arg_type destination = get_arg_type(s.argv[0]);
        /* the destination type can be 1, 2 or 3*/
        return destination==1 || destination==2 || destination==3;
    }
    
    /* if the operation is - lea */
    if (strcmp(op,"lea")==0) {
        arg_type origin = get_arg_type(s.argv[0]);
        arg_type destination = get_arg_type(s.argv[1]);
        /* both operands can be 1, 2, 3 or 4*/
        return (origin==1 || origin==2) &&
               (destination==1 || destination==2 || destination==3);
    }

	/* if the operation is - jmp, bne or jsr */
    if (strcmp(op,"jmp")==0 || strcmp(op,"bne")==0 || strcmp(op,"jsr")==0) {
        arg_type destination = get_arg_type(s.argv[0]);
        /* both operands can be 1, 2, 3 or 4*/
        return destination==1 || destination==3;
    }
	
	/* if the operation is - prn */
    if (strcmp(op,"prn")==0) {
        arg_type destination = get_arg_type(s.argv[0]);
        /* both operands can be 1, 2, 3 or 4*/
        return destination==0 || destination==1 || destination==2 || destination==3;
    }

    /* the remaining operations are rts and hlt which can not have wrong arg types because they have no args */
    return TRUE;
}



/* the find_errors function only find errors in the composition of the sentence,
 * it does not check if a label already exist or if the arguments are real.
 */
char* find_error(sentence s) {
    /* find error in the label name composition (if there is a label) */
    if (s.label != NULL) {
        if (is_conserved_word(s.label))
            return "Label Is a Conserved Word";
        if (!is_valid_name(s.label)) 
            return "Name Must Strart With A Latin Letter And Consist Of Only Latin Letters Or Numbers";
    }
    
    /* find errors in the operation composition */
    if (!is_valid_operation(s.operation)) 
        return "Unknown Operation";
    
    /* finds errors in the amount of arguments and their types */
    if (!is_valid_argc(s))
        return "Incorrect Amount Of Arguments For This Operation";
    if (!valid_arg_types(s))
        return "Invalid Argument Types For This Operation";
    
    return NULL; /* return null if no error was found */
}
