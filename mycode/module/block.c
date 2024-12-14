#include "../include/instructions.h"
#include "../include/utils.h"

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

                    // 由于 Block_/Loop/If 操作码的立即数用于表示该控制块的类型
                    // 所以可以根据该立即数，来获取控制块的类型，即控制块的返回值的数量和类型

                    // get_block_type 根据表示该控制块的类型的值，返回控制块的签名，即控制块的返回值的数量和类型
                    // wasm中的块可以具有参数和返回值，块后面紧跟的就是返回值类型
                    // 0x40 :u8 表示没有返回值
                    // 0x7c :u8 表示一个 f64 类型返回值
                    // 0x7d :u8 表示一个 f32 类型返回值
                    // 0x7e :u8 表示一个 i64 类型返回值
                    // 0x7f :u8 表示一个 i32 类型返回值
                    // x    :s33 表示在type section中定义的函数签名的索引
                    block->type = get_block_type(m, m->bytes, pos + 1);
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


void skip_immediate_FC(const u8 *bytes, u32 *pos) {
    u32 opcode = read_LEB128_unsigned(bytes, pos, 32);
    u32 count;
    switch (opcode) {
        case MemoryInit:
            // MemoryInit 指令的立即数有两个，第一个立即数表示数据的索引（占 4 个字节），
            // 第二个立即数表示内存的索引，目前必须是 0
            read_LEB128_unsigned(bytes, pos, 32);
            *pos += 1;
            break;
        case DataDrop:
            // DataDrop 指令的立即数表示要丢弃的数据的索引（占 4 个字节）
            read_LEB128_unsigned(bytes, pos, 32);
            break;
        case MemoryCopy:
            // MemoryCopy 指令的立即数有两个，第一个立即数表示源内存的索引，第二个立即数表示目的内存的索引
            *pos += 2;
            break;
        case MemoryFill:
            // MemoryFill 指令的立即数有一个，表示内存的索引
            *pos += 1;
            break;
        case TableInit:
            // TableInit 指令的立即数有两个，第一个表示元素的索引，第二个表示表的索引
            read_LEB128_unsigned(bytes, pos, 32);
            read_LEB128_unsigned(bytes, pos, 32);
            break;
        case ElemDrop:
            // ElemDrop 指令的立即数表示要丢弃的元素的索引（占 4 个字节）
            read_LEB128_unsigned(bytes, pos, 32);
            break;
        case TableCopy:
            // TableCopy 指令的立即数有两个，第一个表示目的表的索引，第二个表示源表的索引
            read_LEB128_unsigned(bytes, pos, 32);
            read_LEB128_unsigned(bytes, pos, 32);
            break;
        case TableGrow:
        case TableSize:
        case TableFill:
            // TableFill 指令的立即数有一个，表示表的索引
            read_LEB128_unsigned(bytes, pos, 32);
            break;
        default:
            // 其他指令没有立即数
            break;
    }
}

void skip_immediate_FD(const u8 *bytes, u32 *pos) {
    u32 opcode = read_LEB128_unsigned(bytes, pos, 32);
    u32 count;
    switch (opcode) {
        case V128Load ... V128Load64Splat:
        case V128Load32Zero:
        case V128Load64Zero:
        case V128Store:
            read_LEB128_unsigned(bytes, pos, 32);
            read_LEB128_unsigned(bytes, pos, 32);
            break;
        case V128Load8Lane ... V128Store64Lane:
            read_LEB128_unsigned(bytes, pos, 32);
            read_LEB128_unsigned(bytes, pos, 32);
            *pos += 1;
            break;
        case V128Const:
        case I8x16Shuffle:
            *pos += 16;
            break;
        case I8x16ExtractLaneS ... F64x2ReplaceLane:
            *pos += 1;
            break;
        default:
            // 其他指令没有立即数
            break;
    }
}

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
            *pos += 1;
            break;
        case Br:
        case BrIf:
            // 跳转指令的立即数表示跳转的目标标签索引（占 4 个字节）
            read_LEB128_unsigned(bytes, pos, 32);
            break;
        case BrTable:
            // BrTable 指令的立即数是指定的 n+1 个跳转的目标标签索引（每个索引值占 4 个字节）
            // 其中前 n 个目标标签索引构成一个索引表，最后 1 个标签索引为默认索引
            // 最终跳转到哪一个目标标签索引，需要在运行期间才能决定
            count = read_LEB128_unsigned(bytes, pos, 32);
            for (u32 i = 0; i < count; i++) {
                read_LEB128_unsigned(bytes, pos, 32);
            }
            read_LEB128_unsigned(bytes, pos, 32);
            break;
        case Call:
            // Call 指令的立即数表示被调用函数的索引（占 4 个字节）
            read_LEB128_unsigned(bytes, pos, 32);
            break;
        case CallIndirect:
            // CallIndirect 指令有两个立即数，第一个立即数表示被调用函数的类型索引（占 4 个字节），
            // 第二个立即数为函数所在表的下标
            read_LEB128_unsigned(bytes, pos, 32);
            read_LEB128_unsigned(bytes, pos, 32);
            break;
        case RefNull:
            // RefNull 指令的立即数表示空引用的类型索引（占 1 个字节）
            *pos += 1;
            break;
        case RefFunc:
            // RefFunc 指令的立即数表示函数引用的函数索引（占 4 个字节）
            read_LEB128_unsigned(bytes, pos, 32);
            break;
        case SelectType:
            // SelectType 指令选择可能的值类型，为vec(type)类型
            count = read_LEB128_unsigned(bytes, pos, 32);
            *pos += count;
            break;
        case LocalGet:
        case LocalSet:
        case LocalTee:
        case GlobalGet:
        case GlobalSet:
            // 变量指令的立即数用于表示全局/局部变量的索引（占 4 个字节）
            read_LEB128_unsigned(bytes, pos, 32);
            break;
        case TableGet:
        case TableSet:
            // 表指令的立即数用于表示表的索引（占 4 个字节）
            read_LEB128_unsigned(bytes, pos, 32);
            break;
        case I32Load ... I64Store32:
            // 内存加载/存储指令有两个立即数，第一个立即数表示内存偏移量（占 4 个字节），
            // 第二个立即数表示对齐提示（占 4 个字节）
            read_LEB128_unsigned(bytes, pos, 32);
            read_LEB128_unsigned(bytes, pos, 32);
            break;
        case MemorySize:
        case MemoryGrow:
            // 内存大小/增加指令的立即数表示所操作的内存索引（占 1 个字节）
            // 由于当前 Wasm 规范规定一个模块最多只能导入或定义一块内存，所以目前必须为 0
            *pos += 1;
            break;
        case I32Const:
            // I32Const 指令的立即数表示 32 有符号整数（占 4 个字节）
            read_LEB128_signed(bytes, pos, 32);
            break;
        case I64Const:
            // F32Const 指令的立即数表示 64 有符号整数（占 8 个字节）
            read_LEB128_signed(bytes, pos, 64);
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
        case CompInstrFC_:
            // 双字节指令0xFC
            skip_immediate_FC(bytes, pos);
            break;
        case CompInstrFD_:
            // 双字节指令0xFD
            skip_immediate_FD(bytes, pos);
            break;
        default:
            // 其他操作码没有立即数
            // 注：Wasm 指令大部分指令没有立即数
            break;
    }
}
