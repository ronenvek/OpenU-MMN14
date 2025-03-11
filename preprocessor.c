#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "sentences.h"
#include "utils.h"
#include "preprocessor.h"

/* +1 for the \n */
#define MAX_LINE_LENGTH 81


/* mcrNode for macro linked list */
typedef struct mcrNode {
    char* name;
    char* macro;
    struct mcrNode* next;
} mcrNode;


/* Function to add a new mcrNode to the end of the macro list */
void add_macro(mcrNode** mcrHead, char* name, char* macro) {
    mcrNode* new_node = (mcrNode*)malloc(sizeof(mcrNode)); /* allocate memory to the node */
    if (new_node == NULL) {
        printf("Memory allocation failed.\n");
        exit(1);
    }
	
	/* assign the values into the new node */
    new_node->name = name;
    new_node->macro = macro;
    new_node->next = NULL;
	
	/* add the new node to the list */
    if (*mcrHead == NULL) 
        *mcrHead = new_node; /* if the list is empty, make the new node the mcrHead */
    else {
        mcrNode* current = *mcrHead;
        while (current->next != NULL) 
            current = current->next; /* go to the end of the list */
        current->next = new_node; /* add the new node to the end of the list */
    }
}

/* Function to get the macro corresponding to a given name */
char* get_macro(mcrNode* mcrHead, char* name) {
    mcrNode* current = mcrHead;
    
    while (current != NULL) { /* go through the list */
        if (strcmp(current->name, name) == 0) { /* return the macro if the name matches */
            return current->macro;
        }
        current = current->next;
    }
    return NULL; /* Return NULL if the name is not found */
}

/* function to free a macro list */
void free_macro_list(mcrNode* current) {
    while (current != NULL) {
        mcrNode* temp = current;
        current = current->next;
        free(temp->name);
        free(temp->macro);
        free(temp);
    }
}

/* this functions returns the text in the am file after handeling the macros */
char* create_am_file(char* text) {
    char* am_text; /* the output */
    mcrNode* mcrHead; /* macro list to keep track of all of the macros */
    char* macro_name;
    char* macro; /* the content of the macro */
    int in_macro;
    int found_error;
    char* line;
    
    am_text = NULL;
    mcrHead = NULL;
    macro_name = NULL;
    macro = NULL;
    in_macro = FALSE;
    found_error = FALSE;

    /* go through every line in the file using strtok */
    line = strtok(text, "\n");
    while (line != NULL) {
    	sentence sent;
    	int line_num;
    	char* macro_content;
    	
        line_num = find_line_number(text, line); /* get the line number */
        sent = to_sentence(line);
        
        /* blank lines and comments are ignored and errors are handled later */
        if (sent.is_blank) {
            free_sentence(sent);
            line = strtok(NULL, "\n");
            continue;
        }
        
        macro_content = get_macro(mcrHead, sent.operation);
		
        /* the line can't be longer than 80 chars */
        if (strlen(line) > MAX_LINE_LENGTH) {
            free_sentence(sent);
            printf("line %d: ERROR: line length exceeds 80 chars", line_num);
            found_error = TRUE;
            line = strtok(NULL, "\n");
            continue;
        }
      
        /* Copy macro to text */
        if (macro_content != NULL) {
        	free_sentence(sent);
            am_text = merge_strings(am_text, macro_content); /* replace macro name with content */
            am_text = merge_strings(am_text, "\n"); /* start new line */
            line = strtok(NULL, "\n");
            continue;
        }
        
        /* set macro */
        if (in_macro) {
            if (strcmp(sent.operation, "endmcr") == 0) {
                if (sent.label != NULL) { /* labels on macros are ignored */
                    printf("line %d: WARNING: Label Ignored When Put On endmcr Lines.", line_num);
                }
                if (sent.argc != 0) { /* endmcr shouldn't have any arguments */
                    sent.err = "Too Many Arguments (0 Argument Expected)";
                    printf("line %d: ERROR: Too Many Arguments (0 Argument Expected)", line_num);
                    free_sentence(sent);
                    found_error = TRUE;
                    line = strtok(NULL, "\n");
                    continue;
                }
                add_macro(&mcrHead, macro_name, macro); /* add macro to the list */
                macro_name = NULL; /* reset macro name and content */
                macro = NULL;
                in_macro = FALSE;
            } else { /* we are in the macro and it didn't end */
                /* add line to the macros content */
                macro = merge_strings(macro, line);
                macro = merge_strings(macro, "\n");
            }
        } else { /* not in a macro */
            if (strcmp(sent.operation, "mcr") == 0) { /* macro has started */
                if (sent.label != NULL) { /* labels on macros are ignored */
                    printf("line %d: WARNING: Label Ignored When Put On mcr Lines.", line_num);
                }

                /* macro must have 1 argument which is its name */
                if (sent.argc == 0) { /* handle 0 args */
                    printf("line %d: error: Missing Macro Name", line_num);
                    free_sentence(sent);
                    found_error = TRUE;
                    line = strtok(NULL, "\n");
                    continue;
                } else if (sent.argc > 1) { /* handle more than one args */
                    printf("line %d: error: Too Many Arguments (1 Argument Expected)", line_num);
                    free_sentence(sent);
                    found_error = TRUE;
                    line = strtok(NULL, "\n");
                    continue;
                }
                macro_name = strdup(sent.argv[0]); /* set macro name to be the first arg */
                in_macro = TRUE;
            } else { /* not in a macro and the line doesn't use a macro */
                am_text = merge_strings(am_text, line); /* add unmodified line to the am file */
                am_text = merge_strings(am_text, "\n"); /* start new line */
            }
        }
        free_sentence(sent); /* free allocated memory for the sentence */
        line = strtok(NULL, "\n");
    }

    /* Free memory allocated for the linked list */
    free_macro_list(mcrHead);
	
    /* return am_text only if no errors were found */
    if (!found_error)
        return am_text;
	
    /* return null if errors were found */
    free(am_text);
    return NULL;
}

