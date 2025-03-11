typedef enum {NUMBER=0, VARIABLE=1, ARRAY_AND_INDEX=2, REGISTER=3, DATA_INTEGER=4, DATA_STRING=5} arg_type;


arg_type get_arg_type(char* arg);
