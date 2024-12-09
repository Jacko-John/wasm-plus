#include "../include/utils.h"

// 申请内存
void *acalloc(size_t num, size_t size, char *name) {
    void *res = calloc(num, size);
    if (res == NULL) {
        FATAL("Could not allocate %lu bytes for %s", num * size, name)
    }
    return res;
}

// 在原有内存基础上重新申请内存
void *arecalloc(void *ptr, size_t old_num, size_t num, size_t size, char *name) {
    // 重新分配内存
    void *res = realloc(ptr, num * size);
    if (res == NULL) {
        FATAL("Could not allocate %lu bytes for %s", num * size, name)
    }
    // 将新申请的内存中的前面部分--即为新数据准备的内存空间，用 0 进行初始化
    memset(res + old_num * size, 0, (num - old_num) * size);
    return res;
}