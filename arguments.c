#include <stdio.h>
#include <string.h>
#include "arguments.h"
#include "utils.h"


#define TRUE 1
#define FALSE 0


char* registers[8] = {
    "r0",
    "r1",
    "r2",
    "r3",
    "r4",
    "r5",
    "r6",
    "r7"
};


/* this function does not check the validity of the number */
int is_number(char* arg) {
    return *arg=='#';
}

/* returns weather the given arg is one of the registers r0-r8 */
int is_register(char* arg) {
    int i;
    for (i=0; i<8; i++)
        if (strcmp(arg, registers[i])==0) return TRUE;
    return FALSE;
}

/* returns weather the arg is an array and index argument
 * (it doesn't check the validity of the arg)
 */
int is_array_and_index(char* arg) {
    while (*arg) {
        if (*arg == '[') return TRUE;
        arg++;
    }
    return FALSE;
}


int is_data_integer(char* arg) {
    return is_integer(arg);
}


int is_data_string(char* arg) {
    return *arg=='"';
}

/* returns the arg_type of the given argument */
arg_type get_arg_type(char* arg) {
    if (is_number(arg)) return NUMBER;
    if (is_register(arg)) return REGISTER;
    if (is_array_and_index(arg)) return ARRAY_AND_INDEX;
    if (is_data_integer(arg)) return DATA_INTEGER;
    if (is_data_string(arg)) return DATA_STRING;
    return VARIABLE;
}
