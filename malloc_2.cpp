#include <unistd.h>

size_t _size_meta_data();

struct MallocMetadata {
    size_t size;
    bool is_free;
    MallocMetadata* next;
    MallocMetadata* prev;
};
MallocMetadata* malloc_list = nullptr;
// malloc_list->next= nullptr;

void* smalloc(size_t size) {
    if (size == 0 || size > 10 * 10 * 10 * 10 * 10 * 10 * 10 * 10) {
        return nullptr;
    }
    size_t new_full_size = size + _size_meta_data();
    MallocMetadata* curr = malloc_list; // todo should maybe copy fields
    void *pointer;
    MallocMetadata *new_meta;


    //TODO need to check for pointer fails, maybe make it a seperate function.
    // if list is empty
    if (curr == nullptr) {
        pointer = std::sbrk(new_full_size); //instead of sbrk call wrapper function.
        new_meta = (MallocMetadata *)pointer;
        new_meta->size = size;
        new_meta->is_free = false;
        new_meta->next = nullptr;
        new_meta->prev = nullptr;
        // TODO return malloc aid func val
    }
    else {
        // if list isn't empty check if it has a free block of an appropriate size
        while (curr != nullptr) {
            if (curr->is_free && curr->size >= size) {
                curr->is_free = false;
                // //use entire block even if we lose space (high internal fragmentation)
                curr++;
                return curr;
            }
            curr = curr->next;
        }
        // if list doesn't have a freeblock of an appropriate size then allocate a new one here.
        // updates both new block and previous block

        // new block allocation
        pointer = std::sbrk(new_full_size);
        new_meta = (MallocMetadata *)pointer;
        new_meta->size = size;
        new_meta->is_free = false;
        new_meta->next = nullptr;
        new_meta->prev = curr;

        // old block fix
        curr->next = new_meta;
        return pointer;
    }
}

void* scalloc(size_t num, size_t size){
    return smalloc(size*num);
    //todo is enough? can all the blocks share one metadata?
}
void sfree(void* p){
    if(p==NULL){
        return;
    }
}

size_t _size_meta_data(){
    return sizeof(MallocMetadata);
}