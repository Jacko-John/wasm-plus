#ifndef WASMC_MODULE_H
#define WASMC_MODULE_H

#define true 1
#define false 0
typedef signed int i32;
typedef signed long long i64;
typedef unsigned int u32;
typedef unsigned long long u64;
typedef unsigned char u8;
typedef double f64;
typedef float f32;
typedef unsigned char boolean;

#define unlikely(x) __builtin_expect(!!(x), 0)
#define likely(x) __builtin_expect(!!(x), 1)

#define WASM_MAGIC_CODE 0x6d736100// 代表这是一个wasm文件
#define WASM_VERSION 0x01         // 版本号

#define PAGE_SIZE 0x10000 // 页大小64KB
#define STACK_SIZE 0x10000// 栈大小64KB

#define CALL_STACK_SIZE 0x1000 // 调用栈大小4KB
#define BLOCK_STACK_SIZE 0x1000// 块栈大小4KB
#define BR_Table_SIZE 0x1000   // 跳转表大小64KB

#define TYPE_I32 0x7f  // 32位整型   mask 0x01
#define TYPE_I64 0x7e  // 64位整型   mask 0x02
#define TYPE_F32 0x7d  // 32位浮点型 mask 0x03
#define TYPE_F64 0x7c  // 64位浮点型 mask 0x04
#define TYPE_V128 0x7b // 128位整型  mask 0x05
#define TYPE_EMPTY 0x40// 空类型     mask 0x40

#define TYPE_FUNC 0x60    // 函数类型      mask 0x20
#define TYPE_REF_FUNC 0x70// 函数引用类型  mask 0x10

#define IMPORT_FUNC 0x00  // 导入函数
#define IMPORT_TABLE 0x01 // 导入表
#define IMPORT_MEM 0x02   // 导入内存
#define IMPORT_GLOBAL 0x03// 导入全局变量

typedef enum {
    CustomID,// 自定义段 ID
    TypeID,  // 类型段 ID
    ImportID,// 导入段 ID
    FuncID,  // 函数段 ID
    TableID, // 表段 ID
    MemID,   // 内存段 ID
    GlobalID,// 全局段 ID
    ExportID,// 导出段 ID
    StartID, // 起始段 ID
    ElemID,  // 元素段 ID
    CodeID,  // 代码段 ID
    DataID   // 数据段 ID
} SectionID;

// 控制块（包含函数）签名结构体
// 注：目前多返回值提案还没有进入 Wasm 标准，根据当前版本的 Wasm 标准，非函数类型的控制块不能有参数，且最多只能有一个返回值
typedef struct {
    u32 param_cnt; // 参数数量
    u32 *params;   // 参数类型集合
    u32 result_cnt;// 返回值数量
    u32 *results;  // 返回值类型集合
    u64 mask;      // 基于控制块（包含函数）签名计算的唯一掩码值
} Type;

// 控制块（包含函数）结构体
typedef struct {
    u8 block_type;// 控制块类型，包含 5 种，分别是 0x00: function, 0x01: init_exp, 0x02: block, 0x03: loop, 0x04: if
    Type *type;   // 控制块签名，即控制块的返回值的数量和类型
    u32 func_idx; // 函数在所有函数中的索引（仅针对控制块类型为函数的情况）

    u32 local_count;// 局部变量数量（仅针对控制块类型为函数的情况）
    u32 *locals;    // 用于存储局部变量的值（仅针对控制块类型为函数的情况）

    u32 start_addr;// 控制块中字节码部分的【起始地址】
    u32 end_addr;  // 控制块中字节码部分的【结束地址】
    u32 else_addr; // 控制块中字节码部分的【else 地址】(仅针对控制块类型为 if 的情况)
    u32 br_addr;   // 控制块中字节码部分的【跳转地址】

    char *import_module;// 导入函数的导入模块名（仅针对从外部模块导入的函数）
    char *import_field; // 导入函数的导入成员名（仅针对从外部模块导入的函数）
    void *(*func_ptr)();// 导入函数的实际值（仅针对从外部模块导入的函数）
} Block;

// 表结构体
typedef struct {
    u8 elem_type;// 表中元素的 ref 类型（函数引用：0x70，外部对象引用：0x6f，这里只考虑函数引用）
    u32 min_size;// 表的元素数量限制下限
    u32 max_size;// 表的元素数量限制上限
    u32 cur_size;// 表的当前元素数量
    u32 *entries;// 用于存储表中的元素
} Table;

// 内存结构体
typedef struct {
    u32 min_size;// 最小页数
    u32 max_size;// 最大页数
    u32 cur_size;// 当前页数
    u8 *bytes;   // 用于存储数据
} Memory;

// 导出项结构体
typedef struct {
    char *name; // 导出项成员名
    u32 type;   // 导出项类型（类型可以是函数/表/内存/全局变量）
    void *value;// 用于存储导出项的值
} Export;

// 全局变量值 / 操作数栈的值结构体
typedef struct {
    u8 type;// 值类型
    u8 mut; // 是否可变
    union {
        u32 uint32;
        i32 int32;
        u64 uint64;
        i64 int64;
        float f32;
        double f64;
    } value;// 值
} Value;

/*
* 栈式虚拟机运行时的执行单位为 frame, 即栈帧，
* 每个 frame 约等于一个控制块，只有在栈顶的控制块才是当前正在执行的块，当执行完之后就会将其弹出，
* 每个 frame 都有三个属性：
*   sp: 上一个 frame 的【操作数栈顶指针】的值
*   fp: 上一个 frame 的【操作数栈底指针】的值
*   ret_addr: 用于保存【函数返回地址】，即【该栈帧调用指令的下一条指令的地址】，
*/
// 栈帧结构体
typedef struct {
    Block *block;// 栈帧对应的控制块（包含函数）结构体
    // 下面三个属性是在该栈帧被压入调用栈顶时，保存的当时的运行时的状态，
    // 目的是为了在该栈帧关联的控制块执行完成，该栈帧弹出时，恢复压栈之前的运行时状态
    int sp;
    int fp;
    u32 ret_addr;// return address 用于保存【函数返回地址】，即【该栈帧调用指令的下一条指令的地址】，
                 // 也就是该栈帧被压入操作数栈顶前的 m->pc 的值
                 // 换句话说，当前函数被弹出之后栈顶就是调用该函数的栈帧，但是我们不能够通过栈帧找到调用该函数的地址，需要通过ret_addr来找到
                 // 注：该属性均针对类型为函数的控制块（只有函数执行完才会返回），其他类型的控制块没有该属性
} StackFrame;

// Wasm 内存格式结构体
typedef struct {
    const u8 *bytes;// 用于存储 Wasm 二进制模块的内容
    u32 byte_cnt;   // Wasm 二进制模块的字节数

    Type *func_types;// 用于存储模块中所有函数签名
    u32 type_cnt;    // 模块中所有函数签名的数量

    u32 import_func_cnt;// 导入函数的数量
    u32 function_cnt;   // 所有函数的数量（包括导入函数）
    u32 start_func;     // 起始函数在本地模块所有函数中索引，而起始函数是在【模块完成初始化后】，【被导出函数可调用之前】自动被调用的函数
    Block *funcs;       // 用于存储模块中所有函数（包括导入函数和模块内定义函数）
    // 很蠢的做法，后续优化为在函数内部使用一个 pair 存储，然后二分查找。
    Block **block_lookup;// 模块中所有 Block 的 map，其中 key 为为对应操作码 Block_/Loop/If 的地址

    Table *tables;// 表
    u32 table_cnt;// 表数量

    Memory memory;// 内存

    Value *globals;// 用于存储全局变量的相关数据（值以及值类型等）
    u32 global_cnt;// 全局变量的数量

    Export *exports;// 用于存储导出项的相关数据（导出项的值、成员名以及类型等）
    u32 export_cnt; // 导出项数量

    // 下面属性用于记录运行时（即栈式虚拟机执行指令流的过程）状态，相关背景知识请查看上面栈帧结构体的注释
    u32 pc;// program counter 程序计数器，记录下一条即将执行的指令的地址

    u32 sp;                         // operand stack pointer 操作数栈顶指针，指向完整的操作数栈顶（注：所有栈帧共享一个完整的操作数栈，分别占用其中的某一部分）
    u32 fp;                         // current frame pointer into stack 当前栈帧的帧指针，指向当前栈帧的操作数栈底
    Value operand_stack[STACK_SIZE];// operand stack 操作数栈，用于存储参数、局部变量、操作数

    u32 call_stack_ptr;                    // callstack pointer 调用栈指针，保存处在调用栈顶的栈帧索引，即当前栈帧在调用栈中的索引
    StackFrame call_stack[CALL_STACK_SIZE];// callstack 调用栈，用于存储栈帧
    u32 br_Table[BR_Table_SIZE];           // 跳转指令索引表
} Module;

// 解析 Wasm 二进制文件内容，将其转化成内存格式 Module
Module *load_module(const u8 *bytes, u32 byte_cnt);

#endif// WASMC_MODULE_H