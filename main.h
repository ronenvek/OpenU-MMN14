typedef struct second_pass_result {
    char* machine_code;
    char* ent_file;
    char* ext_file;
} second_pass_result;

typedef struct {
    char* name;
    int line;
    operation_type type;
    struct label_node *next; /* Pointer to the next label_node */
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
