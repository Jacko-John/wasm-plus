#ifndef WASMC_INSTRUCTIONS_H
#define WASMC_INSTRUCTIONS_H

typedef enum {
    /*控制指令 */
    Unreachable = 0x00, // unreachable
    Nop = 0x01,         // nop
    Block_ = 0x02,      // block bt instr* end
    Loop = 0x03,        // loop bt instr* end
    If = 0x04,          // if bt instr* else instr* end
    Else_ = 0x05,       // else
    End_ = 0x0B,        // end
    Br = 0x0C,          // br l
    BrIf = 0x0D,        // br_if l
    BrTable = 0x0E,     // br_table label_table* label_index
    Return = 0x0F,      // return
    Call = 0x10,        // call funcidx
    CallIndirect = 0x11,// call_indirect tableidx typeidx

    /*引用指令 */
    RefNull = 0xD0,  // ref.null reftype
    RefIsNull = 0xD1,// ref.is_null
    RefFunc = 0xD2,  // ref.func funcidx

    /*参数指令 */
    Drop = 0x1A,      // drop
    Select = 0x1B,    // select
    SelectType = 0x1C,// select t*:vec(valtype)

    //--------------------------------------

    /*变量指令 */
    LocalGet = 0x20, // local.get x
    LocalSet = 0x21, // local.set x
    LocalTee = 0x22, // local.tee x
    GlobalGet = 0x23,// global.get x
    GlobalSet = 0x24,// global.set x

    /* 表指令*/
    TableGet = 0x25,// table.get x
    TableSet = 0x26,// table.set x

    /* 内存指令 */
    // m:memarg {align a:u32, offset o:u32}
    I32Load = 0x28,   // i32.load m
    I64Load = 0x29,   // i64.load m
    F32Load = 0x2A,   // f32.load m
    F64Load = 0x2B,   // f64.load m
    I32Load8S = 0x2C, // i32.load8_s m
    I32Load8U = 0x2D, // i32.load8_u m
    I32Load16S = 0x2E,// i32.load16_s m
    I32Load16U = 0x2F,// i32.load16_u m
    I64Load8S = 0x30, // i64.load8_s m
    I64Load8U = 0x31, // i64.load8_u m
    I64Load16S = 0x32,// i64.load16_s m
    I64Load16U = 0x33,// i64.load16_u m
    I64Load32S = 0x34,// i64.load32_s m
    I64Load32U = 0x35,// i64.load32_u m
    I32Store = 0x36,  // i32.store m
    I64Store = 0x37,  // i64.store m
    F32Store = 0x38,  // f32.store m
    F64Store = 0x39,  // f64.store m
    I32Store8 = 0x3A, // i32.store8 m
    I32Store16 = 0x3B,// i32.store16 m
    I64Store8 = 0x3C, // i64.store8 m
    I64Store16 = 0x3D,// i64.store16 m
    I64Store32 = 0x3E,// i64.store32 m
    MemorySize = 0x3F,// memory.size 0x00
    MemoryGrow = 0x40,// memory.grow 0x00

    /* 数值指令 */
    I32Const = 0x41,// i32.const n:i32
    I64Const = 0x42,// i64.const n:i64
    F32Const = 0x43,// f32.const z:f32
    F64Const = 0x44,// f64.const z:f64

    I32Eqz = 0x45,// i32.eqz
    I32Eq = 0x46, // i32.eq
    I32Ne = 0x47, // i32.ne
    I32LtS = 0x48,// i32.lt_s
    I32LtU = 0x49,// i32.lt_u
    I32GtS = 0x4A,// i32.gt_s
    I32GtU = 0x4B,// i32.gt_u
    I32LeS = 0x4C,// i32.le_s
    I32LeU = 0x4D,// i32.le_u
    I32GeS = 0x4E,// i32.ge_s
    I32GeU = 0x4F,// i32.ge_u

    I64Eqz = 0x50,// i64.eqz
    I64Eq = 0x51, // i64.eq
    I64Ne = 0x52, // i64.ne
    I64LtS = 0x53,// i64.lt_s
    I64LtU = 0x54,// i64.lt_u
    I64GtS = 0x55,// i64.gt_s
    I64GtU = 0x56,// i64.gt_u
    I64LeS = 0x57,// i64.le_s
    I64LeU = 0x58,// i64.le_u
    I64GeS = 0x59,// i64.ge_s
    I64GeU = 0x5A,// i64.ge_u

    F32Eq = 0x5B,// f32.eq
    F32Ne = 0x5C,// f32.ne
    F32Lt = 0x5D,// f32.lt
    F32Gt = 0x5E,// f32.gt
    F32Le = 0x5F,// f32.le
    F32Ge = 0x60,// f32.ge

    F64Eq = 0x61,// f64.eq
    F64Ne = 0x62,// f64.ne
    F64Lt = 0x63,// f64.lt
    F64Gt = 0x64,// f64.gt
    F64Le = 0x65,// f64.le
    F64Ge = 0x66,// f64.ge

    I32Clz = 0x67,   // i32.clz
    I32Ctz = 0x68,   // i32.ctz
    I32PopCnt = 0x69,// i32.popcnt
    I32Add = 0x6A,   // i32.add
    I32Sub = 0x6B,   // i32.sub
    I32Mul = 0x6C,   // i32.mul
    I32DivS = 0x6D,  // i32.div_s
    I32DivU = 0x6E,  // i32.div_u
    I32RemS = 0x6F,  // i32.rem_s
    I32RemU = 0x70,  // i32.rem_u
    I32And = 0x71,   // i32.and
    I32Or = 0x72,    // i32.or
    I32Xor = 0x73,   // i32.xor
    I32Shl = 0x74,   // i32.shl
    I32ShrS = 0x75,  // i32.shr_s
    I32ShrU = 0x76,  // i32.shr_u
    I32Rotl = 0x77,  // i32.rotl
    I32Rotr = 0x78,  // i32.rotr

    I64Clz = 0x79,   // i64.clz
    I64Ctz = 0x7A,   // i64.ctz
    I64PopCnt = 0x7B,// i64.popcnt
    I64Add = 0x7C,   // i64.add
    I64Sub = 0x7D,   // i64.sub
    I64Mul = 0x7E,   // i64.mul
    I64DivS = 0x7F,  // i64.div_s
    I64DivU = 0x80,  // i64.div_u
    I64RemS = 0x81,  // i64.rem_s
    I64RemU = 0x82,  // i64.rem_u
    I64And = 0x83,   // i64.and
    I64Or = 0x84,    // i64.or
    I64Xor = 0x85,   // i64.xor
    I64Shl = 0x86,   // i64.shl
    I64ShrS = 0x87,  // i64.shr_s
    I64ShrU = 0x88,  // i64.shr_u
    I64Rotl = 0x89,  // i64.rotl
    I64Rotr = 0x8A,  // i64.rotr

    F32Abs = 0x8B,     // f32.abs
    F32Neg = 0x8C,     // f32.neg
    F32Ceil = 0x8D,    // f32.ceil
    F32Floor = 0x8E,   // f32.floor
    F32Trunc = 0x8F,   // f32.trunc
    F32Nearest = 0x90, // f32.nearest
    F32Sqrt = 0x91,    // f32.sqrt
    F32Add = 0x92,     // f32.add
    F32Sub = 0x93,     // f32.sub
    F32Mul = 0x94,     // f32.mul
    F32Div = 0x95,     // f32.div
    F32Min = 0x96,     // f32.min
    F32Max = 0x97,     // f32.max
    F32CopySign = 0x98,// f32.copysign

    F64Abs = 0x99,     // f64.abs
    F64Neg = 0x9A,     // f64.neg
    F64Ceil = 0x9B,    // f64.ceil
    F64Floor = 0x9C,   // f64.floor
    F64Trunc = 0x9D,   // f64.trunc
    F64Nearest = 0x9E, // f64.nearest
    F64Sqrt = 0x9F,    // f64.sqrt
    F64Add = 0xA0,     // f64.add
    F64Sub = 0xA1,     // f64.sub
    F64Mul = 0xA2,     // f64.mul
    F64Div = 0xA3,     // f64.div
    F64Min = 0xA4,     // f64.min
    F64Max = 0xA5,     // f64.max
    F64CopySign = 0xA6,// f64.copysign

    I32WrapI64 = 0xA7,       // i32.wrap_i64
    I32TruncF32S = 0xA8,     // i32.trunc_f32_s
    I32TruncF32U = 0xA9,     // i32.trunc_f32_u
    I32TruncF64S = 0xAA,     // i32.trunc_f64_s
    I32TruncF64U = 0xAB,     // i32.trunc_f64_u
    I64ExtendI32S = 0xAC,    // i64.extend_i32_s
    I64ExtendI32U = 0xAD,    // i64.extend_i32_u
    I64TruncF32S = 0xAE,     // i64.trunc_f32_s
    I64TruncF32U = 0xAF,     // i64.trunc_f32_u
    I64TruncF64S = 0xB0,     // i64.trunc_f64_s
    I64TruncF64U = 0xB1,     // i64.trunc_f64_u
    F32ConvertI32S = 0xB2,   // f32.convert_i32_s
    F32ConvertI32U = 0xB3,   // f32.convert_i32_u
    F32ConvertI64S = 0xB4,   // f32.convert_i64_s
    F32ConvertI64U = 0xB5,   // f32.convert_i64_u
    F32DemoteF64 = 0xB6,     // f32.demote_f64
    F64ConvertI32S = 0xB7,   // f64.convert_i32_s
    F64ConvertI32U = 0xB8,   // f64.convert_i32_u
    F64ConvertI64S = 0xB9,   // f64.convert_i64_s
    F64ConvertI64U = 0xBA,   // f64.convert_i64_u
    F64PromoteF32 = 0xBB,    // f64.promote_f32
    I32ReinterpretF32 = 0xBC,// i32.reinterpret_f32
    I64ReinterpretF64 = 0xBD,// i64.reinterpret_f64
    F32ReinterpretI32 = 0xBE,// f32.reinterpret_i32
    F64ReinterpretI64 = 0xBF,// f64.reinterpret_i64

    I32Extend8S = 0xC0, // i32.extend8_s
    I32Extend16S = 0xC1,// i32.extend16_s
    I64Extend8S = 0xC2, // i64.extend8_s
    I64Extend16S = 0xC3,// i64.extend16_s
    I64Extend32S = 0xC4,// i64.extend32_s

    CompInstrFC_ = 0xFC,
    CompInstrFD_ = 0xFD
} Instr;

typedef enum {
    /*数值指令*/
    I32TruncSatF32S = 0,// 0xFC 0:u32                      → i32.trunc_sat_f32_s
    I32TruncSatF32U = 1,// 0xFC 1:u32                      → i32.trunc_sat_f32_u
    I32TruncSatF64S = 2,// 0xFC 2:u32                      → i32.trunc_sat_f64_s
    I32TruncSatF64U = 3,// 0xFC 3:u32                      → i32.trunc_sat_f64_u
    I64TruncSatF32S = 4,// 0xFC 4:u32                      → i64.trunc_sat_f32_s
    I64TruncSatF32U = 5,// 0xFC 5:u32                      → i64.trunc_sat_f32_u
    I64TruncSatF64S = 6,// 0xFC 6:u32                      → i64.trunc_sat_f64_s
    I64TruncSatF64U = 7,// 0xFC 7:u32                      → i64.trunc_sat_f64_u
    /*内存指令*/
    MemoryInit = 8, // 0xFC 8:u32 x dataidx 0x00           → memory.init x
    DataDrop = 9,   // 0xFC 9:u32 x:dataidx                → data.drop x
    MemoryCopy = 10,// 0xFC 10:u32 0x00 0x00               → memory.copy
    MemoryFill = 11,// 0xFC 11:u32 0x00                    → memory.fill
    /*表指令*/
    TableInit = 12,// 0xFC 12:u32 y:elemidx x:tableidx     → table.init x y
    ElemDrop = 13, // 0xFC 13:u32 x:elemidx                → elem.drop x
    TableCopy = 14,// 0xFC 14:u32 x:tableidx y:tableidx    → table.copy x y
    TableGrow = 15,// 0xFC 15:u32 x:tableidx               → table.grow x
    TableSize = 16,// 0xFC 16:u32 x:tableidx               → table.size x
    TableFill = 17,// 0xFC 17:u32 x:tableidx               → table.fill x
} CompInstrFC;

typedef enum {
    /*Vector指令*/
    V128Load = 0,        // 0xFD 0:u32 m:memarg                → v128.load m
    V128Load8x8S = 1,    // 0xFD 1:u32 m:memarg                → v128.load8x8_s m
    V128Load8x8U = 2,    // 0xFD 2:u32 m:memarg                → v128.load8x8_u m
    V128Load16x4S = 3,   // 0xFD 3:u32 m:memarg                → v128.load16x4_s m
    V128Load16x4U = 4,   // 0xFD 4:u32 m:memarg                → v128.load16x4_u m
    V128Load32x2S = 5,   // 0xFD 5:u32 m:memarg                → v128.load32x2_s m
    V128Load32x2U = 6,   // 0xFD 6:u32 m:memarg                → v128.load32x2_u m
    V128Load8Splat = 7,  // 0xFD 7:u32 m:memarg                → v128.load8_splat m
    V128Load16Splat = 8, // 0xFD 8:u32 m:memarg                → v128.load16_splat m
    V128Load32Splat = 9, // 0xFD 9:u32 m:memarg                → v128.load32_splat m
    V128Load64Splat = 10,// 0xFD 10:u32 m:memarg               → v128.load64_splat m
    V128Load32Zero = 92, // 0xFD 92:u32 m:memarg               → v128.load32_zero m
    V128Load64Zero = 93, // 0xFD 93:u32 m:memarg               → v128.load64_zero m
    V128Store = 11,      // 0xFD 11:u32 m:memarg               → v128.store m
    V128Load8Lane = 84,  // 0xFD 84:u32 m:memarg lane:u8       → v128.load8_lane m lane
    V128Load16Lane = 85, // 0xFD 85:u32 m:memarg lane:u8       → v128.load16_lane m lane
    V128Load32Lane = 86, // 0xFD 86:u32 m:memarg lane:u8      → v128.load32_lane m lane
    V128Load64Lane = 87, // 0xFD 87:u32 m:memarg lane:u8      → v128.load64_lane m lane
    V128Store8Lane = 88, // 0xFD 88:u32 m:memarg lane:u8      → v128.store8_lane m lane
    V128Store16Lane = 89,// 0xFD 89:u32 m:memarg lane:u8      → v128.store16_lane m lane
    V128Store32Lane = 90,// 0xFD 90:u32 m:memarg lane:u8      → v128.store32_lane m lane
    V128Store64Lane = 91,// 0xFD 91:u32 m:memarg lane:u8      → v128.store64_lane m lane

    V128Const = 12,// 0xFD 12:u32 num:u8*16                   → v128.const num

    I8x16Shuffle = 13,// 0xFD 13:u32 x:u8*16                  → i8x16.shuffle x

    I8x16ExtractLaneS = 21,// 0xFD 21:u32 lane:u8             → i8x16.extract_lane_s lane
    I8x16ExtractLaneU = 22,// 0xFD 22:u32 lane:u8             → i8x16.extract_lane_u lane
    I8x16ReplaceLane = 23, // 0xFD 23:u32 lane:u8             → i8x16.replace_lane lane

    I16x8ExtractLaneS = 24,// 0xFD 24:u32 lane:u8             → i16x8.extract_lane_s lane
    I16x8ExtractLaneU = 25,// 0xFD 25:u32 lane:u8             → i16x8.extract_lane_u lane
    I16x8ReplaceLane = 26, // 0xFD 26:u32 lane:u8             → i16x8.replace_lane lane

    I32x4ExtractLane = 27,// 0xFD 27:u32 lane:u8              → i32x4.extract_lane lane
    I32x4ReplaceLane = 28,// 0xFD 28:u32 lane:u8              → i32x4.replace_lane lane

    I64x2ExtractLane = 29,// 0xFD 29:u32 lane:u8              → i64x2.extract_lane lane
    I64x2ReplaceLane = 30,// 0xFD 30:u32 lane:u8              → i64x2.replace_lane lane

    F32x4ExtractLane = 31,// 0xFD 31:u32 lane:u8              → f32x4.extract_lane lane
    F32x4ReplaceLane = 32,// 0xFD 32:u32 lane:u8              → f32x4.replace_lane lane

    F64x2ExtractLane = 33,// 0xFD 33:u32 lane:u8              → f64x2.extract_lane lane
    F64x2ReplaceLane = 34,// 0xFD 34:u32 lane:u8              → f64x2.replace_lane lane

    I8x16Swizzle = 14,// 0xFD 14:u32                           → i8x16.swizzle
    I8x16Splat = 15,  // 0xFD 15:u32                           → i8x16.splat
    I16x8Splat = 16,  // 0xFD 16:u32                           → i16x8.splat
    I32x4Splat = 17,  // 0xFD 17:u32                           → i32x4.splat
    I64x2Splat = 18,  // 0xFD 18:u32                           → i64x2.splat
    F32x4Splat = 19,  // 0xFD 19:u32                           → f32x4.splat
    F64x2Splat = 20,  // 0xFD 20:u32                           → f64x2.splat

    I8x16Eq = 35, // 0xFD 35:u32                           → i8x16.eq
    I8x16Ne = 36, // 0xFD 36:u32                           → i8x16.ne
    I8x16LtS = 37,// 0xFD 37:u32                           → i8x16.lt_s
    I8x16LtU = 38,// 0xFD 38:u32                           → i8x16.lt_u
    I8x16GtS = 39,// 0xFD 39:u32                           → i8x16.gt_s
    I8x16GtU = 40,// 0xFD 40:u32                           → i8x16.gt_u
    I8x16LeS = 41,// 0xFD 41:u32                           → i8x16.le_s
    I8x16LeU = 42,// 0xFD 42:u32                           → i8x16.le_u
    I8x16GeS = 43,// 0xFD 43:u32                           → i8x16.ge_s
    I8x16GeU = 44,// 0xFD 44:u32                           → i8x16.ge_u

    I16x8Eq = 45, // 0xFD 45:u32                           → i16x8.eq
    I16x8Ne = 46, // 0xFD 46:u32                           → i16x8.ne
    I16x8LtS = 47,// 0xFD 47:u32                           → i16x8.lt_s
    I16x8LtU = 48,// 0xFD 48:u32                           → i16x8.lt_u
    I16x8GtS = 49,// 0xFD 49:u32                           → i16x8.gt_s
    I16x8GtU = 50,// 0xFD 50:u32                           → i16x8.gt_u
    I16x8LeS = 51,// 0xFD 51:u32                           → i16x8.le_s
    I16x8LeU = 52,// 0xFD 52:u32                           → i16x8.le_u
    I16x8GeS = 53,// 0xFD 53:u32                           → i16x8.ge_s
    I16x8GeU = 54,// 0xFD 54:u32                           → i16x8.ge_u

    I32x4Eq = 55, // 0xFD 55:u32                           → i32x4.eq
    I32x4Ne = 56, // 0xFD 56:u32                           → i32x4.ne
    I32x4LtS = 57,// 0xFD 57:u32                           → i32x4.lt_s
    I32x4LtU = 58,// 0xFD 58:u32                           → i32x4.lt_u
    I32x4GtS = 59,// 0xFD 59:u32                           → i32x4.gt_s
    I32x4GtU = 60,// 0xFD 60:u32                           → i32x4.gt_u
    I32x4LeS = 61,// 0xFD 61:u32                           → i32x4.le_s
    I32x4LeU = 62,// 0xFD 62:u32                           → i32x4.le_u
    I32x4GeS = 63,// 0xFD 63:u32                           → i32x4.ge_s
    I32x4GeU = 64,// 0xFD 64:u32                           → i32x4.ge_u

    I64x2Eq = 214, // 0xFD 214:u32                         → i64x2.eq
    I64x2Ne = 215, // 0xFD 215:u32                         → i64x2.ne
    I64x2LtS = 216,// 0xFD 216:u32                         → i64x2.lt_s
    I64x2GtS = 217,// 0xFD 217:u32                         → i64x2.gt_s
    I64x2LeS = 218,// 0xFD 218:u32                         → i64x2.le_s
    I64x2GeS = 219,// 0xFD 219:u32                         → i64x2.ge_s

    F32x4Eq = 65,// 0xFD 65:u32                           → f32x4.eq
    F32x4Ne = 66,// 0xFD 66:u32                           → f32x4.ne
    F32x4Lt = 67,// 0xFD 67:u32                           → f32x4.lt
    F32x4Gt = 68,// 0xFD 68:u32                           → f32x4.gt
    F32x4Le = 69,// 0xFD 69:u32                           → f32x4.le
    F32x4Ge = 70,// 0xFD 70:u32                           → f32x4.ge

    F64x2Eq = 71,// 0xFD 71:u32                           → f64x2.eq
    F64x2Ne = 72,// 0xFD 72:u32                           → f64x2.ne
    F64x2Lt = 73,// 0xFD 73:u32                           → f64x2.lt
    F64x2Gt = 74,// 0xFD 74:u32                           → f64x2.gt
    F64x2Le = 75,// 0xFD 75:u32                           → f64x2.le
    F64x2Ge = 76,// 0xFD 76:u32                           → f64x2.ge

    V128Not = 77,      // 0xFD 77:u32                           → v128.Not
    V128And = 78,      // 0xFD 78:u32                           → v128.And
    V128Or = 79,       // 0xFD 79:u32                           → v128.Or
    V128Xor = 80,      // 0xFD 80:u32                           → v128.Xor
    V128AndNot = 81,   // 0xFD 81:u32                           → v128.AndNot
    V128Bitselect = 82,// 0xFD 82:u32                           → v128.Bit_select
    V128AnyTrue = 83,  // 0xFD 83:u32                           → v128.Any_True

    I8x16Abs = 96,          // 0xFD 96:u32                     → i8x16.abs
    I8x16Neg = 97,          // 0xFD 97:u32                     → i8x16.neg
    I8x16Popcnt = 98,       // 0xFD 98:u32                     → i8x16.popcnt
    I8x16AllTrue = 99,      // 0xFD 99:u32                     → i8x16.all_true
    I8x16Bitmask = 100,     // 0xFD 100:u32                     → i8x16.bitmask
    I8x16NarrowI16x8S = 101,// 0xFD 101:u32                     → i8x16.narrow_i16x8_s
    I8x16NarrowI16x8U = 102,// 0xFD 102:u32                     → i8x16.narrow_i16x8_u
    I8x16Shl = 107,         // 0xFD 107:u32                     → i8x16.shl
    I8x16ShrS = 108,        // 0xFD 108:u32                     → i8x16.shr_s
    I8x16ShrU = 109,        // 0xFD 109:u32                     → i8x16.shr_u
    I8x16Add = 110,         // 0xFD 110:u32                     → i8x16.add
    I8x16AddSatS = 111,     // 0xFD 111:u32                     → i8x16.add_sat_s
    I8x16AddSatU = 112,     // 0xFD 112:u32                     → i8x16.add_sat_u
    I8x16Sub = 113,         // 0xFD 113:u32                     → i8x16.sub
    I8x16SubSatS = 114,     // 0xFD 114:u32                     → i8x16.sub_sat_s
    I8x16SubSatU = 115,     // 0xFD 115:u32                     → i8x16.sub_sat_u
    I8x16MinS = 118,        // 0xFD 118:u32                     → i8x16.min_s
    I8x16MinU = 119,        // 0xFD 119:u32                     → i8x16.min_u
    I8x16MaxS = 120,        // 0xFD 120:u32                     → i8x16.max_s
    I8x16MaxU = 121,        // 0xFD 121:u32                     → i8x16.max_u
    I8x16AvgrU = 123,       // 0xFD 123:u32                     → i8x16.avgr_u

    I16x8ExtAddPairwiseI8x16S = 124,// 0xFD 124:u32                     → i16x8.extadd_pairwise_i8x16_s
    I16x8ExtAddPairwiseI8x16U = 125,// 0xFD 125:u32                     → i16x8.extadd_pairwise_i8x16_u
    I16x8Abs = 128,                 // 0xFD 128:u32                     → i16x8.abs
    I16x8Neg = 129,                 // 0xFD 129:u32                     → i16x8.neg
    I16x8Q15MulrSatS = 130,         // 0xFD 130:u32                     → i16x8.q15mulr_sat_s
    I16x8AllTrue = 131,             // 0xFD 131:u32                     → i16x8.all_true
    I16x8Bitmask = 132,             // 0xFD 132:u32                     → i16x8.bitmask
    I16x8NarrowI32x4S = 133,        // 0xFD 133:u32                     → i16x8.narrow_i32x4_s
    I16x8NarrowI32x4U = 134,        // 0xFD 134:u32                     → i16x8.narrow_i32x4_u
    I16x8ExtendLowI8x16S = 135,     // 0xFD 135:u32                     → i16x8.extend_low_i8x16_s
    I16x8ExtendHighI8x16S = 136,    // 0xFD 136:u32                     → i16x8.extend_high_i8x16_s
    I16x8ExtendLowI8x16U = 137,     // 0xFD 137:u32                     → i16x8.extend_low_i8x16_u
    I16x8ExtendHighI8x16U = 138,    // 0xFD 138:u32                     → i16x8.extend_high_i8x16_u
    I16x8Shl = 139,                 // 0xFD 139:u32                     → i16x8.shl
    I16x8ShrS = 140,                // 0xFD 140:u32                     → i16x8.shr_s
    I16x8ShrU = 141,                // 0xFD 141:u32                     → i16x8.shr_u
    I16x8Add = 142,                 // 0xFD 142:u32                     → i16x8.add
    I16x8AddSatS = 143,             // 0xFD 143:u32                     → i16x8.add_sat_s
    I16x8AddSatU = 144,             // 0xFD 144:u32                     → i16x8.add_sat_u
    I16x8Sub = 145,                 // 0xFD 145:u32                     → i16x8.sub
    I16x8SubSatS = 146,             // 0xFD 146:u32                     → i16x8.sub_sat_s
    I16x8SubSatU = 147,             // 0xFD 147:u32                     → i16x8.sub_sat_u
    I16x8Mul = 149,                 // 0xFD 149:u32                     → i16x8.mul
    I16x8MinS = 150,                // 0xFD 150:u32                     → i16x8.min_s
    I16x8MinU = 151,                // 0xFD 151:u32                     → i16x8.min_u
    I16x8MaxS = 152,                // 0xFD 152:u32                     → i16x8.max_s
    I16x8MaxU = 153,                // 0xFD 153:u32                     → i16x8.max_u
    I16x8AvgrU = 155,               // 0xFD 155:u32                     → i16x8.avgr_u
    I16x8ExtmulLowI8x16S = 156,     // 0xFD 156:u32                     → i16x8.extmul_low_i8x16_s
    I16x8ExtmulHighI8x16S = 157,    // 0xFD 157:u32                     → i16x8.extmul_high_i8x16_s
    I16x8ExtmulLowI8x16U = 158,     // 0xFD 158:u32                     → i16x8.extmul_low_i8x16_u
    I16x8ExtmulHighI8x16U = 159,    // 0xFD 159:u32                     → i16x8.extmul_high_i8x16_u

    I32x4ExtaddPairwiseI16x8S = 126,// 0xFD 126:u32                     → i32x4.extadd_pairwise_i16x8_s
    I32x4ExtaddPairwiseI16x8U = 127,// 0xFD 127:u32                     → i32x4.extadd_pairwise_i16x8_u
    I32x4Abs = 160,                 // 0xFD 160:u32                     → i32x4.abs
    I32x4Neg = 161,                 // 0xFD 161:u32                     → i32x4.neg
    I32x4AllTrue = 163,             // 0xFD 163:u32                     → i32x4.all_true
    I32x4Bitmask = 164,             // 0xFD 164:u32                     → i32x4.bitmask
    I32x4ExtendLowI16x8S = 167,     // 0xFD 167:u32                     → i32x4.extend_low_i16x8_s
    I32x4ExtendHighI16x8S = 168,    // 0xFD 168:u32                     → i32x4.extend_high_i16x8_s
    I32x4ExtendLowI16x8U = 169,     // 0xFD 169:u32                     → i32x4.extend_low_i16x8_u
    I32x4ExtendHighI16x8U = 170,    // 0xFD 170:u32                     → i32x4.extend_high_i16x8_u
    I32x4Shl = 171,                 // 0xFD 171:u32                     → i32x4.shl
    I32x4ShrS = 172,                // 0xFD 172:u32                     → i32x4.shr_s
    I32x4ShrU = 173,                // 0xFD 173:u32                     → i32x4.shr_u
    I32x4Add = 174,                 // 0xFD 174:u32                     → i32x4.add
    I32x4Sub = 177,                 // 0xFD 177:u32                     → i32x4.sub
    I32x4Mul = 181,                 // 0xFD 181:u32                     → i32x4.mul
    I32x4MinS = 182,                // 0xFD 182:u32                     → i32x4.min_s
    I32x4MinU = 183,                // 0xFD 183:u32                     → i32x4.min_u
    I32x4MaxS = 184,                // 0xFD 184:u32                     → i32x4.max_s
    I32x4MaxU = 185,                // 0xFD 185:u32                     → i32x4.max_u
    I32x4DotI16x8S = 186,           // 0xFD 186:u32                     → i32x4.dot_i16x8_s
    I32x4ExtmulLowI16x8S = 188,     // 0xFD 188:u32                     → i32x4.extmul_low_i16x8_s
    I32x4ExtmulHighI16x8S = 189,    // 0xFD 189:u32                     → i32x4.extmul_high_i16x8_s
    I32x4ExtmulLowI16x8U = 190,     // 0xFD 190:u32                     → i32x4.extmul_low_i16x8_u
    I32x4ExtmulHighI16x8U = 191,    // 0xFD 191:u32                     → i32x4.extmul_high_i16x8_u

    F32x4Ceil = 103,   // 0xFD 103:u32                     → f32x4.ceil
    F32x4Floor = 104,  // 0xFD 104:u32                     → f32x4.floor
    F32x4Trunc = 105,  // 0xFD 105:u32                     → f32x4.trunc
    F32x4Nearest = 106,// 0xFD 106:u32                     → f32x4.nearest
    F32x4Abs = 224,    // 0xFD 224:u32                     → f32x4.abs
    F32x4Neg = 225,    // 0xFD 225:u32                     → f32x4.neg
    F32x4Sqrt = 227,   // 0xFD 227:u32                     → f32x4.sqrt
    F32x4Add = 228,    // 0xFD 228:u32                     → f32x4.add
    F32x4Sub = 229,    // 0xFD 229:u32                     → f32x4.sub
    F32x4Mul = 230,    // 0xFD 230:u32                     → f32x4.mul
    F32x4Div = 231,    // 0xFD 231:u32                     → f32x4.div
    F32x4Min = 232,    // 0xFD 232:u32                     → f32x4.min
    F32x4Max = 233,    // 0xFD 233:u32                     → f32x4.max
    F32x4Pmin = 234,   // 0xFD 234:u32                     → f32x4.pmin
    F32x4Pmax = 235,   // 0xFD 235:u32                     → f32x4.pmax

    F64x2Ceil = 116,   // 0xFD 116:u32                     → f64x2.ceil
    F64x2Floor = 117,  // 0xFD 117:u32                     → f64x2.floor
    F64x2Trunc = 118,  // 0xFD 118:u32                     → f64x2.trunc
    F64x2Nearest = 119,// 0xFD 119:u32                     → f64x2.nearest
    F64x2Abs = 236,    // 0xFD 236:u32                     → f64x2.abs
    F64x2Neg = 237,    // 0xFD 237:u32                     → f64x2.neg
    F64x2Sqrt = 239,   // 0xFD 239:u32                     → f64x2.sqrt
    F64x2Add = 240,    // 0xFD 240:u32                     → f64x2.add
    F64x2Sub = 241,    // 0xFD 241:u32                     → f64x2.sub
    F64x2Mul = 242,    // 0xFD 242:u32                     → f64x2.mul
    F64x2Div = 243,    // 0xFD 243:u32                     → f64x2.div
    F64x2Min = 244,    // 0xFD 244:u32                     → f64x2.min
    F64x2Max = 245,    // 0xFD 245:u32                     → f64x2.max
    F64x2Pmin = 246,   // 0xFD 246:u32                     → f64x2.pmin
    F64x2Pmax = 247,   // 0xFD 247:u32                     → f64x2.pmax

    I32x4TruncSatF32x4S = 248,    // 0xFD 248:u32                     → i32x4.trunc_sat_f32x4_s
    I32x4TruncSatF32x4U = 249,    // 0xFD 249:u32                     → i32x4.trunc_sat_f32x4_u
    F32x4ConvertI32x4S = 250,     // 0xFD 250:u32                     → f32x4.convert_i32x4_s
    F32x4ConvertI32x4U = 251,     // 0xFD 251:u32                     → f32x4.convert_i32x4_u
    I32x4TruncSatF64x2SZero = 252,// 0xFD 252:u32                     → i32x4.trunc_sat_f64x2_s_zero
    I32x4TruncSatF64x2UZero = 253,// 0xFD 253:u32                     → i32x4.trunc_sat_f64x2_u_zero
    F64x2ConvertLowI32x4S = 254,  // 0xFD 254:u32                     → f64x2.convert_low_i32x4_s
    F64x2ConvertLowI32x4U = 255,  // 0xFD 255:u32                     → f64x2.convert_low_i32x4_u
    F32x4DemoteF64x2Zero = 94,    // 0xFD 94:u32                      → f32x4.demote_f64x2_zero
    F64x2PromoteLowF32x4 = 95,    // 0xFD 95:u32                      → f64x2.promote_low_f32x4
} CompInstrFD;

#endif
