#include "../include/sections.h"

inline double fmin(double x, double y) {
    return x < y ? x : y;
}

// 解析表段中的表 table_type（目前表段只会包含一张表）
// 表 table_type 编码如下：
// table_type: 0x70|limits
// limits: flags|min|(max)?
// 注：之所以要封装成独立函数，是因为在 load_module 函数中有两次调用：1.解析本地定义的表段；2. 解析从外部导入的表
u32 parse_table_type(Module *m, u32 *pos) {
    // 由于表段中只会有一张表，所以无需遍历

    // 表中的元素必需为函数引用，所以编码必需为 0x70
    u32 cnt = m->table_cnt;
    m->table_cnt += 1;
    m->tables = arecalloc(m->tables, cnt, m->table_cnt, sizeof(Table), "tables");
    Table *table = &m->tables[cnt];
    table->elem_type = read_LEB128_unsigned(m->bytes, pos, 7);
    ASSERT(table->elem_type == TYPE_REF_FUNC, "Table elem_type 0x%x unsupported\n", table->elem_type)

    // flags 为标记位，如果为 0 表示只需指定表中元素数量下限；为 1 表示既要指定表中元素数量的上限，又指定表中元素数量的下限
    u32 flags = read_LEB128_unsigned(m->bytes, pos, 32);
    // 先读取表中元素数量下限，同时设置为该表的当前元素数量
    u32 tsize = read_LEB128_unsigned(m->bytes, pos, 32);
    table->min_size = tsize;
    table->cur_size = tsize;
    // flags 为 1 表示既要指定表中元素数量的上限，又指定表中元素数量的下限
    if (flags & 0x1) {
        // 读取表中元素数量的上限
        tsize = read_LEB128_unsigned(m->bytes, pos, 32);
        // 表的元素数量最大上限为 64K，如果读取的表的元素数量上限值超过 64K，则默认设置 64K，否则设置为读取的值即可
        table->max_size = (u32) fmin(0x10000, tsize);
    } else {
        // flags 为 0，表示没有特别指定表的元素数量上限，所以设置为默认的 64K 即可
        table->max_size = 0x10000;
    }
    return cnt;
}

// 解析内存段中的内存 mem_type（目前内存段只会包含一块内存）
// 内存 mem_type 编码如下：
// mem_type: limits
// limits: flags|min|(max)?
// 注：之所以要封装成独立函数，是因为在 load_module 函数中有两次调用：1.解析本地定义的内存段；2. 解析从外部导入的内存
void parse_memory_type(Module *m, u32 *pos) {
    // 由于内存段中只会有一块内存，所以无需遍历

    // flags 为标记位，如果为 0 表示只指定内存大小的下限；为 1 表示既指定内存大小的上限，又指定内存大小的下限
    u32 flags = read_LEB128_unsigned(m->bytes, pos, 32);
    // 先读取内存大小的下限，并设置为该内存的初始大小
    u32 pages = read_LEB128_unsigned(m->bytes, pos, 32);
    m->memory.min_size = pages;
    m->memory.cur_size = pages;

    // flags 为 1 表示既指定内存大小上限，又指定内存大小下限
    if (flags & 0x1) {
        // 读取内存大小上限
        pages = read_LEB128_unsigned(m->bytes, pos, 32);
        // 内存大小最大上限为 2GB，如果读取的内存大小上限值超过 2GB，则默认设置 2GB，否则设置为读取的值即可
        m->memory.max_size = (u32) fmin(0x8000, pages);
    } else {
        // flags 为 0，表示没有特别指定内存大小上限，所以设置为默认的 2GB 即可
        m->memory.max_size = 0x8000;
    }
}