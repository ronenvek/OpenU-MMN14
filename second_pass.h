typedef enum {ABSOLUTE_ARE=0, EXTERNAL_ARE=1, RELOCATABLE_ARE=2} ARE_field;


second_pass_result* second_pass(char* as_text, char* am_text, int IC, int DC, int has_error, label_node** label_head, extern_node** extern_head, entry_node** entry_head);


extern struct opcode_list_struct {
    char* name;
    char* opcode;
} opcode_list[16];


extern struct adrs_mode_struct {
    arg_type type;
    char* num;
} addressing_mode[4];



extern struct reg_list_struct {
    char* name;
    char* str;
} register_list[8];
