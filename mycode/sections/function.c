#include "../include/sections.h"

void read_function_section(Module *m, const u8 *bytes, u32 *pos) {
    // 解析函数段
    // 函数段列出了内部函数的函数签名在所有函数签名中的索引，函数的局部变量和字节码则存在代码段中

    // 函数段编码格式如下：
    // func_sec: 0x03|byte_count|vec<type_idx>

    // 读取函数段所有函数的数量 (import + local)
    m->function_cnt += read_LEB128_unsigned(bytes, pos, 32);

    // 为存储函数段中的所有函数申请内存
    Block *functions;
    functions = acalloc(m->function_cnt, sizeof(Block), "Block(function)");

    // 由于解析了导入段在解析函数段之前，而导入段中可能有导入外部模块函数
    // 因此如果 m->import_func_cnt 不为 0，则说明已导入外部函数，并存储在了 m->funcs 中
    // 所以需要先将存储在了 m->funcs 中的导入函数对应数据拷贝到 functions 中
    // 简单来说，就是先将之前解析导入函数所得到的数据，拷贝到新申请的内存中（因为之前申请的内存已不足以存储所有函数的数据）
    if (m->import_func_cnt != 0) {
        memcpy(functions, m->funcs, sizeof(Block) * m->import_func_cnt);
        free(m->funcs);// 拷贝完之后释放原来的内存
    }
    m->funcs = functions;

    // 遍历每个函数项，读取其对应的函数签名在所有函数签名中的索引，并根据索引获取到函数签名
    for (u32 f = m->import_func_cnt; f < m->function_cnt; f++) {
        // f 为该函数在所有函数（包括导入函数）中的索引
        m->funcs[f].func_idx = f;
        // tidx 为该内部函数的函数签名在所有函数签名中的索引
        u32 tidx = read_LEB128_unsigned(bytes, pos, 32);
        // 通过索引 tidx 从所有函数签名中获取到具体的函数签名，然后设置为该函数的函数签名
        m->funcs[f].type = &m->func_types[tidx];
    }
}