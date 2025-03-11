typedef enum {INSTRUCTION, DATA, STRING, EXTERN, ENTRY} operation_type;

typedef struct second_pass_result {
    char* machine_code;
    char* ent_file;
    char* ext_file;
} second_pass_result;

typedef struct {
    char* name;
    int line;
    operation_type type;
    struct label_node *next;
} label_node;

typedef struct extern_node {
    char* name;
    struct extern_node* next;
} extern_node;

typedef struct entry_node {
    char* name;
    struct entry_node* next;
} entry_node;

typedef struct define_node {
    char* name;
    int value;
    struct define_node* next;
} define_node;



label_node* create_label_node(char* name, int line, operation_type type);

label_node* get_label(label_node* head, char* name);

void add_label(label_node **head, char *name, int line, operation_type type);

void free_labels(label_node* head);

void print_labels(label_node *head);

extern_node* create_extern_node(char* name);

entry_node* create_entry_node(char* name);

extern_node* get_extern(extern_node* head, char* name);

entry_node* get_entry(entry_node* head, char* name);

void add_extern(extern_node** head, char* name);

void add_entry(entry_node** head, char* name);

void free_externs(extern_node* head);

void free_entrys(entry_node* head);

void print_externs(extern_node* head);

void print_entrys(entry_node* head);

define_node* create_define_node(char* name, int value);

define_node* get_define(define_node* head, char* name);

void add_define(define_node** head, char* name, int value);

void free_defines(define_node* head);

void print_defines(define_node* head);
