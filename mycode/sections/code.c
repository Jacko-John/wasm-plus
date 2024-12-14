#include "../include/sections.h"

void read_code_section(Module *m, const u8 *bytes, u32 *pos) {
    // 解析代码段
    // 代码段用于存放函数的字节码和局部变量，是 Wasm 二进制模块的核心，其他段存放的都是辅助信息
    // 为了节约空间，局部变量的信息是被压缩的：即连续多个相同类型的局部变量会被统一记录变量数量和类型

    // 代码段编码格式如下：
    // code_sec: 0xoA|byte_count|vec<code>
    // code: byte_count|func
    // func: vec<locals>|expr
    // locals: local_count|val_type

    // 读取代码段中的代码项的数量
    // 注：当前只解析函数
    u32 code_count = read_LEB128_unsigned(bytes, pos, 32);

    // 声明局部变量的值类型
    u8 val_type;

    // 遍历代码段中的每个代码项，解析对应数据
    for (u32 c = 0; c < code_count; c++) {
        // 获取代码项
        Block *function = &m->funcs[m->import_func_cnt + c];

        // 读取代码项所占字节数（暂用 4 个字节）
        u32 code_size = read_LEB128_unsigned(bytes, pos, 32);

        // 保存当前位置为代码项的起始位置（除去前面的表示代码项目长度的 4 字节）
        u32 payload_start = *pos;

        // 读取 locals 数量（注：相同类型的局部变量算一个 locals）
        u32 local_cnt = read_LEB128_unsigned(bytes, pos, 32);

        u32 save_pos, lidx, lecount;

        // 接下来需要对局部变量的相关字节进行两次遍历，所以先保存当前位置，方便第二次遍历前恢复位置
        save_pos = *pos;

        // 将代码项的局部变量数量初始化为 0
        function->local_count = 0;

        // 第一次遍历所有的 locals，目的是统计代码项的局部变量数量，将所有 locals 所包含的变量数量相加即可
        // 注：相同类型的局部变量算一个 locals
        for (u32 l = 0; l < local_cnt; l++) {
            // 读取单个 locals 所包含的变量数量
            lecount = read_LEB128_unsigned(bytes, pos, 32);

            // 累加 locals 所对应的局部变量的数量
            function->local_count += lecount;

            // 局部变量的数量后面接的是局部变量的类型，暂时不用
            read_LEB128_unsigned(bytes, pos, 7);
        }

        // 为保存函数局部变量的值类型的 function->locals 数组申请内存
        function->locals = acalloc(function->local_count, sizeof(u32), "function->locals");

        // 恢复之前的位置，重新遍历所有的 locals
        *pos = save_pos;

        // 将局部变量的索引初始化为 0
        lidx = 0;

        // 第二次遍历所有的 locals，目的是所有的代码项中所有的局部变量设置值类型
        for (u32 l = 0; l < local_cnt; l++) {
            // 读取单个 locals 所包含的变量数量
            lecount = read_LEB128_unsigned(bytes, pos, 32);

            // 读取单个 locals 的值类型
            val_type = read_LEB128_unsigned(bytes, pos, 7);

            // 为该 locals 所对应的每一个变量设置值类型（注：相同类型的局部变量算一个 locals）
            for (u32 n = 0; n < lecount; n++) {
                function->locals[lidx++] = val_type;
            }
        }

        // 在代码项中，紧跟在局部变量后面的就是代码项的字节码部分

        // 先读取单个代码项的字节码部分【起始地址】（即局部变量部分的后一个字节）
        function->start_addr = pos;

        // 然后读取单个代码项的字节码部分【结束地址】，同时作为字节码部分【跳转地址】
        function->end_addr = payload_start + code_size - 1;
        function->br_addr = function->end_addr;

        // 代码项的字节码部分必须以 0x0b 结尾
        ASSERT(bytes[function->end_addr] == 0x0b, "Code section did not end with 0x0b\n")

        // 更新当前的地址为当前代码项的【结束地址】（即代码项的字节码部分【结束地址】）加 1，以便遍历下一个代码项
        *pos = function->end_addr + 1;
    }
}