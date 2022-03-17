



#include <stdio.h>
#include <unistd.h>
#include <pthread.h>




typedef struct Node_{
    size_t size;
    struct Node_ *next;
    struct Node_ *prev;
} Node;

void * bf_malloc(size_t size,int lock_version, Node **head, Node **tail);
void *remove_node(Node * curr,size_t size,Node **head, Node **tail);
void *ts_malloc_lock(size_t size);
void ts_free_lock(void *ptr);
void *ts_malloc_nolock(size_t size);
void ts_free_nolock(void *ptr);
void add(Node * freenode,Node **head, Node **tail);
void check_whether_to_merge(Node * first, Node * sec, Node **tail);



void merge(Node *freenode, Node **tail);
void my_free(void *ptr,Node **head, Node **tail);




Node * head_lock = NULL;
Node * tail_lock = NULL;


__thread Node * head_nolock = NULL;
__thread Node * tail_nolock = NULL;

