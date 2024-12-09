#include "../include/module.h"
#include "../include/instructions.h"
#include "../include/sections.h"
#include "../include/utils.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// 在单条指令中，除了占一个字节的操作码之外，后面可能也会紧跟着立即数，如果有立即数，则直接跳过立即数
// 注：指令是否存在立即数，是由操作数的类型决定，这也是 Wasm 标准规范的内容之一
void skip_immediate(const u8 *bytes, u32 *pos) {
    // 读取操作码
    u32 opcode = bytes[*pos];
    u32 count;
    *pos = *pos + 1;
    // 根据操作码类型，判断其有占多少位的立即数（或者没有立即数），并直接跳过该立即数
    switch (opcode) {
        /*
         * 控制指令
         * */
        case Block_:
        case Loop:
        case If:
            // Block_/Loop/If 指令的立即数有两部分，第一部分表示控制块的返回值类型（占 1 个字节），
            // 第二部分为子表达式（Block_/Loop 有一个子表达式，If 有两个子表达式）
            // 注：子表达式无需跳过，因为 find_block 主要就是要从控制块的表达式（包括子表达式）收集控制块的相关信息
            read_LEB128_unsigned(bytes, pos, 7);
            break;
        case Br:
        case BrIf:
            // 跳转指令的立即数表示跳转的目标标签索引（占 4 个字节）
            read_LEB_unsigned(bytes, pos, 32);
            break;
        case BrTable:
            // BrTable 指令的立即数是指定的 n+1 个跳转的目标标签索引（每个索引值占 4 个字节）
            // 其中前 n 个目标标签索引构成一个索引表，最后 1 个标签索引为默认索引
            // 最终跳转到哪一个目标标签索引，需要在运行期间才能决定
            count = read_LEB_unsigned(bytes, pos, 32);
            for (u32 i = 0; i < count; i++) {
                read_LEB_unsigned(bytes, pos, 32);
            }
            read_LEB_unsigned(bytes, pos, 32);
            break;
        case Call:
            // Call 指令的立即数表示被调用函数的索引（占 4 个字节）
            read_LEB_unsigned(bytes, pos, 32);
            break;
        case CallIndirect:
            // CallIndirect 指令有两个立即数，第一个立即数表示被调用函数的类型索引（占 4 个字节），
            // 第二个立即数为保留立即数（占 1 个比特位），暂无用途
            read_LEB_unsigned(bytes, pos, 32);
            read_LEB_unsigned(bytes, pos, 1);
            break;

        /*
         * 变量指令
         * */
        case LocalGet:
        case LocalSet:
        case LocalTee:
        case GlobalGet:
        case GlobalSet:
            // 变量指令的立即数用于表示全局/局部变量的索引（占 4 个字节）
            read_LEB_unsigned(bytes, pos, 32);
            break;

        /*
         * 内存指令
         * */
        case I32Load ... I64Store32:
            // 内存加载/存储指令有两个立即数，第一个立即数表示内存偏移量（占 4 个字节），
            // 第二个立即数表示对齐提示（占 4 个字节）
            read_LEB_unsigned(bytes, pos, 32);
            read_LEB_unsigned(bytes, pos, 32);
            break;
        case MemorySize:
        case MemoryGrow:
            // 内存大小/增加指令的立即数表示所操作的内存索引（占 1 个比特位）
            // 由于当前 Wasm 规范规定一个模块最多只能导入或定义一块内存，所以目前必须为 0
            read_LEB_unsigned(bytes, pos, 1);
            break;
        case I32Const:
            // I32Const 指令的立即数表示 32 有符号整数（占 4 个字节）
            read_LEB_unsigned(bytes, pos, 32);
            break;
        case I64Const:
            // F32Const 指令的立即数表示 64 有符号整数（占 8 个字节）
            read_LEB_unsigned(bytes, pos, 64);
            break;
        case F32Const:
            // F32Const 指令的立即数表示 32 位浮点数（占 4 个字节）
            // 注：LEB128 编码仅针对整数，而该指令的立即数为浮点数，并没有被编码，而是直接写入到 Wasm 二进制文件中的
            *pos += 4;
            break;
        case F64Const:
            // F64Const 指令的立即数表示 64 位浮点数（占 8 个字节）
            // 注：LEB128 编码仅针对整数，而该指令的立即数为浮点数，并没有被编码，而是直接写入到 Wasm 二进制文件中的
            *pos += 8;
            break;
        case TruncSat:
            // TruncSat 指令的操作码由两个字节表示，第二个字节的数值用来表示不同类型的浮点数和整数之间的转换
            read_LEB128_unsigned(bytes, pos, 8);
            break;
        default:
            // 其他操作码没有立即数
            // 注：Wasm 指令大部分指令没有立即数
            break;
    }
}

// 收集所有本地模块定义的函数中 Block_/Loop/If 控制块的相关信息，例如起始地址、结束地址、跳转地址、控制块类型等，
// 便于后续虚拟机解释执行指令时可以借助这些信息
void find_blocks(Module *m) {
    Block *function;
    Block *block;
    // 声明用于在遍历过程中存储控制块 block 的相关信息的栈
    Block *blockstack[BLOCK_STACK_SIZE];
    int top = -1;
    u8 opcode = Unreachable;

    // 遍历 m->functions 中所有的本地模块定义的函数，从每个函数字节码部分中收集 Block_/Loop/If 控制块的相关信息
    // 注：跳过从外部模块导入的函数，原因是导入函数的执行只需要执行 func_ptr 指针所指向的真实函数即可，无需通过虚拟机执行指令的方式
    for (u32 f = m->import_func_cnt; f < m->function_cnt; f++) {
        // 获取单个函数对应的结构体
        function = &m->funcs[f];

        // 从该函数的字节码部分的【起始地址】开始收集 Block_/Loop/If 控制块的相关信息--遍历字节码中的每条指令
        u32 pos = function->start_addr;
        // 直到该函数的字节码部分的【结束地址】结束
        while (pos <= function->end_addr) {
            // 每次 while 循环都会分析一条指令，而每条指令都是以占单个字节的操作码开始

            // 获取操作码，根据操作码类型执行不同逻辑
            opcode = m->bytes[pos];
            switch (opcode) {
                case Block_:
                case Loop:
                case If:
                    // 如果操作码为 Block_/Loop/If 之一，则声明一个 Block 结构体
                    block = acalloc(1, sizeof(Block), "Block");

                    // 设置控制块的块类型：Block_/Loop/If
                    block->block_type = opcode;

                    // 由于 Block_/Loop/If 操作码的立即数用于表示该控制块的类型（占一个字节）
                    // 所以可以根据该立即数，来获取控制块的类型，即控制块的返回值的数量和类型

                    // get_block_type 根据表示该控制块的类型的值（占一个字节），返回控制块的签名，即控制块的返回值的数量和类型
                    // 0x7f 表示有一个 i32 类型返回值、0x7e 表示有一个 i64 类型返回值、0x7d 表示有一个 f32 类型返回值、0x7c 表示有一个 f64 类型返回值、0x40 表示没有返回值
                    // 注：目前多返回值提案还没有进入 Wasm 标准，根据当前版本的 Wasm 标准，控制块不能有参数，且最多只能有一个返回值
                    block->type = get_block_type(m->bytes[pos + 1]);
                    // 设置控制块的起始地址
                    block->start_addr = pos;

                    // 向控制块栈中添加该控制块对应结构体
                    blockstack[++top] = block;
                    // 向 m->block_lookup 映射中添加该控制块对应结构体，其中 key 为对应操作码 Block_/Loop/If 的地址
                    m->block_lookup[pos] = block;
                    break;
                case Else_:
                    // 如果当前控制块中存在操作码为 Else_ 的指令，则当前控制块的块类型必须为 If
                    ASSERT(blockstack[top]->block_type == If, "Else not matched with if\n")

                    // 将 Else_ 指令的下一条指令地址，设置为该控制块的 else_addr，即 else 分支对应的字节码的首地址，
                    // 便于后续虚拟机在执行指令时，根据条件跳转到 else 分支对应的字节码继续执行指令
                    blockstack[top]->else_addr = pos + 1;
                    break;
                case End_:
                    // 如果操作码 End_ 的地址就是函数的字节码部分的【结束地址】，说明该控制块为该函数的最后一个控制块，则直接退出
                    if (pos == function->end_addr) {
                        break;
                    }

                    // 如果执行了 End_ 指令，说明至少收集了一个控制块的相关信息，所以 top 不可能是初始值 -1，至少大于等于 0
                    ASSERT(top >= 0, "Blockstack underflow\n")

                    // 从控制块栈栈弹出该控制块
                    block = blockstack[top--];

                    // 将操作码 End_ 的地址设置为控制块的结束地址
                    block->end_addr = pos;
                    // 设置控制块的跳转地址 br_addr
                    if (block->block_type == Loop) {
                        // 如果是 Loop 类型的控制块，需要循环执行，所以跳转地址就是该控制块开头指令（即 Loop 指令）的下一条指令地址
                        // 注：Loop 指令占用两个字节（1 字节操作码 + 1 字节操作数），所以需要加 2
                        block->br_addr = block->start_addr + 2;
                    } else {
                        // 如果是非 Loop 类型的控制块，则跳转地址就是该控制块的结尾地址，也就是操作码 End_ 的地址
                        block->br_addr = pos;
                    }
                    break;
                default:
                    break;
            }
            // 在单条指令中，除了占一个字节的操作码之外，后面可能也会紧跟着立即数，如果有立即数，则直接跳过立即数去处理下一条指令的操作码
            // 注：指令是否存在立即数，是由操作数的类型决定，这也是 Wasm 标准规范的内容之一
            skip_immediate(m->bytes, &pos);
        }
        // 当执行完 End_ 分支后，top 应该重新回到 -1，否则就是没有执行 End_ 分支
        ASSERT(top == -1, "Function ended in middle of block\n")
        // 控制块应该以操作码 End_ 结束
        ASSERT(opcode == End_, "Function block did not end with 0xb\n")
    }
}


// 解析 Wasm 二进制文件内容，将其转化成内存格式 Module，以便后续虚拟机基于此执行对应指令
Module *load_module(const u8 *bytes, const u32 byte_cnt) {
    // 用于标记解析 Wasm 二进制文件第 pos 个字节
    u32 pos = 0;

    // 声明内存格式对应的结构体 m
    Module *m;

    // 为 Wasm 内存格式对应的结构体 m 申请内存
    m = acalloc(1, sizeof(Module), "Module");

    // 重置运行时相关状态，主要是清空操作数栈、调用栈等
    m->sp = -1;
    m->fp = -1;
    m->call_stack_ptr = -1;

    m->bytes = bytes;
    m->byte_cnt = byte_cnt;
    m->block_lookup = acalloc(m->byte_cnt, sizeof(Block *), "function->block_lookup");

    m->type_cnt = 0;
    m->import_func_cnt = 0;
    m->function_cnt = 0;
    m->export_cnt = 0;
    m->global_cnt = 0;

    // 起始函数索引初始值设置为 -1
    m->start_func = -1;

    // 首先读取魔数 (magic number)，检查是否正确
    // 注：和其他很多二进制文件（例如 Java 类文件）一样，Wasm 也同样使用魔数来标记其二进制文件类型
    // 所谓魔数，你可以简单地将它理解为具有特定含义的一串数字
    // 一个标准 Wasm 二进制模块文件的头部数据是由具有特殊含义的字节组成的
    // 其中开头的前四个字节为 '（高地址）0x6d 0x73 0x61 0x00（低地址）'，这四个字节对应的 ASCII 字符为 'asm'
    u32 magic = ((u32 *) (bytes + pos))[0];
    pos += 4;
    ASSERT(magic == WASM_MAGIC_CODE, "Wrong module magic 0x%x\n", magic)

    // 然后读取当前 Wasm 二进制文件所使用的 Wasm 标准版本号，检查是否正确
    // 注：紧跟在魔数后面的 4 个字节是用来表示当前 Wasm 二进制文件所使用的 Wasm 标准版本号
    // 目前所有 Wasm 模块该四个字节的值为 '（高地址）0x00 0x00 0x00 0x01（低地址）'，即表示使用的 Wasm 标准版本为 1
    u32 version = ((u32 *) (bytes + pos))[0];
    pos += 4;
    ASSERT(version == WASM_VERSION, "Wrong module version 0x%x\n", version)

    // 最后根据段 ID 分别解析后面的各个段的内容
    // 和其他二进制格式（例如 Java 类文件）一样，Wasm 二进制格式也是以魔数和版本号开头，
    // 之后就是模块的主体内容，这些内容被分别放在不同的段（Section）中。
    // 一共定义了 12 种段，每种段分配了 ID（从 0 到 11）。除了自定义段之外，其他所有段都最多只能出现一次，且须按照 ID 递增的顺序出现。
    // ID 从 0 到 11 依次有如下 12 个段：
    // 自定义段、类型段、导入段、函数段、表段、内存段、全局段、导出段、起始段、元素段、代码段、数据段
    while (pos < byte_cnt) {
        // 每个段的第 1 个字节为该段的 ID，用于标记该段的类型
        u32 id = read_LEB128_unsigned(bytes, &pos, 7);

        // 紧跟在段 ID 后面的 4 个字节用于记录该段所占字节总长度
        u32 slen = read_LEB128_unsigned(bytes, &pos, 32);

        // 每次解析某个段的数据时，先将当前解析到的位置保存起来，以便后续使用
        u32 start_pos = pos;

        switch (id) {
            case CustomID: {
                // 解析自定义段
                // TODO: 暂不处理自定义段内容，直接跳过 （自定义段是实现者自定义的，对wasm本身的语义没有影响）
                pos += slen;
                break;
            }
            case TypeID: {
                // 解析类型段 (所有用到的函数类型都应该先在此定义，包括import)
                read_type_section(m, bytes, &pos);
                ASSERT(pos - start_pos != slen, "Type section length mismatch\n");
                break;
            }
            case ImportID: {
                read_import_section(m, bytes, &pos);
                ASSERT(pos - start_pos != slen, "Import section length mismatch\n")
                break;
            }
            case FuncID: {
                // 解析函数段
                read_function_section(m, bytes, &pos);
                ASSERT(pos - start_pos != slen, "Function section length mismatch\n")
                break;
            }
            case TableID: {
                read_table_section(m, bytes, &pos);
                ASSERT(pos - start_pos != slen, "Table section length mismatch\n")
                break;
            }
            case MemID: {
                read_memory_section(m, bytes, &pos);
                ASSERT(pos - start_pos != slen, "Memory section length mismatch\n")
                break;
            }
            case GlobalID: {
                read_global_section(m, bytes, &pos);
                pos = start_pos + slen;
                break;
            }
            case ExportID: {
                read_export_section(m, bytes, &pos);
                ASSERT(pos - start_pos != slen, "Export section length mismatch\n")
                break;
            }
            case StartID: {
                // 解析起始段
                // 起始段记录了起始函数在本地模块所有函数中索引，而起始函数是在【模块完成初始化后】，【被导出函数可调用之前】自动被调用的函数
                // 可以将起始函数视为一种初始化全局变量或内存的函数，且函数必须处于被模块内部，不能是从外部导入的
                // 起始函数的作用有两个：
                // 1. 在模块加载后进行初始化工作
                // 2. 将模块变成可执行文件

                // 起始段的编码格式如下：
                // start_sec: 0x08|byte_count|func_idx
                m->start_func = read_LEB128_unsigned(bytes, &pos, 32);
                break;
            }
            case ElemID: {
                // 解析元素段
                // 元素段用于存放表初始化数据
                // 元素项包含三部分：1.表索引（初始化哪张表）2.表内偏移量（从哪开始初始化）3. 函数索引列表（给定的初始化数据）

                // 元素段编码格式如下：
                // elem_sec: 0x09|byte_count|vec<elem>
                // elem: table_idx|offset_expr|vec<func_id>

                // 读取元素数量
                u32 elem_count = read_LEB128_unsigned(bytes, &pos, 32);

                // 依次对表中每个元素进行初始化
                for (u32 c = 0; c < elem_count; c++) {
                    // 读取表索引 table_idx（即初始化哪张表）
                    u32 index = read_LEB128_unsigned(bytes, &pos, 32);
                    // 目前 Wasm 版本规定一个模块只能定义一张表，所以 index 只能为 0
                    ASSERT(index == 0, "Only 1 default table in MVP\n")

                    // 计算初始化表达式 offset_expr，并将计算结果设置为当前表内偏移量 offset
                    run_init_expr(m, TYPE_I32, &pos);

                    // 计算初始化表达式 offset_expr 也就是栈式虚拟机执行表达式的字节码中的指令流过程，最终操作数栈顶保存的就是表达式的返回值，即计算结果
                    // 将栈顶的值弹出并赋值给当前表内偏移量 offset
                    u32 offset = m->operand_stack[m->sp--].value.uint32;

                    // 函数索引列表（即给定的元素初始化数据）
                    u32 num_elem = read_LEB128_unsigned(bytes, &pos, 32);
                    // 遍历函数索引列表，将列表中的函数索引设置为元素的初始值
                    for (u32 n = 0; n < num_elem; n++) {
                        m->table.entries[offset + n] = read_LEB128_unsigned(bytes, &pos, 32);
                    }
                }
                pos = start_pos + slen;
                break;
            }
            case CodeID: {
                // 解析代码段
                // 代码段用于存放函数的字节码和局部变量，是 Wasm 二进制模块的核心，其他段存放的都是辅助信息
                // 为了节约空间，局部变量的信息是被压缩的：即连续多个相同类型的局部变量会被统一记录变量数量和类型

                // 代码段编码格式如下：
                // code_sec: 0xoA|byte_count|vec<code>
                // code: byte_count|vec<locals>|expr
                // locals: local_count|val_type

                // 读取代码段中的代码项的数量
                u32 code_count = read_LEB_unsigned(bytes, &pos, 32);

                // 声明局部变量的值类型
                u8 val_type;

                // 遍历代码段中的每个代码项，解析对应数据
                for (u32 c = 0; c < code_count; c++) {
                    // 获取代码项
                    Block *function = &m->functions[m->import_func_count + c];

                    // 读取代码项所占字节数（暂用 4 个字节）
                    u32 code_size = read_LEB_unsigned(bytes, &pos, 32);

                    // 保存当前位置为代码项的起始位置（除去前面的表示代码项目长度的 4 字节）
                    u32 payload_start = pos;

                    // 读取 locals 数量（注：相同类型的局部变量算一个 locals）
                    u32 local_count = read_LEB_unsigned(bytes, &pos, 32);

                    u32 save_pos, lidx, lecount;

                    // 接下来需要对局部变量的相关字节进行两次遍历，所以先保存当前位置，方便第二次遍历前恢复位置
                    save_pos = pos;

                    // 将代码项的局部变量数量初始化为 0
                    function->local_count = 0;

                    // 第一次遍历所有的 locals，目的是统计代码项的局部变量数量，将所有 locals 所包含的变量数量相加即可
                    // 注：相同类型的局部变量算一个 locals
                    for (u32 l = 0; l < local_count; l++) {
                        // 读取单个 locals 所包含的变量数量
                        lecount = read_LEB_unsigned(bytes, &pos, 32);

                        // 累加 locals 所对应的局部变量的数量
                        function->local_count += lecount;

                        // 局部变量的数量后面接的是局部变量的类型，暂时不需要，标记为无用
                        val_type = read_LEB_unsigned(bytes, &pos, 7);
                        (void) val_type;
                    }

                    // 为保存函数局部变量的值类型的 function->locals 数组申请内存
                    function->locals = acalloc(function->local_count, sizeof(u32), "function->locals");

                    // 恢复之前的位置，重新遍历所有的 locals
                    pos = save_pos;

                    // 将局部变量的索引初始化为 0
                    lidx = 0;

                    // 第二次遍历所有的 locals，目的是所有的代码项中所有的局部变量设置值类型
                    for (u32 l = 0; l < local_count; l++) {
                        // 读取单个 locals 所包含的变量数量
                        lecount = read_LEB_unsigned(bytes, &pos, 32);

                        // 读取单个 locals 的值类型
                        val_type = read_LEB_unsigned(bytes, &pos, 7);

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
                    pos = function->end_addr + 1;
                }
                break;
            }
            case DataID: {
                // 解析数据段
                // 数据段用于存放内存的初始化数据
                // 元素项包含三部分：1.内存索引（初始化哪块内存）2. 内存偏移量（从哪里开始初始化）3. 初始化数据

                // 数据段编码格式如下：
                // data_sec: 0x09|byte_count|vec<data>
                // data: mem_idx|offset_expr|vec<byte>

                // 读取数据数量
                u32 mem_count = read_LEB_unsigned(bytes, &pos, 32);

                // 依次对内存中每个部分进行初始化
                for (u32 s = 0; s < mem_count; s++) {
                    // 读取内存索引 mem_idx（即初始化哪块内存）
                    u32 index = read_LEB_unsigned(bytes, &pos, 32);
                    // 目前 Wasm 版本规定一个模块只能定义一块内存，所以 index 只能为 0
                    ASSERT(index == 0, "Only 1 default memory in MVP\n")

                    // 计算初始化表达式 offset_expr，并将计算结果设置为当前内存偏移量 offset
                    run_init_expr(m, I32, &pos);

                    // 计算初始化表达式 offset_expr 也就是栈式虚拟机执行表达式的字节码中的指令流过程，最终操作数栈顶保存的就是表达式的返回值，即计算结果
                    // 将栈顶的值弹出并赋值给当前内存偏移量 offset
                    u32 offset = m->stack[m->sp--].value.uint32;

                    // 读取初始化数据所占内存大小
                    u32 size = read_LEB_unsigned(bytes, &pos, 32);

                    // 将写在二进制文件中的初始化数据拷贝到指定偏移量的内存中
                    memcpy(m->memory.bytes + offset, bytes + pos, size);
                    pos += size;
                }
                break;
            }
            default: {
                // 如果没有匹配到任何段，则只需 pos 增加相应值即可
                pos += slen;
                // 如果不是上面 0 到 11 ID，则报错
                FATAL("Section %d unimplemented\n", id)
            }
        }
    }

    // 收集所有本地模块定义的函数中 Block_/Loop/If 控制块的相关信息，例如起始地址、结束地址、跳转地址、控制块类型等，
    // 便于后续虚拟机解释执行指令时可以借助这些信息
    find_blocks(m);

    // 起始函数 m->start_function 是在【模块完成初始化后】，【被导出函数可调用之前】自动被调用的函数
    // 可以将起始函数视为一种初始化全局变量或内存的函数，且起始函数必须处于本地模块内部，不能是从外部导入的函数

    // m->start_function 初始赋值为 -1
    // 在解析 Wasm 二进制文件中的起始段时，start_function 会被赋值为起始段中保存的起始函数索引（在本地模块所有函数的索引）
    // 所以 m->start_function 不为 -1，说明本地模块存在起始函数，
    // 需要在本地模块已完成初始化后，且本地模块的导出函数被调用之前，执行本地模块的起始函数
    if (m->start_function != -1) {
        // 保存起始函数索引到 fidx
        u32 fidx = m->start_function;
        bool result;

        // 起始函数必须处于本地模块内部，不能是从外部导入的函数
        // 注：从外部模块导入的函数在本地模块的所有函数中的前部分，可参考上面解析 Wasm 二进制文件导入段中处理外部模块导入函数的逻辑
        ASSERT(fidx >= m->import_func_count, "Start function should be local function of native module\n")

        // 调用 Wasm 模块的起始函数
        result = invoke(m, fidx);

        // 虚拟机在执行起始函数的字节码中的指令，如果遇到错误会返回 false，否则顺利执行完成后会返回 true
        // 如果为 false，则将运行时（虚拟机执行指令过程）收集的异常信息打印出来
        if (!result) {
            FATAL("Exception: %s\n", exception)
        }
    }

    return m;
}
