/* This file is part of OpenU's C project implementation, called assembler
 * Copyright (C) 2020 Arthur Zamarin, Norel Farjun */

#include "opcodes.h"
#include "global.h"

#include <string.h>

/**
  * Fully heap best sorted BTree of all instruction's texts
  * The array is sorted so that for every i:
  *     [2i + 1] < [i] < [2i + 2]
  */
static opcode_t g_all_instructions[] = {
/*  sort IDX  text  opcode    [3..6]        [7..10]       */
    /* 08 */ {"lea",  4, {OPERAND_ALL_RW, OPERAND_LABEL}},
    /* 04 */ {"dec",  8, {OPERAND_ALL_RW, OPERAND_NONE}},
    /* 12 */ {"red", 11, {OPERAND_ALL_RW, OPERAND_NONE}},
    /* 02 */ {"clr",  5, {OPERAND_ALL_RW, OPERAND_NONE}},
    /* 06 */ {"jmp",  9, {OPERAND_ALL_ADDR, OPERAND_NONE}},
    /* 10 */ {"not",  6, {OPERAND_ALL_RW, OPERAND_NONE}},
    /* 14 */ {"stop",15, {OPERAND_NONE, OPERAND_NONE}},
    /* 01 */ {"bne", 10, {OPERAND_ALL_ADDR, OPERAND_NONE}},
    /* 03 */ {"cmp",  1, {OPERAND_ALL_RO, OPERAND_ALL_RO}},
    /* 05 */ {"inc",  7, {OPERAND_ALL_RW, OPERAND_NONE}},
    /* 07 */ {"jsr", 13, {OPERAND_ALL_ADDR, OPERAND_NONE}},
    /* 09 */ {"mov",  0, {OPERAND_ALL_RW, OPERAND_ALL_RO}},
    /* 11 */ {"prn", 12, {OPERAND_ALL_RO, OPERAND_NONE}},
    /* 13 */ {"rts", 14, {OPERAND_NONE, OPERAND_NONE}},
    /* 15 */ {"sub",  3, {OPERAND_ALL_RW, OPERAND_ALL_RO}},
    /* 00 */ {"add",  2, {OPERAND_ALL_RW, OPERAND_ALL_RO}},
};

opcode_t *find_opcode(const char *instruction_text) {
    /* uses sorted BTree traversal */
    unsigned index = 0;
    while (index < ARR_SIZE(g_all_instructions)) {
        const int cmp_res = strcmp(g_all_instructions[index].opcode_text, instruction_text);
        if (cmp_res == 0)
            return g_all_instructions + index;
        else
            index = (2 * index) + (1 + !!(cmp_res < 0));
    }
    return NULL;
}
