#include <unistd.h>

void* smalloc(size_t size){
     if(size==0 || size >10*10*10*10*10*10*10*10){
         return nullptr;
     }
     void* pointer = std::sbrk(size);
     if (pointer == (void*) -1) {
         return nullptr;
     }
     return pointer;
 }
