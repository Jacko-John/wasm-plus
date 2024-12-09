#include "../include/sections.h"

void read_export_section(Module *m, const u8 *bytes, u32 *pos) {
    // 解析导出段
    // 导出段包含模块所有导出成员，主要包含四种：函数、表、内存、全局变量
    // 导出项主要包括两个：1.导出成员名  2.导出描述：1 个字节的类型（0-函数、1-表、2-内存、3-全局变量）+ 导出项在相应段中的索引

    // 导出段编码格式如下：
    // export_sec: 0x07|byte_count|vec<export>
    // export: name|export_desc
    // export_desc: tag|[func_idx, table_idx, mem_idx, global_idx]

    // 读取导出项数量
    u32 export_count = read_LEB128_unsigned(bytes, pos, 32);
    u32 cur_cnt = m->export_cnt;
    m->exports = arecalloc(m->exports, cur_cnt, export_count + cur_cnt, sizeof(Export), "exports");

    // 遍历所有导出项，解析对应数据
    for (u32 e = 0; e < export_count; e++) {
        // 读取导出成员名
        char *name = read_string(bytes, pos, NULL);

        // 读取导出类型
        u32 extern_type = bytes[(*pos)++];

        // 读取导出项在相应段中的索引
        u32 index = read_LEB128_unsigned(bytes, pos, 32);

        // 先保存当前导出项的索引
        u32 eidx = m->export_cnt;

        // 设置导出项的成员名
        m->exports[eidx].name = name;

        // 设置导出项的类型
        m->exports[eidx].type = extern_type;

        // 根据导出项的类型，设置导出项的值
        switch (extern_type) {
            case IMPORT_FUNC:
                // 获取函数并赋给导出项
                m->exports[eidx].value = &m->funcs[index];
                break;
            case IMPORT_TABLE:
                // 目前 Wasm 版本规定只能定义一张表，所以索引只能为 0
                ASSERT(index == 0, "Only 1 table in MVP\n")
                // 获取模块内定义的表并赋给导出项
                m->exports[eidx].value = &m->table;
                break;
            case IMPORT_MEM:
                // 目前 Wasm 版本规定只能定义一个内存，所以索引只能为 0
                ASSERT(index == 0, "Only 1 memory in MVP\n")
                // 获取模块内定义的内存并赋给导出项
                m->exports[eidx].value = &m->memory;
                break;
            case IMPORT_GLOBAL:
                // 获取全局变量并赋给导出项
                m->exports[eidx].value = &m->globals[index];
                break;
            default:
                break;
        }

        // 导出项数量加 1
        m->export_cnt += 1;
    }
}