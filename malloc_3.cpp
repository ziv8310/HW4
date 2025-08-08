#include <algorithm>
#include <unistd.h>
#include <cstring>
#include <cstddef>

#define MAX_ORDER 10
#define BLOCKS_NUM 32
#define MAX_BLOCK_SIZE 131072 //1024*128

const size_t BLOCK_SIZES[] = { 128, 256, 512, 1024, 2048, 4096, 8192, 16384, 32768, 65536 ,131072}; //TODO must update values with metadata count


void *startAdrr = nullptr;
struct MallocMetadata {
    size_t size;
    size_t used_size;
    bool is_free;
    MallocMetadata *next;
    MallocMetadata *prev;
    MallocMetadata *parent;
    MallocMetadata *left;
    MallocMetadata *right;
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

    startAdrr = sbrk(MAX_BLOCK_SIZE * 32);
    if (pointer == (void *) -1) {
        return -1; //todo ending up sending nullptr is that wanted?
    }
    for (int i = 0; i < 32; ++i) {
        malloc_tree[i]->size = MAX_BLOCK_SIZE - _size_meta_data();
        malloc_tree[i]->used_size = 0;
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
        // malloc_tree[i]->address = static_cast<void*>(static_cast<std::byte*>(startAdrr) + MAX_BLOCK_SIZE*i);
    }
    malloc_lists[10] = malloc_tree[0];
    for(int i = 0; i < 10; i++) {
        malloc_lists[i] = nullptr;
    }
}

void update_parent(MallocMetadata* current, int current_order, bool add_only_metadata) {
    //TODO add check if current->parent == nullptr and current_order != 0
    // DEBUG: this check should trigger only for block that aren't 128Kb, if it didn't trigger for a block <128Kb this is a bug
    if (!add_only_metadata) {
        if (current->parent != nullptr) {
            current->parent->used_size += BLOCK_SIZES[current_order] - _size_meta_data(); //TODO assume that metadata gets added to parent when node was created.
            current->parent->is_free = false;
            update_parent(current->parent, current_order+1, add_only_metadata);
        }
    } else {
        if (current->parent != nullptr) {
            current->parent->used_size += _size_meta_data(); //TODO assume that metadata gets added to parent when node was created.
            update_parent(current->parent, current_order+1, add_only_metadata);
        }
    }
}

void remove_from_free_list(MallocMetadata* current) {
    if (current->prev != nullptr) {
        current->prev->next = current->next;
    }
    if (current->next != nullptr) {
        current->next->prev = current->prev;
    }
}

void occupy_block(MallocMetadata* current, int current_order) {
    remove_from_free_list(current);
    update_parent(current, current_order, false);
}

MallocMetadata* find_optimal_size(MallocMetadata* start, int current_order, size_t requested_size) {

    // returns nullptr if starting section doesnt have enough free space
    if((start->size - start->used_size) < requested_size) { //TODO may be redundant?
        return nullptr;
    }
    if(current_order == 0) {
        start->is_free = false;
        occupy_block(start, current_order); //removes from free list and updates parent
        return start;
    }
    // check if after splitting the smaller can still fit the request
    if(requested_size < (BLOCK_SIZES[current_order-1]/2) - _size_meta_data()) {
        //if a smaller block is enough, try to find a fit in either one of the child nodes
        MallocMetadata* search_left = start + 1;
        MallocMetadata* result = nullptr;
        // if left node doesn't exist
        if(start->left == nullptr) { //TODO maybe need add check for if child block exists but not free
            search_left->size = BLOCK_SIZES[current_order-1] - _size_meta_data();
            search_left->used_size = 0;
            search_left->is_free = true;
            search_left->parent = start;
            search_left->left = nullptr;
            search_left->right = nullptr;
            // search_left->address = start->address;
            add_to_free_list(search_left);
            result = find_optimal_size(search_left, current_order-1, requested_size);
        }
        // if left node does exist, search if it has space
        else if((start->left->size - start->left->used_size >= requested_size)) {
             result = find_optimal_size(start->left, current_order-1, requested_size);
        }
        // if we found a space for it in either one of the ways then return metadata pointer
        if (result != nullptr) {
            return result;
        }
        MallocMetadata* search_right = reinterpret_cast<MallocMetadata*>(reinterpret_cast<std::byte*>(start+1) + BLOCK_SIZES[current_order-1]);

        // if left node isn't an option try right
        // if right node doesn't exist
        if(start->right == nullptr) { //TODO maybe need add check for if child block exists but not free
            search_right->size = BLOCK_SIZES[current_order-1] - _size_meta_data();
            search_right->used_size = 0;
            search_right->is_free = true;
            search_right->parent = start;
            search_right->left = nullptr;
            search_right->right = nullptr;
            add_to_free_list(search_right);
            result = find_optimal_size(search_right, current_order-1, requested_size);
        }
        else if ((start->right->size - start->right->used_size >= requested_size)) {
            result = find_optimal_size(start->right, current_order-1, requested_size);
        }
        //at this point return no matter if result is actual address or nullptr
        return result;
    } else {
        //if lower block can't fit request, we found optimal size
        //if one the child nodes is allocated and uses space, cant allocate
        //otherwise return data
        if (start->is_free == true) {
            start->is_free = false;
            start->used_size = requested_size;
            occupy_block(start, current_order);
        } else {
            return nullptr;
        }
    }
}

void *smalloc(size_t size) {
    //focus only on sbrk memory
    if (size == 0 || size > 10 * 10 * 10 * 10 * 10 * 10 * 10 * 10) {
        return nullptr;
    }
    size_t new_full_size = size + _size_meta_data();
    void *pointer;
    MallocMetadata *new_meta = nullptr;
    MallocMetadata *curr = nullptr;
    MallocMetadata *optimal_fit_result = nullptr;

    for(int i = 0; i<=10; i++) {
        if(BLOCK_SIZES[i]-_size_meta_data() >= size) {
            curr = malloc_lists[i];
            while(curr != nullptr) {
                optimal_fit_result = find_optimal_size(curr, i, size);
                //DEBUG works under assumption that if optimal fit doesnt find a space it returns nullptr
                if (optimal_fit_result != nullptr) {
                    return optimal_fit_result;
                }
            }//while
            //loop throught entire list of the current size and look for optimal fit
            //while(next != nullptr){
            //if (free_size > requested) use find_optimal otherwise stop
            //find_optimal_size(malloc_lists[i], i);
            //}
        }
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
