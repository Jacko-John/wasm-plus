#include "../include/sections.h"

void read_type_section(Module *m, const u8 *bytes, u32 *pos) {
    // 解析类型段
    // 即解析模块中所有函数签名（也叫函数原型）
    // 函数原型示例：(a, b, ...) -> (x, y, ...)

    // 类型段编码格式如下：
    // type_sec: 0x01|byte_count|vec<func_type>

    // 读取类型段中所有函数签名的数量
    m->type_cnt = read_LEB128_unsigned(bytes, pos, 32);

    // 为存储类型段中的函数签名申请内存
    m->func_types = acalloc(m->type_cnt, sizeof(Type), "Module->func_types");

    // 遍历解析每个类型 func_type，其编码格式如下：
    // func_type: 0x60|param_count|(param_val)+|return_count|(return_val)+
    for (u32 i = 0; i < m->type_cnt; i++) {
        Type *type = &m->func_types[i];

        // 函数标记值 FtTag（即 0x60），暂时忽略
        u32 tag = read_LEB128_unsigned(bytes, pos, 7);
        ASSERT(tag == TYPE_FUNC, "Wrong function type tag 0x%x\n", tag)

        // 解析函数参数个数
        type->param_cnt = read_LEB128_unsigned(bytes, pos, 32);
        type->params = acalloc(type->param_cnt, sizeof(u32),
                               "type->params");
        // 解析函数每个参数的类型
        for (u32 p = 0; p < type->param_cnt; p++) {
            type->params[p] = read_LEB128_unsigned(bytes, pos, 32);
        }

        // 解析函数返回值个数
        type->result_cnt = read_LEB128_unsigned(bytes, pos, 32);
        type->results = acalloc(type->result_cnt, sizeof(u32),
                                "type->results");
        // 解析函数每个返回值的类型
        for (u32 r = 0; r < type->result_cnt; r++) {
            type->results[r] = read_LEB128_unsigned(bytes, pos, 32);
        }

        // 基于函数签名计算的唯一掩码值
        type->mask = get_type_mask(type);
    }
}