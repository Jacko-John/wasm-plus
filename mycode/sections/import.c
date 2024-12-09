#include "../include/sections.h"

void read_import_section(Module *m, const u8 *bytes, u32 *pos) {
    // 解析导入段
    // 一个模块可以从其他模块导入 4 种类型：函数、表、内存、全局变量
    // 导入项包含 4 个：1.模块名（从哪个模块导入）2.成员名 3. 具体描述
    // 导入段在模块的开头，所以直接使用导入段来初始化模块的相关信息

    // 导入段编码格式如下：
    // import_sec: 0x02|byte_count|vec<import>
    // import: module_name|member_name|import_desc
    // import_desc: tag|[type_idx, table_type, mem_type, global_type]

    // 读取导入项数量
    u32 import_cnt = read_LEB128_unsigned(bytes, pos, 32);

    // 遍历所有导入项，解析对应数据
    for (u32 idx = 0; idx < import_cnt; idx++) {
        u32 module_len, field_len;

        // 读取模块名 module_name（从哪个模块导入）
        char *import_module = read_string(bytes, pos, &module_len);

        // 读取导入项的成员名 member_name
        char *import_field = read_string(bytes, pos, &field_len);

        // 读取导入项类型 tag（四种类型：函数、表、内存、全局变量）
        u32 extern_type = bytes[(*pos)++];

        u32 type_index, fidx;
        u8 global_type, mutability;

        // 根据不同的导入项类型，读取对应的内容
        switch (extern_type) {
            case IMPORT_FUNC:
                // 读取函数签名索引 type_idx
                type_index = read_LEB128_unsigned(bytes, pos, 32);
                break;
            case IMPORT_TABLE:
                // 解析表段中的表 table_type（目前表段只会包含一张表）
                parse_table_type(m, pos);
                break;
            case IMPORT_MEM:
                // 解析内存段中内存 mem_type（目前模块只会包含一块内存）
                parse_memory_type(m, pos);
                break;
            case IMPORT_GLOBAL:
                // 先读取全局变量的值类型 global_type
                global_type = read_LEB128_unsigned(bytes, pos, 7);

                // 再读取全局变量的可变性
                mutability = read_LEB128_unsigned(bytes, pos, 1);
                break;
            default:
                break;
        }

        void *val;
        char *err, *sym = malloc(module_len + field_len + 5);

        // 尝试从导入的模块中查找导入项，并将导入项的值赋给 val
        // 第一个句柄参数为模块名 import_module
        // 第二个符号参数为成员名 import_field
        // resolve_sym 函数中，如果从外部模块中找到导入项，则会将导入项的值赋给 val，并返回 true
        if (!resolve_sym(import_module, import_field, &val, &err)) {
            FATAL("Error: %s\n", err);// 如果未找到，则报错
        }

        free(sym);

        // 根据导入项类型，将导入项的值保存到对应的地方
        switch (extern_type) {
            case IMPORT_FUNC:
                // 导入项为导入函数的情况

                // 获取当前导入函数在本地模块所有函数中的索引
                fidx = m->function_cnt;

                // 本地模块的函数数量和导入函数数量均加 1
                m->import_func_cnt += 1;
                m->function_cnt += 1;

                // 为当前的导入函数对应在本地模块的函数申请内存
                m->funcs = arecalloc(m->funcs, fidx, m->import_func_cnt, sizeof(Block), "Block(imports)");
                // 获取当前的导入函数对应在本地模块的函数
                Block *func = &m->funcs[fidx];
                // 设置【导入函数的导入模块名】为【本地模块中对应函数的导入模块名】
                func->import_module = import_module;
                // 设置【导入函数的导入成员名】为【本地模块中对应函数的导入成员名】
                func->import_field = import_field;
                // 设置【本地模块中对应函数的指针 func_ptr】指向【导入函数的实际值】
                func->func_ptr = val;
                // 设置【导入函数签名】为【本地模块中对应函数的函数签名】
                func->type = &m->func_types[type_index];
                break;
            case IMPORT_TABLE:
                // 导入项为表的情况

                // 一个模块只能定义一张表，如果 m->table.entries 不为空，说明已经存在表，则报错
                ASSERT(!m->table.entries, "More than 1 table not supported\n")
                Table *tval = val;
                m->table.entries = val;
                // 如果【本地模块的表的当前元素数量】大于【导入表的元素数量上限】，则报错
                ASSERT(m->table.cur_size <= tval->max_size, "Imported table is not large enough\n")
                m->table.entries = *(u32 **) val;
                // 设置【导入表的当前元素数量】为【本地模块表的当前元素数量】
                m->table.cur_size = tval->cur_size;
                // 设置【导入表的元素数量限制上限】为【本地模块表的元素数量限制上限】
                m->table.max_size = tval->max_size;
                // 设置【导入表的存储的元素】为【本地模块表的存储的元素】
                m->table.entries = tval->entries;
                break;
            case IMPORT_MEM:
                // 导入项为内存的情况

                // 一个模块只能定义一块内存，如果 m->memory.bytes 不为空，说明已经存在表，则报错
                ASSERT(!m->memory.bytes, "More than 1 memory not supported\n")
                Memory *mval = val;
                // 如果【本地模块的内存的当前页数】大于【导入内存的最大页数】，则报错
                ASSERT(m->memory.cur_size <= mval->max_size, "Imported memory is not large enough\n")
                // 设置【导入内存的当前页数】为【本地模块内存的当前页数】
                m->memory.cur_size = mval->cur_size;
                // 设置【导入内存的最大页数】为【本地模块内存的最大页数】
                m->memory.max_size = mval->max_size;
                // 设置【导入内存的存储的数据】为【本地模块内存的存储的数据】
                m->memory.bytes = mval->bytes;
                break;
            case IMPORT_GLOBAL:
                // 导入项为全局变量的情况

                // 本地模块的全局变量数量加 1
                m->global_cnt += 1;

                // 为全局变量申请内存，在原有模块本身的全局变量基础上，再添加导入的全局变量对应的全局变量
                m->globals = arecalloc(m->globals, m->global_cnt - 1, m->global_cnt, sizeof(Value), "globals");
                // 获取当前的导入全局变量对应在本地模块中的全局变量
                Value *glob = &m->globals[m->global_cnt - 1];
                // 设置【导入全局变量的值类型】为【本地模块中对应全局变量的值类型】
                // 注：变量的值类型主要为 I32/I64/F32/F64
                glob->type = global_type;
                glob->mut = mutability;
                // 根据全局变量的值类型，设置【导入全局变量的值】为【本地模块中对应全局变量的值】
                switch (global_type) {
                    case TYPE_I32:
                        memcpy(&glob->value.uint32, val, 4);
                        break;
                    case TYPE_I64:
                        memcpy(&glob->value.uint64, val, 8);
                        break;
                    case TYPE_F32:
                        memcpy(&glob->value.f32, val, 4);
                        break;
                    case TYPE_F64:
                        memcpy(&glob->value.f64, val, 8);
                        break;
                    default:
                        break;
                }
                break;
            default:
                // 如果导入项为其他类型，则报错
                FATAL("Import of kind %d not supported\n", extern_type)
        }
    }
}