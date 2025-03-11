#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sentences.h"


/* macro for skipping all white characters in a string */
#define SKIP_SPACES(line) for(; *line==' ' || *line=='\t' || *line=='\n'; line++)


/* returns weather a given line has a label */
int hasLabel(char *line) {
    for (;*line;line++) /* go through each char in the string */
        if (*line == ':') /* if there is a colon on the line return true (only a lable uses the colon) */
            return TRUE;
    return FALSE;
}

/* creates and returns a new sentence */
sentence create_sentence() {
    sentence s;
    
    /* Initialize all fields to default values */
    s.label = NULL;
    s.operation = NULL;
    s.argc = 0;
    s.argv = NULL;
    s.is_blank = FALSE;
    s.err = NULL;
    
    return s;
}

/* Function to add an argument to the sentence */
void add_arg(sentence *sent, char *arg) {
    if (sent->argc == 0) { /* if this is the first argument, regular mallloc */
        sent->argv = malloc(sizeof(char*));
    } else { /* else, realloc */
        sent->argv = realloc(sent->argv, (sent->argc + 1) * sizeof(char*));
    }
    if (sent->argv == NULL) { /* Handle memory allocation failure */
        printf("Error: Memory allocation failed\n");
        exit(EXIT_FAILURE);
    } /* add the new arg and increase argc by 1 */
    sent->argv[sent->argc++] = arg;
}


/* a function to free a sentence */
void free_sentence(sentence sntnc) {
	int i;
	
    /* Free dynamically allocated memory for label and operation */
    free(sntnc.label);
    free(sntnc.operation);
	
    /* Free dynamically allocated memory for each argument */
    if (sntnc.argc > 0 && sntnc.argv != NULL) {
        for (i = 0; i < sntnc.argc; i++) {
            free(sntnc.argv[i]);
        }
        free(sntnc.argv);
    }
}

/* returns weather the given line is blank (or a comment) */
int isBlank(char *line) {
    for (; *line==' ' || *line=='\t'; line++);
    return (*line=='\0' || *line==';');
}

/* the given line is expected to start from the name and the given sentence already has ".define" as the operation */
void to_define_sentence(sentence* sent, char* line) {
	int length;
	char* temp;
	char* name;
	char* value;

    /* Find the length of the name */
    length = 0;
    temp = line;
    while (*temp != '=' && *temp != ' ' && *temp != '\t' && *temp != '\n' && *temp != '\0') {
        length++;
        temp++;
    }

    /* malloc for the name */
    name = (char*)malloc((length + 1) * sizeof(char));
    if (name == NULL) {
        printf("memory allocation failed\n");
        exit(1);
    }
    
    /* copy the name into the variable */
    strncpy(name, line, length);
    name[length] = '\0'; /* Null-terminate the string */
    
    SKIP_SPACES(temp);
    if (*temp != '=') { /* if the next char after the name isn't '=' raise error */
        free(name);
        sent->err = "Invalid .define statement, Expected: \".define <name> = <value>\"";
        return;
    }
    temp++; /* skip the '=' */
    SKIP_SPACES(temp);
    line = temp;
    /* Find the length of the value */
    length = 0;
    temp = line;
    while (*temp != '=' && *temp != ' ' && *temp != '\t' && *temp != '\n' && *temp != '\0') {
        length++;
        temp++;
    }
    
    /* malloc for the value */
    value = (char*)malloc((length + 1) * sizeof(char));
    if (value == NULL) {
        printf("memory allocation failed\n");
        exit(1);
    }

    /* copy the value into the variable */
    strncpy(value, line, length);
    value[length] = '\0'; /* Null-terminate the string */

    SKIP_SPACES(temp);
    if (*temp != '\0') { /* if the line didn't end after the value there is a problem with the statement */
        free(value);
        free(name);
        sent->err = "Invalid .define statement, Expected: \".define <name> = <value>\"";
        return;
    }
    /* the name and the value of the .define statement will be saved as two arguments in the sentence */
    add_arg(sent, name);
    add_arg(sent, value);
}


/* creates a sentence without a label */
sentence to_sentence_no_label(char* line) {
    
    sentence sent;
    int length;
    char* temp;
    char* operation;
    
    sent = create_sentence();
    
    if (line==NULL || *line=='\0') {
        sent.is_blank = TRUE;
        return sent;
    }
    
    SKIP_SPACES(line);
    
    /* Find the length of the operation */
    length = 0;
    temp = line;
    while (*temp != ' ' && *temp != '\t' && *temp != '\n' && *temp != '\0') {
        length++;
        temp++;
    }

    /* Allocate memory for the operation plus the null terminator */
    operation = (char*)malloc((length + 1) * sizeof(char));
    if (operation == NULL) {
        printf("memory allocation failed\n");
        exit(1);
    } /* Memory allocation failed */

    /* Copy the operation into the dynamically allocated memory */
    strncpy(operation, line, length);
    operation[length] = '\0'; /* Null-terminate the string */
    sent.operation = operation;
    /* .define statments behave differently that other statements */
    if (strcmp(operation, ".define")==0) {
        SKIP_SPACES(temp);
        to_define_sentence(&sent, temp);
        return sent;
    }
    line = temp;

    SKIP_SPACES(line);
    if (*line=='\0') return sent;

	
    while (1) { /* breaks when detected error or got to '\0' */
    	char* arg;
    
        SKIP_SPACES(line);
        length = 0;
        temp = line;
        while (*temp != ',' && *temp != ' ' && *temp != '\t' && *temp != '\n' && *temp != '\0') {
            /* if square brackets opened all chars until the closing bracket are a part of the same arg */
            if(*temp=='[') {
                temp++;
                while (*temp!=']') {
                    if (*temp=='\0') {
                        sent.err="Unclosed Square Brackets";
                        return sent;
                    }
                    length++;
                    temp++;
                }
            }
            /* if a quote has opened all chars until the last quote are a part of the same string arg */
            if(*temp=='"') {
                temp++;
                while (*temp!='"') {
                    if (*temp=='\0') {
                        sent.err="Unclosed Quotes";
                        return sent;
                    }
                    length++;
                    temp++;
                }
            }
            length++;
            temp++;
        }
        /* Allocate memory for the operation plus the null terminator */
        arg = (char*)malloc((length + 1) * sizeof(char));
        if (arg == NULL) {
            printf("memory allocation failed");
            exit(1);
        } /* Memory allocation failed */

        /* Copy the operation into the dynamically allocated memory */
        strncpy(arg, line, length);
        arg[length] = '\0'; /* Null-terminate the string */

        add_arg(&sent, arg); /* add arg to sentence */
        line = temp;
        SKIP_SPACES(line);

        if (*line == '\0') { /* if the line is complete */
            return sent;
        }
        
        if (*line != ',') { /* if the next argument apears without a comma it's an argument error */
            sent.err = "Missing Comma Between Arguments";
            return sent;
        }
        line++;
    }
    return sent;
}


/* gets a line, turns it to a sentence and returns the sentence.
 * the to_sentence function is intended to split a given line and tell everything you need to know about it.
 * it contains the label, operation, arguments, error (if one is found) and more information about the line.
 * it should contain any information you might need to know about the line and even removes white chars.
 * turning lines into sentences help with the readability and simplicity of the code by organizing all of the information about the line into one big variable
 */
sentence to_sentence(char *line) {
	int length;
	char* temp;
	char* label;
	char* error;
	sentence s;
	
	error = NULL;

    if (line==NULL || isBlank(line)) {
        sentence s = create_sentence();
        s.is_blank=TRUE;
        return s;
    }

    if (!hasLabel(line)) return to_sentence_no_label(line);
    
    /* fine label */
    SKIP_SPACES(line);
    
    /* Find the length of the label */
    length = 0;
    temp = line;
    while (*temp != ' ' && *temp != '\t' && *temp != '\n' && *temp != ':' && *temp != '\0') {
        length++;
        temp++;
    }
    
    if (*temp != ':') {
        error = "':' Must Be Attached To The End Of The Label";
    }
    /* Allocate memory for the operation plus the null terminator */
    label = (char*)malloc((length + 1) * sizeof(char));
    if (label == NULL) return create_sentence(); /* Memory allocation failed */

    /* Copy the operation into the dynamically allocated memory */
    strncpy(label, line, length);
    label[length] = '\0'; /* Null-terminate the string */

    line = temp;
    SKIP_SPACES(line);
    line++; /* skip ':' */
    
    s = to_sentence_no_label(line);
    s.err = error;
    s.label = label;
    return s;
}


