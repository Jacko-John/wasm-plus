#ifndef WASMC_UTILS_H
#define WASMC_UTILS_H

#include "module.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// typedef u32 size_t;

// 已使用fprintf, malloc, memcpy, memset, alloc, realloc, size_t, free, <dlfcn.h>

// 用于保存异常信息内容
extern char exception[];

// 报错
#define FATAL(...)                                             \
    {                                                          \
        fprintf(stderr, "Error(%s:%d): ", __FILE__, __LINE__); \
        fprintf(stderr, __VA_ARGS__);                          \
        exit(1);                                               \
    }

// 断言
#define ASSERT(exp, ...)                                                    \
    {                                                                       \
        if (!(exp)) {                                                       \
            fprintf(stderr, "Assert Failed (%s:%d): ", __FILE__, __LINE__); \
            fprintf(stderr, __VA_ARGS__);                                   \
            exit(1);                                                        \
        }                                                                   \
    }

#define ERROR(...) fprintf(stderr, __VA_ARGS__);

// 解码针对无符号整数的 LEB128 编码
u64 read_LEB128_unsigned(const u8 *bytes, u32 *pos, u32 maxbits);

// 解码针对有符号整数的 LEB128 编码
u64 read_LEB128_signed(const u8 *bytes, u32 *pos, u32 maxbits);

// 从字节数组中读取字符串，其中字节数组的开头 4 个字节用于表示字符串的长度
// 注：如果参数 result_len 不为 NULL，则会被赋值为字符串的长度
char *read_string(const u8 *bytes, u32 *pos, u32 *result_len);

// 申请内存
void *acalloc(size_t num, size_t size, char *name);

// 在原有内存基础上重新申请内存
void *arecalloc(void *ptr, size_t old_num, size_t num, size_t size, char *name);

// 查找动态库中的 symbol
// 如果解析成功则返回 true
// 如果解析失败则返回 false 并设置 err
boolean resolve_sym(char *filename, char *symbol, void **val, char **err);

// 基于函数签名计算唯一的掩码值
u64 get_type_mask(Type *type);

// 根据表示该控制块的类型的值（占一个字节），返回控制块的类型（或签名），即控制块的返回值的数量和类型
// 0x7f 表示有一个 i32 类型返回值、0x7e 表示有一个 i64 类型返回值、0x7d 表示有一个 f32 类型返回值、0x7c 表示有一个 f64 类型返回值、0x40 表示没有返回值
// 注：目前多返回值提案还没有进入 Wasm 标准，根据当前版本的 Wasm 标准，控制块不能有参数，且最多只能有一个返回值
Type *get_block_type(const Module *m, const u8 *bytes, u32 pos);

// 收集所有本地模块定义的函数中 Block_/Loop/If 控制块的相关信息，例如起始地址、结束地址、跳转地址、控制块类型等，
// 便于后续虚拟机解释执行指令时可以借助这些信息
void find_blocks(Module *m);

// 在单条指令中，除了占一个字节的操作码之外，后面可能也会紧跟着立即数，如果有立即数，则直接跳过立即数
// 注：指令是否存在立即数，是由操作数的类型决定，这也是 Wasm 标准规范的内容之一
void skip_immediate(const u8 *bytes, u32 *pos);

// 符号扩展 (sign extension)
// 分以下两种情况：
// 1. 将无符号数转换为更大的数据类型：
// 只需简单地在开头添加 0 至所需位数，这种运算称为 0 扩展
// 2. 将有符号数转换为更大的数据类型：
// 需要执行符号扩展，规则是将符号位扩展至所需的位数，即符号位为 0 时在开头添加 0 至所需位数，符号位为 1 时在开头添加 1 至所需位数，
// 例如 char: 1000 0000  --> short: 1111 1111 1000 0000

void sext_8_32(u32 *val);

void sext_16_32(u32 *val);

void sext_8_64(u64 *val);

void sext_16_64(u64 *val);

void sext_32_64(u64 *val);

// 32 位整型循环左移
u32 rotl32(u32 n, unsigned int c);

// 32 位整型循环右移
u32 rotr32(u32 n, unsigned int c);

// 64 位整型循环左移
u64 rotl64(u64 n, unsigned int c);

// 64 位整型循环右移
u64 rotr64(u64 n, unsigned int c);

// 32 位浮点型比较两数之间最大值
float wa_fmaxf(float a, float b);

// 32 位浮点型比较两数之间最小值
float wa_fminf(float a, float b);

// 64 位浮点型比较两数之间最大值
double wa_fmax(double a, double b);

// 64 位浮点型比较两数之间最小值
double wa_fmin(double a, double b);

// 非饱和截断
#define OP_TRUNC(RES, A, TYPE, RMIN, RMAX)                   \
    if (isnan(A)) {                                          \
        sprintf(exception, "invalid conversion to integer"); \
        return false;                                        \
    }                                                        \
    if ((A) <= (RMIN) || (A) >= (RMAX)) {                    \
        sprintf(exception, "integer overflow");              \
        return false;                                        \
    }                                                        \
    (RES) = (TYPE) (A);

#define OP_I32_TRUNC_F32(RES, A) OP_TRUNC(RES, A, i32, -2147483904.0f, 2147483648.0f)
#define OP_U32_TRUNC_F32(RES, A) OP_TRUNC(RES, A, u32, -1.0f, 4294967296.0f)
#define OP_I32_TRUNC_F64(RES, A) OP_TRUNC(RES, A, i32, -2147483649.0, 2147483648.0)
#define OP_U32_TRUNC_F64(RES, A) OP_TRUNC(RES, A, u32, -1.0, 4294967296.0)

#define OP_I64_TRUNC_F32(RES, A) OP_TRUNC(RES, A, i64, -9223373136366403584.0f, 9223372036854775808.0f)
#define OP_U64_TRUNC_F32(RES, A) OP_TRUNC(RES, A, u64, -1.0f, 18446744073709551616.0f)
#define OP_I64_TRUNC_F64(RES, A) OP_TRUNC(RES, A, i64, -9223372036854777856.0, 9223372036854775808.0)
#define OP_U64_TRUNC_F64(RES, A) OP_TRUNC(RES, A, u64, -1.0, 18446744073709551616.0)

// 饱和截断
// 注：和非饱和截断的区别在于，饱和截断会对异常情况做特殊处理。
// 比如将 NaN 转换为 0。再比如如果超出了类型能表达的范围，让该变量等于一个最大值或者最小值。
#define OP_TRUNC_SAT(RES, A, TYPE, RMIN, RMAX, IMIN, IMAX) \
    if (isnan(A)) {                                        \
        (RES) = 0;                                         \
    } else if ((A) <= (RMIN)) {                            \
        (RES) = IMIN;                                      \
    } else if ((A) >= (RMAX)) {                            \
        (RES) = IMAX;                                      \
    } else {                                               \
        (RES) = (TYPE) (A);                                \
    }

#define OP_I32_TRUNC_SAT_F32(RES, A) OP_TRUNC_SAT(RES, A, i32, -2147483904.0f, 2147483648.0f, INT32_MIN, INT32_MAX)
#define OP_U32_TRUNC_SAT_F32(RES, A) OP_TRUNC_SAT(RES, A, u32, -1.0f, 4294967296.0f, 0UL, UINT32_MAX)
#define OP_I32_TRUNC_SAT_F64(RES, A) OP_TRUNC_SAT(RES, A, i32, -2147483649.0, 2147483648.0, INT32_MIN, INT32_MAX)
#define OP_U32_TRUNC_SAT_F64(RES, A) OP_TRUNC_SAT(RES, A, u32, -1.0, 4294967296.0, 0UL, UINT32_MAX)

#define OP_I64_TRUNC_SAT_F32(RES, A) OP_TRUNC_SAT(RES, A, i64, -9223373136366403584.0f, 9223372036854775808.0f, INT64_MIN, INT64_MAX)
#define OP_U64_TRUNC_SAT_F32(RES, A) OP_TRUNC_SAT(RES, A, u64, -1.0f, 18446744073709551616.0f, 0ULL, UINT64_MAX)
#define OP_I64_TRUNC_SAT_F64(RES, A) OP_TRUNC_SAT(RES, A, i64, -9223372036854777856.0, 9223372036854775808.0, INT64_MIN, INT64_MAX)
#define OP_U64_TRUNC_SAT_F64(RES, A) OP_TRUNC_SAT(RES, A, u64, -1.0, 18446744073709551616.0, 0ULL, UINT64_MAX)

// 将 StackValue 类型数值用字符串形式展示，展示形式 "<value>:<value_type>"
char *value_repr(Value *v);

// 通过名称从 Wasm 模块中查找同名的导出项
void *get_export(Module *m, char *name);

// 打开文件并将文件映射进内存
u8 *mmap_file(char *path, int *len);

// 将字符串 str 按照空格拆分成多个参数
// 其中 argc 被赋值为拆分字符串 str 得到的参数数量
char **split_argv(char *str, int *argc);

// 解析函数参数，并将参数压入到操作数栈
void parse_args(Module *m, Type *type, int argc, char **argv);

#endif
