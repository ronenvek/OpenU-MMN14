#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sentences.h"
#include "utils.h"
#include "arguments.h"
#include "data_nodes.h"
#include "first_pass.h"
#include "preprocessor.h"
#include "errors.h"


/* returns the number of machine words a single given argument takes up */
int number_of_machine_words_one_arg(arg_type arg) {
    if (arg == ARRAY_AND_INDEX) /* array and index arg takes 2 words */
        return 2;
    return 1; /* every other arg takes one machine word */
}

/* returns the number of machine words a given instruction takes */
int instruction_number_of_machine_words(sentence s) {
    arg_type argt1;
    arg_type argt2;
    
    if (s.argc > 2) /* if there are more than two args reutrn -1 (meaning error) */
        return -1;
        
    if (s.argc == 0) /* if there are 0 args, the only word is the default word */
        return 1;
        
    if (s.argc == 1) { /* if there is one arg */
        argt1 = get_arg_type(s.argv[0]);
        return 1 + number_of_machine_words_one_arg(argt1); /* 1 default word + the amount of words the arg takes up */
    }
    
    /* if we reached this part there are 2 args */
    
    argt1 = get_arg_type(s.argv[0]);
    argt2 = get_arg_type(s.argv[1]);
    
    if (argt1 == REGISTER && argt2 == REGISTER) /* if both args are registers they share 1 word so with the default word, its a total of 2 */
        return 2;
    
    /* 1 default word + the amount of words the 2 args take up */
    return 1 + number_of_machine_words_one_arg(argt1) + number_of_machine_words_one_arg(argt2);
}

/* gets a data sentence and returns how many machine words the sentence takes up */
int data_number_of_machine_words(sentence s, operation_type type) {
    if (type == DATA)
        return s.argc; /* each argument is a number which takes up one word */
	
	/* if it's not .data it's .string */
    return strlen(s.argv[0]); /* there is one '"' in the start of a string so -1 but we also need the null operator so +1 so +0 overall*/
}

/* receives a name of an operation and returns the type of the operation */
operation_type get_operation_type(char* op) {
    if (strcmp(op, ".data") == 0)
        return DATA;
    if (strcmp(op, ".string") == 0)
        return STRING;
    if (strcmp(op, ".extern") == 0)
        return EXTERN;
    if (strcmp(op, ".entry") == 0)
        return ENTRY;
    return INSTRUCTION;
}

/* this function performs the first pass on the code, it puts all of the labels, externs and entrys in the given lists, it returns whether there is an error in the file or not.

*the function doesn't start creating the .ob file like in the algorithm suggested in the book.

the function handles some errors but not all of them, the secnond pass handles the rest

as_text and am_text are strings containing the .as and .am file's content
IC_ptr and DC_ptr are pointers who's values will be set to the instruction counter and data counter
label_head, extern_head and entry_node are the lists for labels
*/
int first_pass(char* as_text, char* am_text, int* IC_ptr, int* DC_ptr, label_node** label_head, extern_node** extern_head, entry_node** entry_head) {
    int IC, DC, has_error;
    char* line;
    label_node* current;
	
    IC = 0;
    DC = 0;
    has_error = FALSE;
    
    /* Using strtok to split the string every new line */
    line = strtok(am_text, "\n");
    
    while (line != NULL) {
        int line_num;
        sentence s;
        char* error;
        operation_type type;
        
        line_num = find_line_number(as_text, line); /* get the line number */
        s = to_sentence(line); /* turn the line into a sentence */
        
        /* blank lines should be ignored and defines are handled in the second pass */
        if (s.is_blank || strcmp(s.operation, ".define") == 0) {
        	free_sentence(s);
            line = strtok(NULL, "\n");
            continue;
        }

        /* handle errors with the sentence (not all errors are handled here) */
        if (s.err != NULL) {
            printf("line %d: error: %s\n", line_num, s.err);
            has_error = TRUE;
            free_sentence(s);
            line = strtok(NULL, "\n");
            continue;
        }
        error = find_error(s);
        if (error != NULL) {
            printf("line %d: error: %s\n", line_num, error);
            has_error = TRUE;
            free_sentence(s);
            line = strtok(NULL, "\n");
            continue;
        }
        
        type = get_operation_type(s.operation); /* get the operation type */
        
        if (type == ENTRY) { /* if it's a .entry sentence */
            if (s.label != NULL) /* if there is a label on the sentence */
                printf("line %d: WARNING: Label Ignored When Put On .entry Lines.", line_num);
            
            if (get_extern(*extern_head, s.argv[0]) != NULL) { /* error if the entry is an extern */
                printf("line %d: error: \"%s\" can't be both extern and entry\n", line_num, s.argv[0]);
                has_error = TRUE;
                free_sentence(s);
                line = strtok(NULL, "\n");
                continue;
            }
            
            if (get_entry(*entry_head, s.argv[0]) == NULL) /* add entry to list only if it is new */
                add_entry(entry_head, s.argv[0]);
        }
        else if (type == EXTERN) { /* if it's a .extern sentence */
            if (s.label != NULL)  /* if there is a label on the sentence */
                printf("line %d: WARNING: Label Ignored When Put On .extern Lines.\n", line_num);
            
            if (get_label(*label_head, s.argv[0]) != NULL) { /* error if the extern is a label */
                printf("line %d: error: \"%s\" can't be both extern and label\n", line_num, s.argv[0]);
                has_error = TRUE;
                free_sentence(s);
                line = strtok(NULL, "\n");
                continue;
            }
            if (get_entry(*entry_head, s.argv[0]) != NULL) {
                printf("line %d: error: \"%s\" can't be both extern and entry\n", line_num, s.argv[0]);
                has_error = TRUE;
                free_sentence(s);
                line = strtok(NULL, "\n");
                continue;
            }
            if (get_extern(*extern_head, s.argv[0]) == NULL) /* add the extern to the list if it's new */
                add_extern(extern_head, s.argv[0]);
        }
        else if (type == INSTRUCTION) {  /* if the operation is an instruction (mov/add/dec/...) */
            if (s.label != NULL) { /* if there is a label */

                if (get_extern(*extern_head, s.label) != NULL) { /* error if the label is an extern */
                    printf("line %d: error: \"%s\" can't be both extern and label\n", line_num, s.argv[0]);
                    has_error = TRUE;
                    free_sentence(s);
                    line = strtok(NULL, "\n");
                    continue;
                }
                if (get_label(*label_head, s.label) != NULL) { /* error if the label already exists */
                    printf("line %d: error: Label Already Exists\n", line_num);
                    has_error = TRUE;
                    free_sentence(s);
                    line = strtok(NULL, "\n");
                    continue;
                }
                add_label(label_head, s.label, IC, INSTRUCTION); /* add the label to the label list */
            }
            IC += instruction_number_of_machine_words(s); /* update the IC */
        }
        else {
            if (s.label != NULL) {
            	if (get_extern(*extern_head, s.label) != NULL) { /* error if the label is an extern */
                    printf("line %d: error: \"%s\" can't be both extern and label\n", line_num, s.label);
                    has_error = TRUE;
                    free_sentence(s);
                    line = strtok(NULL, "\n");
                    continue;
                }
                if (get_label(*label_head, s.label) != NULL) { /* if the label already exists */
                    printf("line %d: error: Label Already Exists\n", line_num);
                    has_error = TRUE;
                    free_sentence(s);
                    line = strtok(NULL, "\n");
                    continue;
                }
                if (type != ENTRY && type != EXTERN)
                    add_label(label_head, s.label, DC, type); /* labels to data are marked with - */
            }
            DC += data_number_of_machine_words(s, type); /* update the DC */
        }
        free_sentence(s);
        line = strtok(NULL, "\n");
    }

    /* add to all of the data labels the IC because they are supposed to come after the instructions and add 100 to every line because the memory starts at 100 */
    current = *label_head;
    while (current != NULL) {
        current->line += 100;
        if (current->type == DATA || current->type == STRING)
            current->line += IC;
        current = (label_node*)current->next;
    }

    *IC_ptr = IC;
    *DC_ptr = DC;
    
    return has_error;
}

