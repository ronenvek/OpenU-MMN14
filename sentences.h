#ifndef SENTENCES_H
#define SENTENCES_H
#define TRUE 1
#define FALSE 0
#endif

typedef struct {
    char *label;
    char *operation;
    unsigned int argc; /* argument counter */
    char **argv; /* all provided arguments */
    int is_blank; /* is the line blank or a comment */
    char* err; /* the error message */
} sentence;


sentence to_sentence(char *line);
void free_sentence(sentence sntnc);
void print_sentence(sentence sntnc);