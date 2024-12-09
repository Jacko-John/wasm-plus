#include "../include/utils.h"

u64 read_LEB128(const u8 *bytes, u32 *pos, u32 maxbits, boolean sign) {
    u64 result = 0;
    u32 shift = 0;
    u32 bcnt = 0;
    u32 startpos = *pos;
    u64 byte;

    while (true) {
        byte = bytes[*pos];
        *pos += 1;
        // 取字节中后 7 位作为值插入到 result 中，按照小端序，即低位字节在前，高位字节在后
        result |= ((byte & 0x7f) << shift);
        shift += 7;
        // 如果某个字节的最高位为 0，即和 0x80 相与结果为 0，则表示该字节为最后一个字节，没有后续字节了
        if ((byte & 0x80) == 0) {
            break;
        }
        bcnt++;
        // (maxbits + 7 - 1) / 7 表示要表示 maxbits 位二进制数字所需要的字节数减 1
        // 由于 bcnt 是从 0 开始
        // 所以 bcnt > (maxbits + 7 - 1) / 7 表示该次循环所得到的字节数 bcnt + 1 已经超过了 maxbits 位二进制数字所需的字节数 (maxbits + 7 - 1) / 7 + 1
        // 也就是该数字的位数超出了传入的最大位数值，所以报错
        if (bcnt > (maxbits + 7 - 1) / 7) {
            FATAL("Unsigned LEB at byte %d overflow", startpos)
        }
    }

    // 如果是有符号整数，针对于最后一个字节，则需要
    if (sign && (shift < maxbits) && (byte & 0x40)) {
        result |= -(1 << shift);
    }
    return result;
}

u64 read_LEB128_signed(const u8 *bytes, u32 *pos, u32 maxbits) {
    return read_LEB128(bytes, pos, maxbits, true);
}

u64 read_LEB128_unsigned(const u8 *bytes, u32 *pos, u32 maxbits) {
    return read_LEB128(bytes, pos, maxbits, false);
}

// 从字节数组中读取字符串，其中字节数组的开头 4 个字节用于表示字符串的长度
// 注：如果参数 result_len 不为 NULL，则会被赋值为字符串的长度
char *read_string(const u8 *bytes, u32 *pos, u32 *result_len) {
    // 读取字符串的长度
    u32 str_len = read_LEB128_unsigned(bytes, pos, 32);
    // 为字符串申请内存
    char *str = malloc(str_len + 1);
    // 将字节数组的数据拷贝到字符串 str 中
    memcpy(str, bytes + *pos, str_len);
    // 字符串以字符 '\0' 结尾
    str[str_len] = '\0';
    // 字节数组位置增加相应字符串长度
    *pos += str_len;
    // 如果参数 result_len 不为 NULL，则会被赋值为字符串的长度
    if (result_len) {
        *result_len = str_len;
    }
    return str;
}