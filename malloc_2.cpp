#include <unistd.h>
#include <cstring>

struct MallocMetadata {
    size_t size;
    bool is_free;
    MallocMetadata *next;
    MallocMetadata *prev;
};

size_t var_num_free_blocks = 0; //
size_t var_num_free_bytes = 0;
size_t var_num_allocated_blocks = 0; //number of total blocks
size_t var_num_allocated_bytes = 0;
size_t var_num_meta_data_bytes = 0;

size_t _size_meta_data() {
    return sizeof(MallocMetadata);
}

MallocMetadata *malloc_list = nullptr;

void *smalloc(size_t size) {
    if (size == 0 || size > 10 * 10 * 10 * 10 * 10 * 10 * 10 * 10) {
        return nullptr;
    }
    size_t new_full_size = size + _size_meta_data();
    MallocMetadata *curr = malloc_list;
    void *pointer;
    MallocMetadata *new_meta;


    // if list is empty
    if (curr == nullptr) {
        pointer = sbrk(new_full_size);
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