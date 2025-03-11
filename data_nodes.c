#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sentences.h"
#include "utils.h"
#include "arguments.h"
#include "data_nodes.h"
#include "first_pass.h"

/* Function to create a new label_node */
label_node* create_label_node(char* name, int line, operation_type type) {
    label_node* new_node = (label_node*)malloc(sizeof(label_node)); /* allocate memory for the new node */
    
    if (new_node == NULL) {
        printf("Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }
    
    new_node->name = strdup(name); /* dupe the name into new node (allocates new memory for it) */
    
    if (new_node->name == NULL) {
        printf("Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }
    
    /* assign the appropriate values to the new node */
    new_node->line = line;
    new_node->type = type;
    new_node->next = NULL;
    
    return new_node;
}

/* function to get the label_node by it's name (NULL if it doesn't exist) */
label_node* get_label(label_node* head, char* name) {
    label_node *current = head;
    
    while (current != NULL) { /* go through the list */
        if (strcmp(current->name, name) == 0) /* if the name is found, return the current node */
            return current;
        
        current = (label_node*)current->next;
    }
    return NULL; /* Label not found */
}

/* Function to add a label to the linked list */
void add_label(label_node **head, char *name, int line, operation_type type) {
    /* Create a new label node */
    label_node *new_node = create_label_node(name, line, type);

    /* Add the new label to the end of the list */
    if (*head == NULL) 
        *head = new_node;

    else {
        label_node *current = *head;
        while (current->next != NULL) /* go to the end of the list */
            current = (label_node*)current->next;
        
        current->next = (struct label_node*)new_node; /* add the new node to the end of the list */
    }
}

/* Function to free memory allocated for the linked list */
void free_labels(label_node* head) {
    label_node* current = head;
    
    while (current != NULL) { /* go through each node in the list */
        label_node* temp = current;
        current = (label_node*)current->next;
        
        /* free the contents of the node */
        free(temp->name);
        free(temp);
    }
}


/* function to create a new extern_node */
extern_node* create_extern_node(char* name) {
    extern_node* new_node = (extern_node*)malloc(sizeof(extern_node)); /* allocate memory for the new node */
    
    if (new_node == NULL) {
        printf("Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }
    /* assign the values to the new node */
    new_node->name = strdup(name);
    new_node->next = NULL;
    
    return new_node; /* return the new node */
}

/* function to create a new entry_node */
entry_node* create_entry_node(char* name) {
    entry_node* new_node = (entry_node*)malloc(sizeof(entry_node)); /* allocate memory for the new node */
    
    if (new_node == NULL) {
        printf("Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }
    /* assign the values to the new node */
    new_node->name = strdup(name);
    new_node->next = NULL;
    
    return new_node; /* return the new node */
}

/* function to get the extern_node with the given name */
extern_node* get_extern(extern_node* head, char* name) {
    const extern_node* current = head;
    
    while (current != NULL) { /* go through every node in the list */
        if (strcmp(current->name, name) == 0) /* if the name is found, return the node */
            return (extern_node*)current;
        current = current->next;
    }
    return NULL; /* Node with given name not found */
}

/* function to get the entry_node with the given name */
entry_node* get_entry(entry_node* head, char* name) {
    const entry_node* current = head;
    
    while (current != NULL) { /* go through every node in the list */
        if (strcmp(current->name, name) == 0) /* if the name is found, return the node */
            return (entry_node*)current;
        
        current = current->next;
    }
    return NULL; /* Node with given name not found */
}

/* function to add a new extern_node to the end of the given list */
void add_extern(extern_node** head, char* name) {
    extern_node* new_node = create_extern_node(name); /* create new node */
    
    if (*head == NULL) 
        *head = new_node;
    
    else {
        extern_node* last = *head;
        while (last->next != NULL) {
            last = last->next;
        }
        last->next = new_node;
    }
}


/* function to add a new entry_node to the end of the given list */
void add_entry(entry_node** head, char* name) {
    entry_node* new_node = create_entry_node(name); /* create new node */
    
    if (*head == NULL) {
        *head = new_node;
    } else {
        entry_node* last = *head;
        while (last->next != NULL) {
            last = last->next;
        }
        last->next = new_node;
    }
}

/* function to free all of the allocated memory for an extern_node linked list */
void free_externs(extern_node* head) {
    while (head != NULL) { /* go through every node in the list */
        extern_node* temp = head;
        head = head->next;
        
        /* free the name and the node */
        free(temp->name);
        free(temp);
    }
}

/* function to free all of the allocated memory for an entry_node linked list */
void free_entrys(entry_node* head) {
    while (head != NULL) { /* go through every node in the list */
        entry_node* temp = head;
        head = head->next;
        
        /* free the name and the node */
        free(temp->name);
        free(temp);
    }
}


/* Function to create a new define_node */
define_node* create_define_node(char* name, int value) {
    /* Allocate memory for new node */
    define_node* new_node = (define_node*)malloc(sizeof(define_node));
    
    if (new_node != NULL) {
        /* Copy name and assign value */
        new_node->name = strdup(name);
        new_node->value = value;
        new_node->next = NULL;
    }
    return new_node; /* return the new node */
}

/* Function to get a define_node with the given name */
define_node* get_define(define_node* head, char* name) {
    /* go through the list */
    define_node* current = head;
    while (current != NULL) {
        /* compare names */
        if (strcmp(current->name, name) == 0) 
            return current; /* Return if found */
        
        current = current->next;
    }
    return NULL; /* Not found */
}

/* function to add a new define_node to the list */
void add_define(define_node** head, char* name, int value) {
    /* create a new node */
    define_node* new_node = create_define_node(name, value);
    if (new_node != NULL) {
        /* add to the beginning of the list */
        new_node->next = *head;
        *head = new_node;
    }
}

/* fnction to free the memory allocated for the list */
void free_defines(define_node* head) {
    define_node* current = head;
    
    while (current != NULL) { /* go through each node in the list */
        define_node* next = current->next;
        
        /* free the allocated memory for the name and the node */
        free(current->name);
        free(current);
        
        current = next;
    }
}
