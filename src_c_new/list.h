#ifndef __NVM_LIST__
#define __NVM_LIST__

#define DECLARE_LIST(VALUE_TYPE)\
struct ##VALUE_TYPE##_list_node {\
    struct ##VALUE_TYPE##* ele;\
    struct ##VALUE_TYPE##_list_node* next;\
    struct ##VALUE_TYPE##_list_node* prev;\
}\
\
struct ##VALUE_TYPE##list_head {\
    struct ##VALUE_TYPE##_list_node* curr;\
    struct ##VALUE_TYPE##_list_node* first;\
    struct ##VALUE_TYPE##_list_node* last;\
}\
\
void insert_head_##VALUE_TYPE##_node(struct ##VALUE_TYPE##* ele, struct ##VALUE_TYPE##_list_head* list_head) {\
    ##VALUE_TYPE##_list_node* new_node = (##VALUE_TYPE##_list_node*)malloc(sizeof ##VALUE_TYPE##_list_node);\
    new_node->ele = ele;\
    new_node->next = list_head->first;\
    if (!list_head->first) {\
        list_head->last = new_node;\
        list_head->curr = new_node;\
    } else {\
        list_head->first->prev = new_node;\
    }\
    list_head->first = new_node;\
}\
\
void insert_tail_##VALUE_TYPE##_node(struct ##VALUE_TYPE##* ele, struct ##VALUE_TYPE##_list_head* list_head) {\
    ##VALUE_TYPE##_list_node* new_node = (##VALUE_TYPE##_list_node*)malloc(sizeof ##VALUE_TYPE##_list_node);\
    new_node->ele = ele;\
    new_node->prev = list_head->last;\
    if (!list_head->last) {\
        list_head->first = new_node;\
        list_head->curr = new_node;\
    } else {\
        list_head->last->next = new_node;\
    }\
    list_head->last = new_node;\
}\
\
void insert_after_curr_##VALUE_TYPE##_node(struct ##VALUE_TYPE##* ele, struct ##VALUE_TYPE##_list_head* list_head) {\
    if (!list_head->curr) {\
        insert_head_##VALUE_TYPE##_node(ele, list_head);\
        return;\
    }\
    ##VALUE_TYPE##_list_node* new_node = (##VALUE_TYPE##_list_node*)malloc(sizeof ##VALUE_TYPE##_list_node);\
    new_node->ele = ele;\
    if (list_head->curr->next) {\
        list_head->next->prev = new_node;\
        new_node->next = list_head->curr->next;\
    } else {\
        list_head->last = new_node;\
    }\
    list_head->curr->next = new_node;\
    new_node->prev = list_head->curr;\
}\
\
void insert_before_curr_##VALUE_TYPE##_node(struct ##VALUE_TYPE##* ele, struct ##VALUE_TYPE##_list_head* list_head) {\
    if (!list_head->curr) {\
        insert_tail_##VALUE_TYPE##_node(ele, list_head);\
        return;\
    }\
    ##VALUE_TYPE##_list_node* new_node = (##VALUE_TYPE##_list_node*)malloc(sizeof ##VALUE_TYPE##_list_node);\
    new_node->ele = ele;\
    if (list_head->curr->prev) {\
        list_head->prev->next = new_node;\
        new_node->prev = list_head->curr->prev;\
    } else {\
        list_head->start = new_node;\
    }\
    list_head->curr->prev = new_node;\
    new_node->next = list_head->curr;\
}\
\
struct ##VALUE_TYPE##* access_list_current(struct ##VALUE_TYPE##_list_head* list_head) {\
    return list_head->curr->ele;\
}\
\
void ##VALUE_TYPE##_list_next_ele(struct ##VALUE_TYPE##_list_head* list_head) {\
    if (list_head->curr && list_head->curr->next) {\
        list_head->curr = list_head->curr->next;\
    }\
}\
\
void ##VALUE_TYPE##_list_prev_ele(struct ##VALUE_TYPE##_list_head* list_head) {\
    if (list_head->curr && list_head->curr->prev) {\
        list_head->curr = list_head->curr->prev;\
    }\
}\

#endif // !__NVM_LIST__
