#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "utils.h"


#define TRUE 1
#define FALSE 0


/* Function to append a line to a string */
char* merge_strings(char* str1, char* str2) {
	
	int str_length;
    int line_length;
    
    int new_size;
    char* new_str;

    if (str2==NULL) return str1;
    
    str_length = (str1 == NULL) ? 0 : strlen(str1);
    line_length = strlen(str2);

    /* Calculate the new size required for the concatenated string */
    new_size = str_length + line_length + 1; /* +1 for the null terminator */

    /* Reallocate memory for the concatenated string */
    new_str = (char*)realloc(str1, new_size);
    if (new_str == NULL) {
        printf("Memory allocation failed.\n");
        exit(1);
    }

    /* Copy the new line to the end of the concatenated string */
    strcpy(new_str + str_length, str2);

    return new_str;
}



int is_integer(char *str) {
    /* Check if the string is empty */
    if (str == NULL || *str == '\0')
        return FALSE;

    /* Check for optional sign */
    if (*str == '+' || *str == '-')
        str++;

    /* Check if the remaining characters are all digits */
    while (*str) {
        if (!isdigit(*str))
            return FALSE;
        str++;
    }

    return TRUE;
}

int to_integer(char* str) {
    int result=1;

    if (*str == '+') str++;
    else if (*str == '-') {
        str++;
        result = -1;
    }
	
    return result * atoi(str);
}


/* returns weather the given variable name is valid, meaning starts with a letter and continues with letters or numbers and its length doesn't esceed 32*/
int is_valid_name(char* name) {
    /* 65 to 90 in ascii are the upper case letters and 97 to 122 are the lower case letters*/
    if (name==NULL || !((65 <= *name && *name <= 90) || (97 <= *name && *name <= 122)))
        return FALSE;

    name++;

    for (; *name; name++)
        /* 48 to 57 in ascii are the numbers 0 to 9*/
        if (!((65 <= *name && *name <= 90) || (97 <= *name && *name <= 122) || (48 <= *name && *name <= 57)))
            return FALSE;

    return strlen(name)<=32; /* the length of any name must not exceed 32 */
}


/* returns the amount od lines in a given string */
int count_lines(char* str) {
	int count;

    if (str==NULL || *str=='\0') return 0;
    
    count=1;
    while (*str) {
        if (*str=='\n' && *(str+1)!='\0') count++;
        str++;
    }
    return count;
}

/* the function recieves 2 strings and returns the line number of the first string in which the second string apears */
int find_line_number(char* str, char* line) {
        char* p;
        int line_number;
        
        p = str;
        line_number = 1;

        while (*p != '\0') {
            char* line_start;
            char* line_end;
            
            line_start = p;
            line_end = strchr(line_start, '\n');

            /* If line_end is NULL, it means we reached the end of the text */
            if (line_end == NULL) {
                line_end = strchr(line_start, '\0');
            }

            /* Check if the line matches */
            if ((line_end - line_start) == strlen(line) && strncmp(line_start, line, line_end - line_start) == 0) {
                return line_number;
            }

            /* Move to the next line */
            p = line_end + 1;
            line_number++;
        }

        return -1; /* Line not found */
    }


/* this function returns the contens of a given file as a string */
char* read_file(char* filename) {
    FILE *file;
    char *file_contents;
    long file_size;
    
    file = fopen(filename, "r");
    if (file == NULL) {
        return NULL;
    }

    /* Determine the file size */
    fseek(file, 0, SEEK_END);
    file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    /* Allocate memory for the file contents */
    file_contents = (char*)malloc(file_size + 1); /* +1 for null terminator */
    if (file_contents == NULL) {
        fclose(file);
        perror("Memory allocation error");
        return NULL;
    }

    /* Read the file contents into the allocated memory */
    if (fread(file_contents, 1, file_size, file) != file_size) {
        fclose(file);
        free(file_contents);
        perror("Error reading file");
        return NULL;
    }

    /* Null-terminate the string */
    file_contents[file_size] = '\0';

    fclose(file);
    return file_contents;
}


void write_file(char* filename, char* str) {
    /* Open the file in write mode */
    FILE* file = fopen(filename, "w+");
    if (file == NULL) {
        printf("Error opening file");
        exit(1);
    }

    /* Write the string to the file */
    if (fprintf(file, "%s", str) < 0) {
        printf("Error writing to file");
        fclose(file);
        exit(1);
    }

    /* Close the file */
    fclose(file);
}


/* this function gets  */
char* reformed_array_and_index(const char* name, int index) {
    int length;
    char* result;
    
    /* Calculate the length of the resulting string */
    length = strlen(name) + 6; /* 6 for maximum integer string length and '[' and '\0' */

    /* Allocate memory for the resulting string */
    result = (char*)malloc(length * sizeof(char));
    if (result == NULL) {
        printf("Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }

    /* Construct the resulting string */
    sprintf(result, "%s[%d", name, index);

    return result;
}


/* has the # before the number */
char* number_to_string(int num) {
    int temp;
    int digits;
    char* str;
    
    temp = num;
    digits = 0;
    
    do {
        temp /= 10;
        digits++;
    } while (temp != 0);
    
    /* Allocate memory for the string (+1 for the null terminator and +1 for the #) */
    str = (char*)malloc((digits + 2) * sizeof(char));
    if (str == NULL) {
        printf("Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }
    
    /* Convert the integer to a string */
    sprintf(str, "#%d", num);
    
    return str;
}

/* doesn't have the # before the number */
char* data_number_to_string(int num) {
    int temp;
    int digits;
    char* str;
    
    /* calculate the length of the number */
    temp = num;
    digits = 0;
    do {
        temp /= 10;
        digits++;
    } while (temp != 0);

    /* allocate memory for the string (+1 for the null terminator) */
    str = (char*)malloc((digits + 1) * sizeof(char));
    if (str == NULL) {
        printf("Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }

    /* convert the integer to a string */
    sprintf(str, "%d", num);

    return str;
}



char* strdup(char* src) {
    /* calculate the length of the source string */
    int len; /* length of the new string */
    char* dst; /* destination */
    
    len = strlen(src) + 1; /* +1 for null terminator */
    /* allocate memory for the duplicated string */
    dst = (char*)malloc(len);

    /* check if memory allocation was successful */
    if (dst != NULL) 
        /* copy the source string to the newly allocated memory */
        strcpy(dst, src);

    return dst; /* return a pointer to the duplicated string */
}







