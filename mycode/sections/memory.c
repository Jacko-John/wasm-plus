#include "../include/sections.h"

void read_memory_section(Module *m, const u8 *bytes, u32 *pos) {
    // 解析内存段
    // 内存段列出了模块内定义的内存，由于 Wasm 模块不能直接访问设备内存，
    // 实例化模块的环境传入一个 ArrayBuffer，Wasm 模块示例将其用作线性内存。
    // 模块的内存被定义为 Wasm 页，每页 64KB。当环境指定 Wasm 模块可以使用多少内存时，指定的是初始页数，
    // 可能还有一个最大页数。如果模块需要更多内存，可以请求内存增长指定页数。如果指定了最大页数，则框架会防止内存增长超过这一点
    // 如果没有指定最大页数，则内存可以无限增长

    // 内存段和内存类型编码格式如下：
    // mem_sec: 0x05|byte_count|vec<mem_type> # vec 目前长度只能是 1
    // mem_type: limits
    // limits: flags|min|(max)?

    // 读取内存的数量
    u32 memory_count = read_LEB128_unsigned(bytes, pos, 32);
    // 模块最多只能定义一块内存，因此 memory_count 必需为 1
    ASSERT(memory_count == 1, "More than 1 memory not supported\n")

    // 解析内存段中内存 mem_type（目前模块只会包含一块内存）
    parse_memory_type(m, pos);

    // 为存储内存中的数据申请内存（在解析数据段时会用到--将数据段中的数据存储到刚申请的内存中）
    m->memory.bytes = acalloc(m->memory.cur_size * PAGE_SIZE, sizeof(u32), "Module->memory.bytes");
}