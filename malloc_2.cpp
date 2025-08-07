
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
//todo move somewhere


size_t  var_num_free_blocks=0;

size_t var_num_free_bytes=0;

size_t var_num_allocated_blocks=0; //number of total blocks

size_t var_num_allocated_bytes=0;

size_t var_num_meta_data_bytes=0;

size_t _size_meta_data(){
    return sizeof(MallocMetadata);
}
struct MallocMetadata {
    size_t size;
    bool is_free;
    MallocMetadata* next;
    MallocMetadata* prev;
} ;

void* smalloc(size_t size) {
    if (size == 0 || size > 10 * 10 * 10 * 10 * 10 * 10 * 10 * 10) {
        return NULL;
    }
    size_t full_size = size + _size_meta_data();
    MallocMetadata *copy_list = malloc_list; // todo should maybe copy fields
    while (copy_list->next != NULL) {
        if (copy_list->is_free && copy_list->size >= full_size) {
            //alocating all block even if we lose space

            copy_list->is_free=0;
            return //todo return pointer adress -metadata;
        }
    }
    void *pointer = sbrk(full_size);
    if (ptr == (void *) -1) {
        return NULL;
    }
    return pointer + _size_meta_data();
}

void* scalloc(size_t num, size_t size){

    void* p=smalloc(size*num);
    if(p==NULL)
    {
        return NULL;
    }
    std::memset(p, 0, size*num);
    return p;
}

void sfree(void* p){
    if(p==NULL){
        return;
    }
    MallocMetadata* meta = static_cast<MallocMetadata*>(p) - 1;
    meta->is_free=1;

}
void* srealloc(void* oldp, size_t size){
    if (size == 0 || size > 10 * 10 * 10 * 10 * 10 * 10 * 10 * 10) {
        return NULL;
    }
    if(oldp==NULL){
        return smalloc(size);
    }
    MallocMetadata* old_meta = static_cast<MallocMetadata*>(oldp) - 1;
    if(old_meta>=size){
        return oldp;
    }
    void*p= std::memmove(newp, oldp, size);
    return p;
}

size_t _num_free_blocks(){
    return var_num_free_blocks;
}

size_t _num_free_bytes(){
    return var_num_free_bytes;
}

size_t _num_allocated_blocks(){
    return var_num_allocated_blocks;
}

size_t _num_allocated_bytes(){
    return var_num_allocated_bytes;
}

size_t _num_meta_data_bytes(){
    return var_num_meta_data_bytes;
}
