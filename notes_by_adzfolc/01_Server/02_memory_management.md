# 02_memory_management

1. MySQL 对内存管理通过 [MySQL Memory Malloc & Calloc](../../mysys/my_malloc.cc) 中的函数分配内存,主要入口是以下几个函数,分析见源文件.
    ```C++
    1. void *my_malloc(PSI_memory_key key, size_t size, int flags)
    2. void *my_memdup(PSI_memory_key key, const void *from, size_t length,
                myf my_flags)
    3. char *my_strdup(PSI_memory_key key, const char *from, myf my_flags)
    4. char *my_strndup(PSI_memory_key key, const char *from, size_t length,
                 myf my_flags)
    ```

2. 在某些复杂的情况下,如果需要分配很多空白空间,那么必须使用 [MEM_ROOT](../../include/my_alloc.h).`MEM_ROOT`是 ARENA 内存池,熟悉的朋友会想到 LevelDB 的内存分配策略.
    ```C++
    1. void *Alloc(size_t length) MY_ATTRIBUTE((malloc))
    2. template <class T, class... Args>
        T *ArrayAlloc(size_t num, Args... args)
    ```
    (待精读)