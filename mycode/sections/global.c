#include "../include/interpreter.h"
#include "../include/sections.h"

void read_global_section(Module *m, const u8 *bytes, u32 *pos) {
    // 解析全局段
    // 全局段列出了模块内定义的所有全局变量
    // 每一项包括全局变量的类型（值类型和可变性）以及初始值

    // 全局段和全局项的编码格式如下：
    // global_sec: 0x60|byte_count|vec<global>
    // global: global_type|init_expr
    // global_type: val_type|mut
    // init_expr: (byte)+|0x0B

    // 读取模块中全局变量的数量
    u32 global_count = read_LEB128_unsigned(bytes, pos, 32);
    u32 cur_cnt = m->global_cnt;
    m->globals = arecalloc(m->globals, cur_cnt, global_count + cur_cnt, sizeof(Value), "globals");

    // 遍历全局段中的每一个全局变量项
    for (u32 g = 0; g < global_count; g++) {
        // 先读取全局变量的值类型
        u8 type = read_LEB128_unsigned(bytes, pos, 7);

        // 再读取全局变量的可变性
        u8 mutability = read_LEB128_unsigned(bytes, pos, 1);

        // 先保存当前全局变量的索引
        u32 gidx = m->global_cnt;

        // 计算初始化表达式 init_expr，并将计算结果设置为当前全局变量的初始值
        run_init_expr(m, type, pos);

        // 计算初始化表达式 init_expr 也就是栈式虚拟机执行表达式的字节码中的指令流过程，最终操作数栈顶保存的就是表达式的返回值，即计算结果
        // 将栈顶的值弹出并赋值给当前全局变量即可
        m->globals[gidx] = m->operand_stack[m->sp--];

        // 全局变量数量加 1
        m->global_cnt += 1;
    }
}