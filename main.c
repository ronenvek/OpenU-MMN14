#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "data_nodes.h"
#include "arguments.h"
#include "first_pass.h"
#include "second_pass.h"
#include "sentences.h"
#include "utils.h"
#include "preprocessor.h"



/* this function compiles the given file (creates the .ob, .ent and .ext files) */
void compile(char* filename) {
    char filename_with_extension[103];
    
    char* as_text;
    char* am_text;
    
    char am_filename[103];
    
    label_node* label_head;
    extern_node* extern_head;
    entry_node* entry_head;
    
    char* temp;
    
    int IC;
    int DC;
    
    int has_error;
    
    second_pass_result* result;
	
    printf("\ncompiling %s.as\n", filename);
    
    /* add the .as extention to the filename */
    strcpy(filename_with_extension, filename);
    strcat(filename_with_extension, ".as");
    
    /* read the .as file */
    as_text = read_file(filename_with_extension);
    
    if (as_text==NULL) {
    	printf("%s.as not found\n\n", filename);
    	return;
    }
	
	/* create am file */
    am_text = create_am_file(as_text);
    if (am_text == NULL) {
        printf("an error in the preprocessor prevented creation of .am file\n\n");
        free(as_text);
        return;
    }
    strcpy(am_filename, filename);
    strcat(am_filename, ".am");
    write_file(am_filename, am_text);
    
    /* the labe, extern and entry lists */
    label_head = NULL;
    extern_head = NULL;
    entry_head = NULL;

    /* in the first pass we use strtok which modifies the text so we copy it to another place so we can use am_text again */
    temp = NULL;
    temp = merge_strings(temp, am_text);
	
    IC=0;
    DC=0;
	
    has_error = first_pass(as_text, temp, &IC, &DC, &label_head, &extern_head, &entry_head);
    
    /* free allocated memory for temp */
    free(temp);
	
    result = second_pass(as_text, am_text, IC, DC, has_error, &label_head, &extern_head, &entry_head);
    
    if (result != NULL) { /* if the code has no erros */

        if (result->ent_file != NULL) {
            /* create the .ent file */
            char ent_filename[103];
            strcpy(ent_filename, filename);
            strcat(ent_filename, ".ent");
            write_file(ent_filename, result->ent_file);
            free(result->ent_file);
        }

        if (result->ext_file != NULL) {
            /* create the .ext file */
            char ext_filename[103];
            strcpy(ext_filename, filename);
            strcat(ext_filename, ".ext");
            write_file(ext_filename, result->ext_file);
            free(result->ext_file);
        }

        /* create the .ob file */
        if (result->machine_code != NULL) {
            /* create the .ext file */
            char ob_filename[103];
            strcpy(ob_filename, filename);
            strcat(ob_filename, ".ob");
            write_file(ob_filename, result->machine_code);
            free(result->machine_code);
        }
        printf("compilation succeeded!\n\n");
	} 
	else 
		printf("compilation failed\n\n");
	
	/* free allocated memory */
	
    free(as_text);
    free(am_text);
    free(result);
    
    free_labels(label_head);
    free_externs(extern_head);
    free_entrys(entry_head);
}


int main(int argc, char* argv[]) {
    int i;
    
    if (argc==1) {
    	printf("error: no files given\n");
    	return 1;
    }
    
    for (i=1; i<argc; i++)  /* go through every given filename */
    	compile(argv[i]); /* compile each every given file */

    return 0;
}

