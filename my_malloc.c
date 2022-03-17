#include "my_malloc.h"
#include "assert.h"

size_t node_size = sizeof(Node);
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;


void *ts_malloc_lock(size_t size){
    pthread_mutex_lock(&lock);
    int lock_version = 0;
    void * ptr = bf_malloc(size,lock_version, &head_lock, &tail_lock);
    pthread_mutex_unlock(&lock);
    return ptr;
}
void ts_free_lock(void *ptr){
    pthread_mutex_lock(&lock);
    my_free(ptr, &head_lock, &tail_lock);
    pthread_mutex_unlock(&lock);
}


void * bf_malloc(size_t size,int lock_version, Node **head, Node **tail){
    if(size==0){
        perror("Cannot malloc 0 size memory.");
        return NULL;
    }
    Node *curr = *head;
    int bf = 0;
    while (curr) {
        if ((curr->size == size) || (curr->size > size && curr->size < (size + node_size))) {
            bf = 1;
            break;
        } else if (curr->size > size + node_size) {
            bf = 2;
        }
        curr = curr->next;
    }
    if (bf == 1) {
        return remove_node(curr,size,head,tail);
    }
    if (bf == 2) {
        curr = *head;
        while (curr) {
            if (curr->size > size + node_size) {
                assert(curr->size > size + node_size);
                Node * redundant = (Node *) ((void*)curr + node_size +size);
                redundant->size = curr->size - size - node_size;
                redundant->prev = curr->prev;
                redundant->next = curr->next;
                curr->size =size;
                curr->next =redundant;
                if(redundant->next) {
                    redundant->next->prev = redundant;
                }
                return remove_node(curr,size,head, tail);
            }
            curr = curr->next;
        }
    }
    if(bf == 0) {
//first lock version:0
        if (lock_version == 0) {
            void *ptr = sbrk(size + node_size);
            Node *block = (Node *) ptr;
            block->prev = NULL;
            block->next = NULL;
            block->size = size;
            return (void *) block + node_size;
        }
// second lock version:1
        else {
            pthread_mutex_lock(&lock);
            void *ptr = sbrk(size + node_size);
            pthread_mutex_unlock(&lock);
            Node *block = (Node *) ptr;
            block->prev = NULL;
            block->next = NULL;
            block->size = size;
            return (void *) block + node_size;

        }
    }
}

void *remove_node(Node * curr,size_t size,Node ** head, Node ** tail){
    if (curr->prev != NULL && curr->next !=NULL) {           //remove node in the middle
        curr->prev->next = curr->next;
        curr->next->prev = curr->prev;
    }
    else if(curr->prev == NULL && curr->next !=NULL){           //remove head
        *head = curr->next;
        curr->next->prev = NULL;
//        curr->next->prev = curr->prev;
    }
    else if(curr->prev != NULL && curr->next ==NULL){           //remove tail
        *tail =curr->prev;
        curr->prev->next = NULL;
//        curr->prev->next = curr->next;
    }
    else{
        *head = NULL;
        *tail = NULL;
    }
    curr->next = NULL;
    curr->prev = NULL;
    curr->size = size;
    return (void *) curr + node_size;
}



void add(Node * freenode,Node **head, Node **tail){
    if(*head == NULL && *tail == NULL){
        *head = freenode;
        *tail = freenode;
    }
    else{
        Node * curr = *head;
        while(curr != NULL){
            if(curr == *head && curr > freenode){     //add to the front
                freenode->next = curr;
                freenode->prev = NULL;
                curr->prev =freenode;
                *head = freenode;
                break;
            }
            else if(curr > freenode ){            //add in the middle
                Node * pre = curr->prev;
                pre->next = freenode;
                curr->prev = freenode;
                freenode->next = curr;
                freenode->prev = pre;
                break;
            }
            else if (curr->next == NULL){         //add at the end
                freenode->prev = curr;
                freenode->next = NULL;
                curr->next = freenode;
                *tail = freenode;
                break;
            }
            curr = curr->next;
        }
    }
}

void my_free(void *ptr, Node **head, Node **tail){
    if(ptr==NULL) return;
    Node *freenode = (Node *)(ptr - node_size);
    add(freenode,head,tail);
    merge(freenode,tail);
}



//void merge(Node *curr, Node **tail){
//        Node *pre = curr->prev;
//        Node *net = curr->next;
//        if(net!=NULL ){
//            if((void *)curr + curr->size + node_size == (void *)net){
//                curr->size = curr->size + net->size + node_size;
//                curr->next = net->next;
//                if(curr->next!=NULL){
//                    curr->next->prev = curr;
//            }
//                else{
//                    *tail = curr;
//                }
//            }
//        }
//        if(pre!=NULL){
//            if((void *)pre + pre->size + node_size == (void *)curr){
//                pre->size = pre->size + curr->size + node_size;
//                pre->next = curr->next;
//                if(pre->next!=NULL){
//                    pre->next->prev = pre;
//                }
//                else{
//                    *tail = pre;
//                }
//            }
//        }
//}

void merge(Node * freenode,Node **tail){
    Node *pre = freenode->prev;
    Node *net = freenode->next;
    if(pre != NULL ){
        check_whether_to_merge(pre,freenode,tail);
    }
    if(net != NULL){
        check_whether_to_merge(freenode,net,tail);
    }
}

void check_whether_to_merge(Node * first, Node * sec, Node **tail){
    void * end_of_first = (void*)first + first->size +node_size;
    void * begin_of_sec = (void*) sec;
    if(end_of_first == begin_of_sec){
        first->size = sec->size + node_size + first->size;
        first->next = sec->next;
        if(sec->next==NULL){
            *tail = first;
        }
        else{
            first->next->prev = first;
        }
    }

}


//Thread Safe malloc/free: non-locking version
void *ts_malloc_nolock(size_t size){
    int lock_version = 1;
    void * ptr = bf_malloc(size,lock_version, &head_nolock, &tail_nolock);
    return ptr;
}
void ts_free_nolock(void *ptr){
    my_free(ptr,&head_nolock,&tail_nolock);
}

