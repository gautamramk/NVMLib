/**
 * File contains a example test program for the generic Hash map implemented
 * 
 * The Output of the program looks like:
 * Inserting : ONE 
 * 0 ---> size = 0 || 
 * 1 ---> size = 1 || ONE | 
 * 
 * Inserting : TWO 
 * 0 ---> size = 0 || 
 * 1 ---> size = 2 || ONE | TWO | 
 * 
 * Inserting : THREE 
 * 0 ---> size = 0 || 
 * 1 ---> size = 3 || ONE | TWO | THREE | 
 * 
 * Inserting : FOUR
 * 0 ---> size = 1 || FOUR | 
 * 1 ---> size = 0 || 
 * 2 ---> size = 1 || ONE | 
 * 3 ---> size = 2 || TWO | THREE | 
 * 
 * Inserting : FIVE 
 * 0 ---> size = 2 || FOUR | FIVE | 
 * 1 ---> size = 0 || 
 * 2 ---> size = 1 || ONE | 
 * 3 ---> size = 2 || TWO | THREE | 
 * 
 * Inserting : SIX 
 * 0 ---> size = 1 || FOUR | 
 * 1 ---> size = 1 || FIVE | 
 * 2 ---> size = 1 || SIX | 
 * 3 ---> size = 0 || 
 * 4 ---> size = 1 || ONE | 
 * 5 ---> size = 0 || 
 * 6 ---> size = 1 || TWO | 
 * 7 ---> size = 1 || THREE | 
 * 
 * Inserting : SEVEN 
 * 0 ---> size = 1 || FOUR | 
 * 1 ---> size = 1 || FIVE | 
 * 2 ---> size = 1 || SIX | 
 * 3 ---> size = 0 || 
 * 4 ---> size = 2 || ONE | SEVEN | 
 * 5 ---> size = 0 || 
 * 6 ---> size = 1 || TWO | 
 * 7 ---> size = 1 || THREE | 
 * 
 * Inserting : EIGTH 
 * 0 ---> size = 1 || FOUR | 
 * 1 ---> size = 1 || FIVE | 
 * 2 ---> size = 1 || SIX | 
 * 3 ---> size = 0 || 
 * 4 ---> size = 3 || ONE | SEVEN | EIGTH | 
 * 5 ---> size = 0 || 
 * 6 ---> size = 1 || TWO | 
 * 7 ---> size = 1 || THREE | 
 * 
 * Finding: FOUND TWO

 * MAP SIZE : Before deletion -- 6
 * Erased 1
 * Erased 2
 * MAP SIZE : After deletion -- 5
 * 0 ---> size = 1 || FOUR | 
 * 1 ---> size = 1 || FIVE | 
 * 2 ---> size = 1 || SIX | 
 * 3 ---> size = 0 || 
 * 4 ---> size = 3 || ONE | SEVEN | EIGTH | 
 * 5 ---> size = 0 || 
 * 6 ---> size = 0 || 
 * 7 ---> size = 1 || THREE | 
 * 
 * MAP SIZE : After reinsertion -- 7
 * 0 ---> size = 0 || 
 * 1 ---> size = 1 || FOUR | 
 * 2 ---> size = 1 || FIVE | 
 * 3 ---> size = 0 || 
 * 4 ---> size = 0 || 
 * 5 ---> size = 1 || SIX | 
 * 6 ---> size = 0 || 
 * 7 ---> size = 0 || 
 * 8 ---> size = 2 || SEVEN | EIGTH | 
 * 9 ---> size = 1 || ONE | 
 * 10 ---> size = 0 || 
 * 11 ---> size = 0 || 
 * 12 ---> size = 0 || 
 * 13 ---> size = 1 || TWO | 
 * 14 ---> size = 0 || 
 * 15 ---> size = 1 || THREE | 
 * 
**/

#include "../hashmap.h"

#include <stdio.h>
#include <string.h>

typedef struct a_st {
 int b;
 char a[10];
} att;


int compare(att *a, att *b) {
	if (a->b == b->b){
		return 0;
	}
	return 1;
}

void print_obj(att *entry) {
	printf("%s", entry->a);
}

int get_hash(att *entry) {
	return entry->b;
}

DECLARE_HASHMAP(att)
DEFINE_HASHMAP(att, compare, get_hash, free, realloc)

HASH_MAP_PRINT_FUNC(att, print_obj)

int main() {
 
 HASH_MAP(att) *map = HASH_MAP_CREATE(att)();
 
 att *x = (att *)malloc(sizeof(att));
 x->b = 1;
 strcpy(x->a, "ONE");
 
 att *y = (att *)malloc(sizeof(att));
 y->b = 3;
 strcpy(y->a, "TWO");
 
 att *z = (att *)malloc(sizeof(att));
 z->b = 8;
 strcpy(z->a, "THREE");
 
 
 att *a = (att *)malloc(sizeof(att));
 a->b = 5;
 strcpy(a->a, "FOUR");
 
 
 att *b = (att *)malloc(sizeof(att));
 b->b = 10;
 strcpy(b->a, "FIVE");
 
 
 att *c = (att *)malloc(sizeof(att));
 c->b = 7;
 strcpy(c->a, "SIX");
 
 
 att *d = (att *)malloc(sizeof(att));
 d->b = 9;
 strcpy(d->a, "SEVEN");
 
 att *e = (att *)malloc(sizeof(att));
 e->b = 17;
 strcpy(e->a, "EIGTH");
 
 HASH_MAP_INSERT(att)(map, &x, HMDR_FIND);
 printf("Inserting : ");
 print_obj(x);
 printf("\n");
 HASH_MAP_PRINT(att)(map);
 
 HASH_MAP_INSERT(att)(map, &y, HMDR_FIND);
 printf("Inserting : ");
 print_obj(y);
 printf("\n");
 HASH_MAP_PRINT(att)(map);
 
 HASH_MAP_INSERT(att)(map, &z, HMDR_FIND);
 printf("Inserting : ");
 print_obj(z);
 printf("\n");
 HASH_MAP_PRINT(att)(map);
 
 HASH_MAP_INSERT(att)(map, &a, HMDR_FIND);
 printf("Inserting : ");
 print_obj(a);
 printf("\n");
 HASH_MAP_PRINT(att)(map);
 
 HASH_MAP_INSERT(att)(map, &b, HMDR_FIND);
 printf("Inserting : ");
 print_obj(b);
 printf("\n");
 HASH_MAP_PRINT(att)(map);
 
 HASH_MAP_INSERT(att)(map, &c, HMDR_FIND);
 printf("Inserting : ");
 print_obj(c);
 printf("\n");
 HASH_MAP_PRINT(att)(map);
 
 HASH_MAP_INSERT(att)(map, &d, HMDR_FIND);
 printf("Inserting : ");
 print_obj(d);
 printf("\n");
 HASH_MAP_PRINT(att)(map);
 
 HASH_MAP_INSERT(att)(map, &e, HMDR_FIND);
 printf("Inserting : ");
 print_obj(e);
 printf("\n");
 HASH_MAP_PRINT(att)(map);

 att *x1 = (att *)malloc(sizeof(att));
 x1->b = 3;
 strcpy(x1->a, "TWO");
 
 if(HASH_MAP_FIND(att)(map, &x1)){
	printf("Finding: FOUND %s\n", x1->a);
 }
 
 printf("\nMAP SIZE : Before deletion -- %ld\n", map->size);
 
 att *x2 = (att *)malloc(sizeof(att));
 x2->b = 3;
 strcpy(x2->a, "TWO");
 
 if(HASH_MAP_ERASE(att)(map, x2)){
	if(HASH_MAP_FIND(att)(map, &x2)){
		printf("Finding %s\n", x2->a);
	} else {
		printf("Erased 1\n");
	}
 }
 
 x = (att *)malloc(sizeof(att));
 x->b = 3;
 strcpy(x->a, "TWO");
 
 if(HASH_MAP_ERASE(att)(map, x)){
	if(HASH_MAP_FIND(att)(map, &x)){
		printf("Finding %s\n", x->a);
	} else {
		printf("Erased 1\n");
	}
 } else {
	 printf("Erased 2\n");
	 
 }
 
 printf("MAP SIZE : After deletion -- %ld\n", map->size);
 
 HASH_MAP_PRINT(att)(map);
 
 y = (att *)malloc(sizeof(att));
 y->b = 3;
 strcpy(y->a, "TWO");
 HASH_MAP_INSERT(att)(map, &y, HMDR_FIND);
 
 printf("MAP SIZE : After reinsertion -- %ld\n", map->size);
 
 HASH_MAP_PRINT(att)(map);
 
 
 HASH_MAP_DESTROY(att)(map);
 
 return 0;
}
