#include <stdio.h>
#include <stdlib.h>   // for size_t, NULL
#include <string.h>   // for memset, memmove
#include <unistd.h>   // for sbrk()
#include <limits.h>

int main(void) {
    printf("Hello, World!\n");
    return 0;
}

//part 1
//===================================================
void* smalloc(size_t size){
    if(size==0 || size >10*10*10*10*10*10*10*10){
        return NULL;
    }
    void* pointer = sbrk(size);
    if (ptr == (void*) -1) {
        return NULL;
    }
    return pointer;
}



//===================================================
//part 2
size_t _size_meta_data(){
    return sizeof(MallocMetadata);
}

//todo move somewhere
MallocMetadata * malloc_list = NULL;
malloc_list->next=NULL;

typedef struct MallocMetadata {
    size_t size;
    bool is_free;
    struct MallocMetadata* next;
    struct MallocMetadata* prev;
} MallocMetadata;

void* smalloc_acalloc(size_t num,, size_t size,int wanted) {
if (size == 0 || size > 10 * 10 * 10 * 10 * 10 * 10 * 10 * 10) {
return NULL;
}
size_t full_size = size + _size_meta_data();
MallocMetadata *copy_list = malloc_list; // todo should maybe copy fields
while (copy_list->next != NULL) {
if (copy_list->is_free && copy_list->size >= full_size) {
//alocating all block even if we lose space
//            new_block.size = full_size;
//            new_block->is_free = 0;
//            new_block->prev = copy_list;
//            new_block->next = copy_list->next;
copy_list->is_free=0;
if(wanted!=-1){
std::memset(ptr,
0, total);
}

return //todo return pointer adress -metadata;
}

void *pointer = sbrk(full_size);
if (ptr == (void *) -1) {
return
NULL;
}
return pointer + _size_meta_data();

void* smalloc(size_t size) {

}

void* scalloc(size_t num, size_t size) {
    return smalloc(size * num);
    //todo is enough? can all the blocks share one metadata?
}
void sfree(void* p) {
    if (p == NULL) {
        return;
    }
    MallocMetadata *meta = static_cast<MallocMetadata *>(p) - 1;
    meta->is_free = 1;
}

