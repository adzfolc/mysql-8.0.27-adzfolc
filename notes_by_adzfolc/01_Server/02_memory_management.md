# 02_memory_management

1. MySQL 对内存管理通过 mysys/my_alloc.c 中的函数分配内存,主要入口是以下几个函数
    ```C++
    1. void *my_malloc(PSI_memory_key key, size_t size, int flags)
    2. 
    ```

2. `void *my_malloc(PSI_memory_key key, size_t size, int flags)` 精读
