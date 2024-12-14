#include "../include/module.h"
#include "../include/instructions.h"
#include "../include/sections.h"
#include "../include/utils.h"

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
    // ? 逆天内存占用
    m->block_lookup = acalloc(m->byte_cnt, sizeof(Block *), "function->block_lookup");

    m->type_cnt = 0;
    m->import_func_cnt = 0;
    m->function_cnt = 0;
    m->export_cnt = 0;
    m->global_cnt = 0;
    m->table_cnt = 0;

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
                // elem:
                //        0|expr|vec<func_idx>                       => active mode in table 0
                //        1|elem_kind|vec<func_idx>                  => passive mode
                //        2|table_idx|expr|elem_kind|vec<func_idx>   => active mode with table index
                //        3|elem_kind|vec<func_idx>                  => declarative mode
                //        4|expr|vec<expr>                           => active mode in table 0
                //        5|ref_type|vec<expr>                       => passive mode
                //        6|table_idx|expr|ref_type|vec<expr>        => active mode with table index
                //        7|ref_type|vec<expr>                       => declarative mode
                // elem_kind: 0x00 => funcref

                // 读取元素数量
                u32 elem_count = read_LEB128_unsigned(bytes, &pos, 32);

                u32 offset, num_elem;
                Table *table;
                // 依次对表中每个元素进行初始化
                for (u32 c = 0; c < elem_count; c++) {
                    // 读取元素项的类型
                    u32 elem_type = read_LEB128_unsigned(bytes, &pos, 32);
                    switch (elem_type) {
                        case 0:
                            run_init_expr(m, TYPE_I32, &pos);
                            offset = m->operand_stack[m->sp--].value.uint32;
                            table = &m->tables[0];
                            num_elem = read_LEB128_unsigned(bytes, &pos, 32);
                            for (u32 i = 0; i < num_elem; i++) {
                                table->entries[offset + i] = read_LEB128_unsigned(bytes, &pos, 32);
                            }
                            break;
                        case 1:
                            break;
                        default:
                            FATAL("Unsupported elem type %d\n", elem_type)
                    }
                }
                pos = start_pos + slen;
                break;
            }
            case CodeID: {
                read_code_section(m, bytes, &pos);
                ASSERT(pos - start_pos != slen, "Code section length mismatch\n")
                break;
            }
            case DataID: {
                // 解析数据段
                // 数据段用于存放内存的初始化数据
                // 元素项包含三部分：1.内存索引（初始化哪块内存）2. 内存偏移量（从哪里开始初始化）3. 初始化数据

                // 数据段编码格式如下：
                // data_sec: 0x0B|byte_count|vec<data>
                // data: data_mode | ...
                //       0:u32 | offset_expr | vec<byte>            => active mode
                //       1:u32 | vec<byte>                          => passive mode
                //       2:u32 | mem_idx | offset_expr | vec<byte>  => active mode with memory index

                // 读取数据数量
                u32 mem_count = read_LEB128_unsigned(bytes, &pos, 32);

                // 依次对内存中每个部分进行初始化
                for (u32 s = 0; s < mem_count; s++) {
                    // 读取内存索引 mem_idx（即初始化哪块内存）
                    u32 index = read_LEB128_unsigned(bytes, &pos, 32);
                    // 目前 Wasm 版本规定一个模块只能定义一块内存，所以 index 只能为 0
                    ASSERT(index == 0, "Only 1 default memory in MVP\n")

                    // 计算初始化表达式 offset_expr，并将计算结果设置为当前内存偏移量 offset
                    run_init_expr(m, TYPE_I32, &pos);

                    // 计算初始化表达式 offset_expr 也就是栈式虚拟机执行表达式的字节码中的指令流过程，最终操作数栈顶保存的就是表达式的返回值，即计算结果
                    // 将栈顶的值弹出并赋值给当前内存偏移量 offset
                    u32 offset = m->operand_stack[m->sp--].value.uint32;

                    // 读取初始化数据所占内存大小
                    u32 size = read_LEB128_unsigned(bytes, &pos, 32);

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
    if (m->start_func != -1) {
        // 保存起始函数索引到 fidx
        u32 fidx = m->start_func;
        boolean result;

        // 起始函数必须处于本地模块内部，不能是从外部导入的函数
        // 注：从外部模块导入的函数在本地模块的所有函数中的前部分，可参考上面解析 Wasm 二进制文件导入段中处理外部模块导入函数的逻辑
        ASSERT(fidx >= m->import_func_cnt, "Start function should be local function of native module\n")

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
