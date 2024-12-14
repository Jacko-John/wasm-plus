#include "../include/utils.h"
#include <dlfcn.h>
char exception[4096];

// 查找动态库中的 symbol
// 如果解析成功则返回 true
// 如果解析失败则返回 false 并设置 err
boolean resolve_sym(char *filename, char *symbol, void **val, char **err) {
    void *handle = NULL;
    dlerror();

    if (filename) {
        handle = dlopen(filename, RTLD_LAZY);
        if (!handle) {
            *err = dlerror();
            return false;
        }
    }

    // 查找动态库中的 symbol
    // 根据 动态链接库 操作句柄 (handle) 与符号 (symbol)，返回符号对应的地址。使用这个函数不但可以获取函数地址，也可以获取变量地址。
    // handle：由 dlopen 打开动态链接库后返回的指针；
    // symbol：要求获取的函数或全局变量的名称。
    // 返回值：指向函数的地址，供调用使用。
    *val = dlsym(handle, symbol);

    if ((*err = dlerror()) != NULL) {
        return false;
    }
    return true;
}

// 基于函数签名计算唯一的掩码值
u64 get_type_mask(Type *type) {
    u64 mask = 0x80;
    u32 res_cnt = type->result_cnt;
    if (type->result_cnt + type->param_cnt > 15) {
        res_cnt = 15 - type->param_cnt;
    }
    for (u32 i = 0; i < res_cnt; i++) {
        mask |= 0x80 - type->results[i];
        mask <<= 4;
    }

    for (u32 i = 0; i < type->param_cnt; i++) {
        mask <<= 4;
        mask |= 0x80 - type->params[i];
    }

    return mask;
}

// 初始化 block_types 数组
u32 block_type_results[4][1] = {{TYPE_I32}, {TYPE_I64}, {TYPE_F32}, {TYPE_F64}};
Type block_types[5] = {
        {
                .param_cnt = 0,
                .params = NULL,
                .result_cnt = 0,
                .results = NULL,
                .mask = 0,
        },
        {
                .param_cnt = 0,
                .params = NULL,
                .result_cnt = 1,
                .results = block_type_results[0],
                .mask = 0,
        },
        {
                .param_cnt = 0,
                .params = NULL,
                .result_cnt = 1,
                .results = block_type_results[1],
                .mask = 0,
        },
        {
                .param_cnt = 0,
                .params = NULL,
                .result_cnt = 1,
                .results = block_type_results[2],
                .mask = 0,
        },
        {
                .param_cnt = 0,
                .params = NULL,
                .result_cnt = 1,
                .results = block_type_results[3],
                .mask = 0,
        },

};


Type *get_block_type(const Module *m, const u8 *bytes, u32 pos) {
    u8 value_type = bytes[pos];
    if ((value_type & 0x80) == 0) {
        // 在 block_types 数组中查找 value_type 对应的类型
        switch (value_type) {
            case TYPE_EMPTY:
                return &block_types[0];
            case TYPE_I32:
                return &block_types[1];
            case TYPE_I64:
                return &block_types[2];
            case TYPE_F32:
                return &block_types[3];
            case TYPE_F64:
                return &block_types[4];
            default:
                FATAL("Invalid block_type value_type: %d\n", value_type)
        }
    } else {
        // 获取Type Section中的类型
        u32 tidx = read_LEB128_unsigned(bytes, &pos, 32);
        return &m->func_types[tidx];
    }
}
