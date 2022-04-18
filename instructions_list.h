/* This file is part of OpenU's C project implementation, called assembler
 * Copyright (C) 2020 Arthur Zamarin, Norel Farjun */

#ifndef ASM_INSTRUCTIONS_LIST_H
#define ASM_INSTRUCTIONS_LIST_H

#include <stdio.h>

#include "opcodes.h"
#include "labels_list.h"

/** values relevant to the ARE field in every instruction */
enum INST_ARE_VALS {
    INST_ARE_ABSOLUTE = 4,
    INST_ARE_RELETIVE = 2,
    INST_ARE_EXTERNAL = 1
};

/** bitfield ranges inside the command binary */
#define INST_ARE_RANGE(F)       F( 0,  2) /* ARE field */
#define INST_OPR1_ACCS_RANGE(F) F( 3,  6) /* dst - second operand */
#define INST_OPR2_ACCS_RANGE(F) F( 7, 10) /* src - first  operand */
#define INST_OPCODE_RANGE(F)    F(11, 14) /* opcode field */

/**
 * Holds content of instruction's operand
 * Based on the {type} the appropriate value inside the union should be decided:
 *  - if type==OPERAND_NONE  then the operand is empty
 *  - if type==OPERAND_LABEL then use u.label_ptr
 *  - otherwise                   use u.value
 */
typedef struct {
    enum operand_access type;
    union {
        int16_t value;
        labels_list_node_t *label_ptr;
    } u;
} operand_t;

/** bitfield range for the data segment */
#define DATASEG_VALUE(F)        F( 0, 14)

/** bitfield ranges inside the instruction's operand binary */
#define DATA_ARE_RANGE(F)       F( 0,  2)
#define DATA_IMMEDIATE_RANGE(F) F( 3, 14)
#define DATA_LABEL_RANGE(F)     F( 3, 14)
#define DATA_DST_REG_RANGE(F)   F( 3,  5)
#define DATA_SRC_REG_RANGE(F)   F( 6,  8)

/**
 * Holds full information about one instruction and its operands
 * Also acts as a node in the linked list
 */
typedef struct instruction_t {
    struct instruction_t *next;
    operand_t operands[MAX_CNT_OPERAND]; /* the instruction's operands */
    unsigned linenum;                    /* the linenum in code of the instruction */
    uint16_t command;                    /* binary representation of the command itself */
} instruction_t;

/**
 * Holds the code segment structure and content.
 * Uses linked list of instruction_t
 */
typedef struct {
    instruction_t *head;
    instruction_t *tail;
    uint16_t size;
} instructions_list;

/**
 * create and return a new instructions_list structure
 */
instructions_list instructions_list_new(void);
/**
 * free and clean the {list} structure
 */
void instructions_list_dealloc(instructions_list *list);

/**
 * adds the {inst} at the end of the {list}
 * {inst} is malloced pointer that it's ownership is taken by {list}
 */
void instructions_list_add(instructions_list *list, instruction_t *inst);
/**
 * output the {seg} structure into {object_file}, while the addressing starts with {start_addr}
 * for every external label usage, output it into {externals_file}
 */
void instructions_list_output(instructions_list *list, unsigned start_addr, FILE *object_file, FILE *externals_file);

#endif
