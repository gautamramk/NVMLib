#include "../list.h"
#include <stdio.h>

DECLARE_LIST(int)

void main() {
    int_list_head* list_head = create_int_list();
    int* numbers = (int*)malloc(20*sizeof(int));
    for (int i = 0; i < 20; i++) {
        numbers[i] = i;
        if (i < 10)
            insert_tail_int_node(&numbers[i], list_head);
    }
    int_list_goto_head(list_head);
    while (list_head->curr != NULL) {
        printf("%d\n", *access_int_list_current(list_head));
        int_list_next_ele(list_head);
    }
    int_list_prev_ele(list_head);
    printf("%d\n", *access_int_list_current(list_head));
    insert_before_curr_int_node(&numbers[11], list_head);
    int_list_prev_ele(list_head);
    printf("%d\n", *access_int_list_current(list_head));
    printf("_______\n");
    while (list_head->curr != NULL) {
        printf("%d\n", *access_int_list_current(list_head));
        int_list_next_ele(list_head);
    }
}