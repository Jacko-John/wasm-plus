#include "../include/sections.h"

void read_table_section(Module *m, const u8 *bytes, u32 *pos) {
    // 解析表段
    // 表段持有一个带有类型的引用数组，比如像函数这种无法作为原始字节存储在模块线性内存中的项目
    // 通过为 Wasm 框架提供一种能安全映射对象的方式，表段可以为 Wasm 提供一部分代码安全性
    // 当代码想要访问表段中引用的数据时，它要向 Wasm 框架请求变种特定索引处的条目，
    // 然后 Wasm 框架会读取存储在这个索引处的地址，并执行相关动作

    // 表段和表项编码格式如下：
    // table_sec: 0x04|byte_count|vec<table_type> # vec 目前长度只能是 1
    // table_type: 0x70|limits
    // limits: flags|min|(max)?

    // 读取表的数量
    u32 table_count = read_LEB128_unsigned(bytes, pos, 32);
    // 模块最多只能定义一张表，因此 table_count 必需为 1
    ASSERT(table_count == 1, "More than 1 table not supported\n")

    // 解析表段中的表 table_type（目前模块只会包含一张表）
    parse_table_type(m, pos);

    // 为存储表中的元素申请内存（在解析元素段时会用到--将元素段中的索引存储到刚申请的内存中）
    m->table.entries = acalloc(m->table.cur_size, sizeof(u32), "Module->table.entries");
}