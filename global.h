/* This file is part of OpenU's C project implementation, called assembler
 * Copyright (C) 2020 Arthur Zamarin, Norel Farjun */

#ifndef ASM_GLOBAL_H
#define ASM_GLOBAL_H

typedef enum {FALSE=0, TRUE} BOOL;

#define ARR_SIZE(arr) (sizeof(arr) / sizeof(*(arr)))

#define XSTR(s) STRIFY(s)
#define STRIFY(s) #s

/* Start of portable bitfield dragons */
#define BIT_RANGE_START(start, end) (start)
#define BIT_RANGE_END(start,   end) (end)

#define BITS_SET_MASK_IMPL(start, end) (~((1U << ((end) + 1U)) - 1U) | ((1U << (start)) - 1U))
#define BITS_SET_MASK(RANGE_MACRO) BITS_SET_MASK_IMPL(RANGE_MACRO(BIT_RANGE_START), RANGE_MACRO(BIT_RANGE_END))
#define BITS_GET_IMPL(start, end, var) (((var) >> (start)) & ((1U << ((end) - (start) + 1U)) - 1U))

/**
 * To use those bitfield set/get first define a MACRO for the bits range in the field:
 *     #define <name>_RANGE(F) F(<first bit>, <last bit>)
 * Note that first bit and last bit are inclusive, when count starts from LSB and zero.
 * For example opcode range is:
 *     #define INST_OPCODE_RANGE(F) F(11, 14)
 * And usage is:
 *     uint16_t inst = ....
 *     opcode = BITS_GET(INST_OPCODE_RANGE, inst)
 *     BITS_GET(INST_OPCODE_RANGE, inst, 14)
 * It is our responsibility to check value range in set and the correctness of RANGE_MACRO
 */
#define BITS_GET(RANGE_MACRO, var) BITS_GET_IMPL(RANGE_MACRO(BIT_RANGE_START), RANGE_MACRO(BIT_RANGE_END), var)
#define BITS_SET(RANGE_MACRO, var, value) (var) = (((var) & (BITS_SET_MASK(RANGE_MACRO))) | ((value) << RANGE_MACRO(BIT_RANGE_START)))
#define BITS_IS_IN_RANGE(RANGE_MACRO, value) ((value) >= 0 && (value) < (1 << (RANGE_MACRO(BIT_RANGE_END) - RANGE_MACRO(BIT_RANGE_START) + 1)))
/* End of bitfield dragons, we have won */


/** Definitions of output formats for the various files */
#define OBJECT_FILE_OUTPUT_FORMAT "%04u %05o\n"
#define ENTRIES_FILE_OUTPUT_FORMAT "%s %04u\n"
#define EXTERNALS_FILE_OUTPUT_FORMAT "%s %04u\n"

/** destination stream for all errors */
#define ERR_STREAM stdout

#endif
