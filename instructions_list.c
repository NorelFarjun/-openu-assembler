/* This file is part of OpenU's C project implementation, called assembler
 * Copyright (C) 2020 Arthur Zamarin, Norel Farjun */

#include "instructions_list.h"

#include <stdlib.h>

instructions_list instructions_list_new() {
    instructions_list list = {NULL, NULL, 0};
    return list;
}

void instructions_list_dealloc(instructions_list *list) {
    instruction_t *iter = list->head, *tmp;
    while (iter) {
        tmp = iter->next;
        free (iter);
        iter = tmp;
    }
}

/**
 * return the needed slot count for the {inst}'s operand (doesn't count the command itself}
 * should be 0, 1 or 2
 * the last line is responsible for the case when both operands are register based
 */
static unsigned instructions_list_operands_size(instruction_t *inst) {
    return !!(inst->operands[0].type != OPERAND_NONE) + !!(inst->operands[1].type != OPERAND_NONE) -
           !!((inst->operands[0].type & OPERAND_ALL_REG) && (inst->operands[1].type & OPERAND_ALL_REG));
}

void instructions_list_add(instructions_list *list, instruction_t *inst) {
    inst->next = NULL;
    if (list->head == NULL)
        list->head = inst;
    else
        list->tail->next = inst;
    list->tail = inst;
    list->size += 1 + instructions_list_operands_size(inst);
}

/**
 * Calculate and return the correct binary representation of the {oprn} operand, based on {is_dst} flag.
 * In case it depends on an external label, output into {externals_file} with {addr} address
 */
static uint16_t operand_get_value(const operand_t *oprn, BOOL is_dst, unsigned addr, FILE *externals_file) {
    uint16_t value = 0;
    switch (oprn->type) {
        case OPERAND_REG:
        case OPERAND_MEM_REG:
            BITS_SET(DATA_ARE_RANGE, value, INST_ARE_ABSOLUTE);
            if (is_dst)
                BITS_SET(DATA_DST_REG_RANGE, value, (uint16_t)oprn->u.value);
            else
                BITS_SET(DATA_SRC_REG_RANGE, value, (uint16_t)oprn->u.value);
            break;
        case OPERAND_IMMEDIATE:
            BITS_SET(DATA_ARE_RANGE, value, INST_ARE_ABSOLUTE);
            BITS_SET(DATA_IMMEDIATE_RANGE, value, (uint16_t)oprn->u.value);
            break;
        case OPERAND_LABEL:
            if (oprn->u.label_ptr->isExtr) {
                BITS_SET(DATA_ARE_RANGE, value, INST_ARE_EXTERNAL);
                fprintf(externals_file, EXTERNALS_FILE_OUTPUT_FORMAT, labels_listnode_get_label(oprn->u.label_ptr), addr);
            } else {
                BITS_SET(DATA_ARE_RANGE, value, INST_ARE_RELETIVE);
                BITS_SET(DATA_LABEL_RANGE, value, (uint16_t)oprn->u.label_ptr->addr);
            }
            break;
        case OPERAND_NONE:
        default:
            break;
    }
    return value;
}

void instructions_list_output(instructions_list *list, unsigned start_addr, FILE *object_file, FILE *externals_file) {
    instruction_t *inst;
    for (inst = list->head; inst; inst = inst->next) {
        fprintf(object_file, OBJECT_FILE_OUTPUT_FORMAT, start_addr++, inst->command);
        if (inst->operands[0].type != OPERAND_NONE) {
            const unsigned size = instructions_list_operands_size(inst);
            uint16_t operand1 = operand_get_value(inst->operands + 0, TRUE , start_addr + size - 1, externals_file);
            if (inst->operands[1].type != OPERAND_NONE) {
                uint16_t operand2 = operand_get_value(inst->operands + 1, FALSE, start_addr, externals_file);
                if (size == 1)
                    operand1 |= operand2;
                else
                    fprintf(object_file, OBJECT_FILE_OUTPUT_FORMAT, start_addr++, operand2);
            }
            fprintf(object_file, OBJECT_FILE_OUTPUT_FORMAT, start_addr++, operand1);
        }
    }
}
