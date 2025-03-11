#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "sentences.h"
#include "utils.h"
#include "errors.h"
#include "arguments.h"
#include "preprocessor.h"
#include "data_nodes.h"
#include "first_pass.h"
#include "second_pass.h"

/* macro to add a string to the end of the result */
#define ADD_TO_RESULT(str) result = merge_strings(result, str)


/* a list containing the binary opcode for every instruction operation */
struct opcode_list_struct opcode_list[16] = {
    {"mov", "0000"},
    {"cmp", "0001"},
    {"add", "0010"},
    {"sub", "0011"},
    {"not", "0100"},
    {"clr", "0101"},
    {"lea", "0110"},
    {"inc", "0111"},
    {"dec", "1000"},
    {"jmp", "1001"},
    {"bne", "1010"},
    {"red", "1011"},
    {"prn", "1100"},
    {"jst", "1101"},
    {"rts", "1110"},
    {"hlt", "1111"},
};


/* a list containing the binary addressing mode for every variable type */
struct adrs_mode_struct addressing_mode[4] = {
    {NUMBER,           "00"},
    {VARIABLE,         "01"},
    {ARRAY_AND_INDEX,  "10"},
    {REGISTER,         "11"},
};


/* a list containing the binary representation for every register */
struct reg_list_struct register_list[8] = {
    {"r0", "000"},
    {"r1", "001"},
    {"r2", "010"},
    {"r3", "011"},
    {"r4", "100"},
    {"r5", "101"},
    {"r6", "110"},
    {"r7", "111"}
};


/* turns a number to a num_bits number of bits binary number using 2's compliment */
char* decimal_to_n_bit_binary(int num, int num_bits) {
    /* Allocate memory for the binary string */
    int index;
    char* result;
    int i;
    
    result = (char*)malloc((num_bits + 1) * sizeof(char)); /* +1 for null terminator */
    
    if (result == NULL) {
        printf("Memory allocation failed\n");
        exit(1);
    }

    index = 0;

    /* Handle negative numbers using 2's complement */
    if (num < 0) 
        num = (1 << num_bits) + num; /* Equivalent to (2^num_bits + num) */

    /* Convert the number to binary */
    for (i = num_bits - 1; i >= 0; i--) {
        result[index++] = ((num >> i) & 1) ? '1' : '0';
    }
    result[index] = '\0'; /* Null-terminate the string */

    return result;
}


/* returns the binary opcode for the given operation */
char* get_opcode(char* operation) {
    int i;
    for (i=0; i<16; i++) /* go through all of the operations and their opcodes */
        if (strcmp(operation, opcode_list[i].name)==0) /* if the operation was found, reutrn the opcode */
            return opcode_list[i].opcode;
    return NULL;
}


/* returns the binary representation of the given register */
char* get_register_str(char* r) {
    int i;
    for (i=0; i<8; i++) /* go through all of the register and their binary representation */
        if (strcmp(r, register_list[i].name)==0) /* if the register was found, return it's binary representation */
            return register_list[i].str;
    return NULL;
}


/* returns the addressing mode of a given arg */
char* get_addressing_mode(char* arg) {
    arg_type type;
    int i;
    type = get_arg_type(arg); /* get the arg_type of the arg */
    for (i=0; i<4; i++) /* go through the addressing mode of every arg_type  */
        if (addressing_mode[i].type == type) /* if the arg_type wass found, return the addressing mode */
            return addressing_mode[i].num;
    return NULL;
}

/* recieves a sentence arg of type ARRAY_AND_INDEX and returns only the array name (for example: "name[12]" -> "name") */
char* get_array_name(char* str) {
    char* bracketPos;
    int lengthUntilBracket;
    char* result;
    
    /* Find the position of '[' in the string */
    bracketPos = strchr(str, '[');

    /* If '[' is not found, return the original string */
    if (bracketPos == NULL) {
        return str;
    }

    /* Calculate the length until the '[' character */
    lengthUntilBracket = bracketPos - str;

    /* Allocate memory for a new string to store the result */
    result = (char*)malloc(lengthUntilBracket + 1);

    /* Copy characters from the original string until '[' */
    strncpy(result, str, lengthUntilBracket);

    /* Null-terminate the result string */
    result[lengthUntilBracket] = '\0';

    return result;
}

/* recieves a string which is an array and index and returns only the index (for example: "name[12]" -> "12") */
char* get_array_index(char* arg) {
	char* indexStart;
	char* indexEnd;
	int indexLength;
	char* index;
	char* trimmedIndex;
	int len;
	
    indexStart = strchr(arg, '[');

    indexStart++; /* Move past '[' */

    /* Find the end of the index */
    indexEnd = strchr(indexStart, '\0');

    /* Calculate the length of the index */
    indexLength = indexEnd - indexStart;

    /* Allocate memory for the index */
    index = (char*)malloc(indexLength + 1);

    /* Copy the index characters into the new string */
    strncpy(index, indexStart, indexLength);

    /* Null-terminate the index string */
    index[indexLength] = '\0';

    /* Trim whitespace characters from the index */
    trimmedIndex = index;
    while (isspace(*trimmedIndex)) {
        trimmedIndex++;
    }
	
    len = strlen(trimmedIndex);
    while (len > 0 && isspace(trimmedIndex[len - 1])) {
        trimmedIndex[--len] = '\0';
    }
	
    /* If the trimmed index is empty, return NULL */
    if (strlen(trimmedIndex) == 0) {
        free(index);
        return NULL;
    }
	
    return trimmedIndex;
}


/* turns the given int to a 4 digit string, if the number's length is less that 4, before the numbers there are zeros untill it's 4 digits long */
char* int_to_four_digit_string(int number) {
    /* Allocate memory for the resulting string (plus one for the null terminator) */
    char* result = (char*)malloc(5 * sizeof(char));
    if (result == NULL) {
        printf("Memory allocation failed.\n");
        exit(EXIT_FAILURE);
    }
	
    /* Convert the integer to string */
    sprintf(result, "%04d", number);
	
    return result;
}

/* returns the given sentence in machine words */
char* to_words(sentence s, label_node** label_head, extern_node** extern_head, entry_node** entry_head, define_node** define_head) {
    int i;
    char* result;
    
    if (s.is_blank) return NULL; /* return null if the sentence is blank */
	
	/* .extern and .entry sentences don't create any machine words */
    if (strcmp(s.operation, ".extern")==0 || strcmp(s.operation, ".entry")==0) 
        return NULL;
    
    result = NULL;
    
    if (strcmp(s.operation, ".data")==0) { /* if the operation is .data */
        for (i=0; i<s.argc; i++) { /* go through all of the args of the sentence */
            char* arg;
            int num;
            char* num_in_binary;
            
            arg = s.argv[i];
            /* convert the integer string argument (or a defined value) to an int */
            if (get_define(*define_head, arg) != NULL) {
            	define_node* value_n;
				value_n = get_define(*define_head, arg);
                num = value_n->value;
            }
            else 
                num = to_integer(arg);
            
            num_in_binary = decimal_to_n_bit_binary(num, 14); /* get the 14 bit representation of the num */

            /* add the word to the result */
            ADD_TO_RESULT(num_in_binary);
            ADD_TO_RESULT("\n"); /* start new word */
            
            free(num_in_binary); /* free allocated memory for num_in_binary */
        }
        return result;
    }
    if (strcmp(s.operation, ".string")==0) { /* if the operation is .string */
        char* temp; 
        temp = s.argv[0]+1; /* +1 to skip the '"' at the start of a string */
        while (*temp != '\0') { /* go through every char in the string */
            int num;
            char* num_in_binary;
            
            num = (int)(*temp); /* convert the char to it's ascii value */
            num_in_binary = decimal_to_n_bit_binary(num, 14); /* get the 14 bit representation of the char */

            /* add the word to the result */
            ADD_TO_RESULT(num_in_binary);
            ADD_TO_RESULT("\n"); /* start new word */
            
            free(num_in_binary); /* free allocated memory for num_in_binary */
            temp++; /* advance to the next char */
        }
        ADD_TO_RESULT("00000000000000\n"); /* add the null terminator */
        return result;
    }
    
    ADD_TO_RESULT("0000"); /* the 4 left most bits of the default word are not used */
    
    ADD_TO_RESULT(get_opcode(s.operation)); /* add the opcode */

    /* add the addressing mode */
    if (s.argc == 0) {
        ADD_TO_RESULT("0000");
    }
    else if (s.argc == 1) {
        ADD_TO_RESULT("00");
        ADD_TO_RESULT(get_addressing_mode(s.argv[0]));
    }
    else {
        ADD_TO_RESULT(get_addressing_mode(s.argv[0]));
        ADD_TO_RESULT(get_addressing_mode(s.argv[1]));
    } 
    
    /* add the ARE field which is alwaise 0 in the first word of an instruction */
    ADD_TO_RESULT("00");
    
    /* go through all of the arguments in the sentence */
    for (i=0; i<s.argc; i++) {
    	char* arg;
    	arg_type type;
    
        ADD_TO_RESULT("\n"); /* start new word in the result */
        arg = s.argv[i];
        type = get_arg_type(arg);
        
        if (type == NUMBER) {
        	int num;
        	char* num_in_binary;
        	
            arg++; /* to skip the '#' in the start of a number argument */
            
            if (get_define(*define_head, arg) != NULL) 
                num = get_define(*define_head, arg)->value;
            else 
                num = to_integer(arg);
            
            num_in_binary = decimal_to_n_bit_binary(num, 12);
            
            ADD_TO_RESULT(num_in_binary); /* add the number in 12 bits */
            ADD_TO_RESULT("00"); /* add the are field which is 00 for a number argument */
            
            free(num_in_binary);
        }
        else if (type == VARIABLE) {
        	int label_address;
        	char* num_in_binary;
        
            if (get_extern(*extern_head, arg) != NULL) {
                ADD_TO_RESULT("00000000000001"); /*if the arg is external this will alwaise be the word */
                continue;
            }
            
            label_address = get_label(*label_head, arg)->line;
            
            num_in_binary = decimal_to_n_bit_binary(label_address, 12);
            
            ADD_TO_RESULT(num_in_binary);
            ADD_TO_RESULT("10"); /* add the ARE field of a relocatable memory address */
            
            free(num_in_binary);
        }
        else if (type == ARRAY_AND_INDEX) {

            char* array_name;
            char* array_index;
            char* num_in_binary;
            int index;
            
            array_name = get_array_name(arg);
            array_index = get_array_index(arg);
            
            
            if (get_extern(*extern_head, array_name) != NULL) {
                ADD_TO_RESULT("00000000000001\n"); /*if the arg is external this will alwaise be the word */
            }
            else {
            	int label_address;
            	char* num_in_binary;
            	
                label_address = get_label(*label_head, array_name)->line;
                
                num_in_binary = decimal_to_n_bit_binary(label_address, 12);
                
                ADD_TO_RESULT(num_in_binary);
                ADD_TO_RESULT("10\n"); /* add the ARE field of a relocatable memory address */
                
                free(num_in_binary);
            }
            
            /* add the index as a word */
            if (get_define(*define_head, array_index) != NULL) 
                index = get_define(*define_head, array_index)->value;
            else 
                index = to_integer(array_index);
			
			num_in_binary = decimal_to_n_bit_binary(index, 12);
			
            ADD_TO_RESULT(num_in_binary); /* add the index in 12 bits */
            ADD_TO_RESULT("00"); /* add the are field which is 00 for an index */
            
            free(num_in_binary);
            free(array_index);
            free(array_name);
        }
        else { /* meaning the arg is a register */
            if (s.argc==1){
                ADD_TO_RESULT("000000000");
                ADD_TO_RESULT(get_register_str(arg));
                ADD_TO_RESULT("00");
            }
            else {
                if (i==0){ /* meaning the arg is the origin operand */
                    if (get_arg_type(s.argv[1])==REGISTER) { /* meaning both operands are registers */
                        /*if both operands are registers they share a single word */
                        ADD_TO_RESULT("000000");
                        ADD_TO_RESULT(get_register_str(arg));
                        ADD_TO_RESULT(get_register_str(s.argv[1]));
                        ADD_TO_RESULT("00\n");
                        return result;
                    }
                    ADD_TO_RESULT("000000");
                    ADD_TO_RESULT(get_register_str(arg));
                    ADD_TO_RESULT("00000");
                } else { /* meaning the arg is the destination operand */
                    ADD_TO_RESULT("000000000");
                    ADD_TO_RESULT(get_register_str(arg));
                    ADD_TO_RESULT("00");
                }
            }
        }
    }
    ADD_TO_RESULT("\n");
    return result;
}


/* recieves a machine word and returns the word in encrypted 4 bit */
char* to_encrypted_four_bit(char* word) {
    char* result = NULL;
    
    while (*word != '\0') { /* untill the end of the word */
        if (*word == '0') { /* the left bit of the current 2 bits is 0 */
            word++;
            if (*word=='0') /* the right bit of the current two bits is 0 */
                ADD_TO_RESULT("*"); /* because '*' is "00" in the encryption */
            else /* the right bit of the current two bits is 1 */
                ADD_TO_RESULT("#"); /* because '#' is "01" in the encryption */
        }
        else { /* the left bit of the current 2 bits is 1 */
            word++;
            if (*word=='0') /* the right bit of the current two bits is 0 */
                ADD_TO_RESULT("%"); /* because '%' is "10" in the encryption */
            else /* the right bit of the current two bits is 1 */
                ADD_TO_RESULT("!"); /* because '!' is "11" in the encryption */
        }
        word++;
    }

    return result;
}


/* formats the IC and DC to be put at the top of the .ob file */
char* format_IC_DC(int IC, int DC) {
	char* str;
	
    /* Allocate memory for the resulting string */
    str = (char*)malloc(12 * sizeof(char)); /* 3 spaces + 2, 4 digit integers (maximum) + 1 string terminator = 12 */
    if (str == NULL) {
        printf("Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }

    /* Format the integers into the string */
    sprintf(str, "  %d %d\n", IC, DC);

    return str;
}


/* recieves the machine words in binary and returns the final .ob text */
char* to_ob_file(char* machine_words, int IC, int DC) {
    int address_counter;
    char* result; /* the output */
    char* formatted_IC_DC;
    char* word;
    
    address_counter = 100; /* addresses start at 100 */
    result = NULL;

    formatted_IC_DC = format_IC_DC(IC, DC);
    ADD_TO_RESULT(formatted_IC_DC); /* add the IC and DC to the result */
    free(formatted_IC_DC);
    
    /* split every line with strtok */
    word = strtok(machine_words, "\n");
    while (word != NULL) {
    	char* four_digit_string;
    	char* encrypted_4_bit_word;
        
        four_digit_string = int_to_four_digit_string(address_counter);
        ADD_TO_RESULT(four_digit_string); /* add the line number */
        ADD_TO_RESULT(" "); /* add spaces */
        encrypted_4_bit_word = to_encrypted_four_bit(word); /* encrypt word */
        ADD_TO_RESULT(encrypted_4_bit_word); /* add the encrypted word */
        ADD_TO_RESULT("\n"); /* start new line */

        address_counter++;
        
        /* free allocated space for the encrypted word */
        free(four_digit_string);
        free(encrypted_4_bit_word);
        
        word = strtok(NULL, "\n");
    }
    
    return result;
}


second_pass_result* second_pass(char* as_text, char* am_text, int IC, int DC, int has_error, label_node** label_head, extern_node** extern_head, entry_node** entry_head) {
    
    char* result;
    
    char* ent_text;
    char* ext_text;
    
    define_node* define_n;
    define_node** define_head;
    
    char* line;
    
    
    result = NULL;
    
    ent_text = NULL;
    ext_text = NULL;
    
    define_n = NULL;
    define_head = &define_n; /* create .define list */
    
    /* Using strtok to split the string every new line */
    line = strtok(am_text, "\n");
    
    while (line != NULL) {
    	int line_num;
        int i;
        sentence s;
    	
        line_num = find_line_number(as_text, line);

        s = to_sentence(line);

        if (s.is_blank || strcmp(s.operation, ".extern")==0) {
        	free_sentence(s);
            line = strtok(NULL, "\n");
            continue;
        }
        
        if (strcmp(s.operation, ".define")==0) { /* if the line is a define statement */
        	char* name;
        	char* value;
			
			
			if (s.label != NULL) { /* the defined value must be an integer */
                printf("line %d: warning: labels ignored when put on define statements\n", line_num);
                has_error = TRUE;
                free_sentence(s);
                line = strtok(NULL, "\n");
                continue;
            }
			
			
            /* check arguments validity */
            if (s.argc != 2) { /* if there arn't the expected 2 arguments */
                printf("line %d: error: define statement should be structured as such: \".define <name>=<value>\"\n", line_num);
                has_error = TRUE;
                free_sentence(s);
                line = strtok(NULL, "\n");
                continue;
            }
            
            name = s.argv[0];
            value = s.argv[1];

            if (!is_valid_name(name)) { /* if the name isn't valid */
                printf("line %d: error: Name Must Strart With A Latin Letter And Consist Of Only Latin Letters Or Numbers\n", line_num);
                has_error = TRUE;
                free_sentence(s);
                line = strtok(NULL, "\n");
                continue;
            }
            
            if (is_conserved_word(name)) { /* if the name is a conserved word */
                printf("line %d: error: instructions, operations, registers and other conserved words can't be defined\n", line_num);
                has_error = TRUE;
                free_sentence(s);
                line = strtok(NULL, "\n");
                continue;
            }
            
            if (!is_integer(value)) { /* the defined value must be an integer */
                printf("line %d: error: the defined value must be an integer\n", line_num);
                has_error = TRUE;
                free_sentence(s);
                line = strtok(NULL, "\n");
                continue;
            }
            
            add_define(define_head, name, to_integer(value)); /* if no errors were found, add the definition to the list */
            free_sentence(s);
            line = strtok(NULL, "\n");
            continue;
        }

        /* turns all of the data that uses a defined variable into the appropriate integers */
        else if (strcmp(s.operation, ".data")==0) {
            for (i=0; i<s.argc; i++) {
            	char* arg;
            	define_node* n;
            	
                arg = s.argv[i];
                n = get_define(*define_head, arg);
                if (n!=NULL) { /* if the arg is a defined value */
                    free(s.argv[i]);
                    s.argv[i] = data_number_to_string(n->value); /* put the integer into its place */
                }
            }
        }
		
        else if (strcmp(s.operation, ".entry")==0) { /* if the operation is .entry */
        	char* name;
        	label_node* l;
        	
        	int length;
        	int spacing;
        	
        	char* four_digit_string;
        
            name = s.argv[0];

           	l = get_label(*label_head, name);
            
            if (l==NULL) { /* if the entry is not defined in file */
                printf("line %d: error: cannot use .entry on a non existent label\n", line_num);
                has_error = TRUE;
                free_sentence(s);
                line = strtok(NULL, "\n");
                continue;
            }
            
            
            /* add the entry to the ent text */
            
            ent_text = merge_strings(ent_text, name); /* add the name to the ent text */

            /* put spaces between the name and the line number */
            length = strlen(name);
            spacing = length>9 ? 1 : 10-length;
            for (i=0; i<spacing; i++) 
                ent_text = merge_strings(ent_text, " ");
            
            four_digit_string = int_to_four_digit_string(l->line);
            
            ent_text = merge_strings(ent_text, four_digit_string); /* add the line number */
            ent_text = merge_strings(ent_text, "\n"); /* start new line */
            
            free(four_digit_string);
            
            free_sentence(s);
            line = strtok(NULL, "\n");
            continue;
        }
        
        for (i=0; i<s.argc; i++) { /* go through every argument in the current sentence */
        	char* arg;
        	arg_type type;
        	
            arg = s.argv[i];
            type = get_arg_type(arg);
            
            if (type == NUMBER) { /* if the arg is a number */
            	define_node* n;
            	
                arg++;
                
                n = get_define(*define_head, arg);
                
                if (n!=NULL) { /* if the number is a defined value, change it back to the integer */
                    free(s.argv[i]);
                    s.argv[i] = number_to_string(n->value);
                }
                
                else if (!is_integer(arg) && get_define(*define_head, arg)==NULL) { /* if the arg is not defined and is not a number, output an error */
                    printf("line %d: error: invalid integer\n", line_num);
                    has_error = TRUE;
                    free_sentence(s);
                    line = strtok(NULL, "\n");
                    continue;
                }
            }
            else if (type == VARIABLE) { /* if the type of the arg is a variable */
				
                extern_node* ext_n = get_extern(*extern_head, arg);
                if (ext_n != NULL) { /* if the variable is external */
                
                	int length;
                	int spacing;
                	
                    ext_text = merge_strings(ext_text, arg); /* add the name to the ext text */
                    
                    /* put spaces between the name and the line number */
                    length = strlen(arg);
                    spacing = length>9 ? 1 : 10-length;

                    for (i=0; i<spacing; i++) 
                        ext_text = merge_strings(ext_text, " ");

                    if (i==0){ /* if it is the first operator */
                        char* memory_address;
                        memory_address = int_to_four_digit_string(100+count_lines(result));
                        ext_text = merge_strings(ext_text, memory_address); /* add the line number */
                        ext_text = merge_strings(ext_text, "\n"); /* start new line */
                        free(memory_address);
                    } else { /* if it's the second operator */
                    	int first_op_len;
                    	char* memory_address;
                    	
                        first_op_len = number_of_machine_words_one_arg(get_arg_type(s.argv[0])); /* length (in machine words) of the first operator */
                        memory_address = int_to_four_digit_string(100+first_op_len+count_lines(result));
                        ext_text = merge_strings(ext_text, memory_address); /* add the line number */
                        ext_text = merge_strings(ext_text, "\n"); /* start new line */
                        free(memory_address);
                    }
                }
                
                else if (get_label(*label_head, arg)==NULL) { /* if the variable doesn't exist, raise an error */
                    printf("line %d: error: unknown variable: %s\n", line_num, arg);
                    has_error = TRUE;
                    free_sentence(s);
                    line = strtok(NULL, "\n");
                    break;
                }
            }
            else if (type == ARRAY_AND_INDEX) { /* if the variable is an array and index */
            	char* name;
            	char* index;
            	
            	extern_node* ext_n;
				define_node* n;
                
                /* seperate the name and the index */
                name = get_array_name(arg);
                index = get_array_index(arg);
				
				ext_n = get_extern(*extern_head, name);
				
                if (ext_n != NULL) { /* if the array is external */
                	int length;
                	int spacing;
                	
                    ext_text = merge_strings(ext_text, name); /* add the name to the ext text */

                    /* put spaces between the name and the line number */
                    length = strlen(name);
                    spacing = length>9 ? 1 : 10-length;

                    for (i=0; i<spacing; i++) 
                        ext_text = merge_strings(ext_text, " ");

                    if (i==0){ /* if it is the first operator */
                        char* memory_address;
                        memory_address = int_to_four_digit_string(100+count_lines(result));
                        ext_text = merge_strings(ext_text, memory_address); /* add the line number */
                        ext_text = merge_strings(ext_text, "\n"); /* start new line */
                        free(memory_address);
                    } else { /* if it's the second operator */
                    	int first_op_len;
                    	char* memory_address;
                    
                        first_op_len = number_of_machine_words_one_arg(get_arg_type(s.argv[0])); /* length (in machine words) of the first operator */
                        memory_address = int_to_four_digit_string(100+first_op_len+count_lines(result));
                        ext_text = merge_strings(ext_text, memory_address); /* add the line number */
                        ext_text = merge_strings(ext_text, "\n"); /* start new line */
                        free(memory_address);
                    }
                }
                
                else if (get_label(*label_head, name)==NULL) { /* if the array name doesn't exist, raise an error */
                    printf("line %d: error: unknown variable: %s\n", line_num, name);
                    has_error = TRUE;
                    free_sentence(s);
                    line = strtok(NULL, "\n");
                    continue;
                }


                /* check if the index is valid */
                
                n = get_define(*define_head, index);
                
                if (n!=NULL) { /* if the index is a define, turn it to an integer */
                    free(s.argv[i]);
                    s.argv[i] = reformed_array_and_index(name, n->value);
                }
                
                if (!is_integer(index) && n==NULL) { /* if the index is not an integer, raise an error */
                    printf("line %d: error: invalid index\n", line_num);
                    has_error = TRUE;
                    free_sentence(s);
                    line = strtok(NULL, "\n");
                    break;
                }
                free(name);
                free(index);
            }
        }
        if (i<s.argc) continue;
        
        if (!has_error) { /* generate the output file only if there in no error */
        	char* words = to_words(s, label_head, extern_head, entry_head, define_head);
            ADD_TO_RESULT(words);
            free(words);
        }
        
        /* free the sentence */
        free_sentence(s);
        line = strtok(NULL, "\n");
    }
	
	/* free the define list */
    free_defines(*define_head);
    
    if (!has_error) { /* if no error was found, we output result to be created into output files */
        second_pass_result* output = (second_pass_result*)malloc(sizeof(second_pass_result));
		
        char* encrypted_result = to_ob_file(result, IC, DC); /* encrypt all of the words and memory addresses */
        free(result); /* free allocated memory for the unencrypted result */

        output->machine_code = encrypted_result;
        output->ent_file = ent_text;
        output->ext_file = ext_text;

        return output; /* return all of the output files */
    }
    
    free(result);
    free(ext_text);
    free(ent_text);
    
    return NULL; /* return NULL if an error was found */
}


