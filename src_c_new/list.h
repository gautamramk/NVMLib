#ifndef __NVM_LIST__
#define __NVM_LIST__

#define DECLARE_LIST(VALUE_TYPE)\
struct ##VALUE_TYPE##list_node {\
    struct ##VALUE_TYPE##*\
    struct ##VALUE_TYPE##_list_node* next;\
    struct ##VALUE_TYPE##_list_node* prev;\
}\
struct ##VALUE_TYPE##list_head {\
    struct ##VALUE_TYPE##_list_node* curr;\
    struct ##VALUE_TYPE##_list_node* first;\
    struct ##VALUE_TYPE##_list_node* last;\
}\
void insert_head_##VALUE_TYPE##_node(struct ##VALUE_TYPE##* ele, struct ##VALUE_TYPE##_list_head* list_head) {\
    \
}\
void insert_tail_##VALUE_TYPE##_node(struct ##VALUE_TYPE##* ele, struct ##VALUE_TYPE##_list_head* list_head) {\
\
}\

#endif // !__NVM_LIST__
