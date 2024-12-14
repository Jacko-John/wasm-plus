#ifndef WSAM_SECTION_H
#define WSAM_SECTION_H

#include "module.h"
#include "utils.h"

u32 parse_table_type(Module *m, u32 *pos);
void parse_memory_type(Module *m, u32 *pos);

void read_type_section(Module *m, const u8 *bytes, u32 *pos);
void read_import_section(Module *m, const u8 *bytes, u32 *pos);
void read_function_section(Module *m, const u8 *bytes, u32 *pos);
void read_table_section(Module *m, const u8 *bytes, u32 *pos);
void read_memory_section(Module *m, const u8 *bytes, u32 *pos);
void read_global_section(Module *m, const u8 *bytes, u32 *pos);
void read_export_section(Module *m, const u8 *bytes, u32 *pos);
// void read_start_section(Module *m, const u8 *bytes, u32 *pos);
void read_element_section(Module *m, const u8 *bytes, u32 *pos);
void read_code_section(Module *m, const u8 *bytes, u32 *pos);
void read_data_section(Module *m, const u8 *bytes, u32 *pos);

#endif /* WSAM_SECTION_H */