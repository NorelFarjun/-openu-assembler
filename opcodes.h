/* This file is part of OpenU's C project implementation, called assembler
 * Copyright (C) 2020 Arthur Zamarin, Norel Farjun */

#ifndef ASM_INSTRUCTIONS_H
#define ASM_INSTRUCTIONS_H

#include <stdint.h>

#define MAX_CNT_OPERAND 2

enum operand_access {
    OPERAND_NONE      = 0,

    OPERAND_IMMEDIATE = 0x1,
    OPERAND_LABEL     = 0x2,
    OPERAND_MEM_REG   = 0x4,
    OPERAND_REG       = 0x8,

    OPERAND_ALL_ADDR  = OPERAND_MEM_REG  | OPERAND_LABEL,
    OPERAND_ALL_RW    = OPERAND_ALL_ADDR | OPERAND_REG,
    OPERAND_ALL_RO    = OPERAND_ALL_RW   | OPERAND_IMMEDIATE,
    OPERAND_ALL_REG   = OPERAND_MEM_REG  | OPERAND_REG
};

/**
 * information about an opcode
 */
typedef struct {
    const char *opcode_text;
    unsigned opcode;
    enum operand_access operands[MAX_CNT_OPERAND];
} opcode_t;

#define INST_OPCODE_PARAM_COUNT(inst) ((!((inst)->operands[0] == OPERAND_NONE)) + (!((inst)->operands[1] == OPERAND_NONE)))

/**
 * returns the instruction info represented by {instruction_text} parameter,
 *  or NULL if not found.
 */
opcode_t *find_opcode(const char *instruction_text);

#endif
