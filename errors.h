extern char* conserved_words[28];

extern char* valid_operations[21];

extern struct valid_argc_struct {
    char* name;
    int argc;
} valid_argc[19];


char* find_error(sentence s);

int is_conserved_word(char* word);
