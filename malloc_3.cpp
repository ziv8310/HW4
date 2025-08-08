#include <unistd.h>
#include <cstring>
#include <cstddef>

#define MAX_ORDER 10
#define BLOCKS_NUM 32
#define BLOCKS_SIZE 131072 //1024*128
void *startAdrr = nullptr;
struct MallocMetadata {
    size_t size;
    bool is_free;
    MallocMetadata *next;
    MallocMetadata *prev;
    MallocMetadata *parent;
    MallocMetadata *left;
    MallocMetadata *right;
    void *address;
};



size_t var_num_free_blocks = 0;
size_t var_num_free_bytes = 0;
size_t var_num_allocated_blocks = 0; //number of total blocks
size_t var_num_allocated_bytes = 0;
size_t var_num_meta_data_bytes = 0;

size_t _size_meta_data() {
    return sizeof(MallocMetadata);
}


MallocMetadata *malloc_lists[MAX_ORDER+1];
MallocMetadata *malloc_tree[BLOCKS_NUM];


int init_tree() {
    void *pointer;
//    MallocMetadata *new_meta;
//    new_meta = (MallocMetadata *) pointer;
//    new_meta = (MallocMetadata *) pointer;
//    new_meta->size = size;
//    new_meta->is_free = false;
//    new_meta->next = nullptr;
//    new_meta->prev = nullptr;
//    malloc_lists = new_meta;
//    pointer = sbrk(0);
//    if (pointer == (void *) -1) {
//        return -1; //todo ending up sending nullptr is that wanted?
//    }
//    int* adress = static_cast<int*>(pointer);
//    int align =*adress%128;
    startAdrr = sbrk(BLOCKS_SIZE * 32);
    if (pointer == (void *) -1) {
        return -1; //todo ending up sending nullptr is that wanted?
    }
    for (int i = 0; i < 32; ++i) {
        malloc_tree[i]->size = BLOCKS_SIZE - _size_meta_data();
        malloc_tree[i]->is_free = true;
        if (i == 0) {
            malloc_tree[i]->prev = nullptr;
        } else {
            malloc_tree[i]->prev = malloc_tree[i-1];
        }
        if (i == 31) {
            malloc_tree[i]->next = nullptr;
        } else {
            malloc_tree[i]->next = malloc_tree[i+1];
        }

        malloc_tree[i]->parent = nullptr;
        malloc_tree[i]->left = nullptr;
        malloc_tree[i]->right = nullptr;
        malloc_tree[i]->address = static_cast<void*>(static_cast<std::byte*>(startAdrr) + BLOCKS_SIZE*i);
    }
    malloc_lists[10] = malloc_tree[0];
}

int init_list(){



}
void *smalloc(size_t size) {
    //focus only on
    if (size == 0 || size > 10 * 10 * 10 * 10 * 10 * 10 * 10 * 10) {
        return nullptr;
    }
    size_t new_full_size = size + _size_meta_data();
    MallocMetadata *curr = malloc_list;
    void *pointer;
    MallocMetadata *new_meta;


    //TODO need to check for pointer fails, maybe make it a seperate function.
    // if list is empty
    if (curr == nullptr) {
        if (init_tree(size) == -1) {
            return nullptr;
        }
        if (init_list(size) == -1) {

        }
        pointer = sbrk(new_full_size); //TODO instead of sbrk, call wrapper function.
        if (pointer == (void *) -1) {
            return nullptr;
        }
        new_meta = (MallocMetadata *) pointer;
        new_meta->size = size;
        new_meta->is_free = false;
        new_meta->next = nullptr;
        new_meta->prev = nullptr;
        malloc_list = new_meta;
        var_num_allocated_blocks++;
        var_num_allocated_bytes += size;
        var_num_meta_data_bytes += _size_meta_data();
        new_meta++;
        return new_meta;
    } else {
        // if list isn't empty check if it has a free block of an appropriate size
        while (curr->next != nullptr) {
            if (curr->is_free && curr->size >= size) {
                curr->is_free = false;
                // //use entire block even if we lose space (high internal fragmentation)
                curr++;

                var_num_free_blocks--;
                var_num_free_bytes -= size;
                return curr;
            }
            curr = curr->next;
        }
        if (curr->is_free && curr->size >= size) {
            curr->is_free = false;
            // //use entire block even if we lose space (high internal fragmentation)
            curr++;

            var_num_free_blocks--;
            var_num_free_bytes -= size;
            return curr;
        }


        // if list doesn't have a freeblock of an appropriate size then allocate a new one here.
        // updates both new block and previous block

        // new block allocation
        pointer = sbrk(new_full_size);
        if (pointer == (void *) -1) {
            return nullptr;
        }

        new_meta = (MallocMetadata *) pointer;
        new_meta->size = size;
        new_meta->is_free = false;
        new_meta->next = nullptr;
        new_meta->prev = curr;
        var_num_allocated_blocks++;
        var_num_allocated_bytes += size;
        var_num_meta_data_bytes += _size_meta_data();

        // old block fix
        curr->next = new_meta;
        new_meta++;
        return new_meta;
    }
    return nullptr;
}

void *scalloc(size_t num, size_t size) {
    void *p = smalloc(size * num);
    if (p == nullptr) {
        return nullptr;
    }
    memset(p, 0, size * num);
    return p;
}

void sfree(void *p) {
    if (p == nullptr) {
        return;
    }
    MallocMetadata *meta = static_cast<MallocMetadata *>(p) - 1;
    meta->is_free = true;
    var_num_free_blocks++;
    var_num_free_bytes += meta->size;
}


void *srealloc(void *oldp, size_t size) {
    if (size == 0 || size > 10 * 10 * 10 * 10 * 10 * 10 * 10 * 10) {
        return nullptr;
    }
    if (oldp == nullptr) {
        return smalloc(size);
    }
    MallocMetadata *old_meta = static_cast<MallocMetadata *>(oldp) - 1;
    if (old_meta->size >= size) {
        return oldp;
    }


    void *new_mem = smalloc(size);
    if (new_mem != nullptr) {
        sfree(oldp);
        memmove(new_mem, oldp, size);
        return new_mem;
    } else {
        return nullptr;
    }
}

size_t _num_free_blocks() {
    return var_num_free_blocks;
}

size_t _num_free_bytes() {
    return var_num_free_bytes;
}

size_t _num_allocated_blocks() {
    return var_num_allocated_blocks;
}

size_t _num_allocated_bytes() {
    return var_num_allocated_bytes;
}

size_t _num_meta_data_bytes() {
    return var_num_meta_data_bytes;
}
