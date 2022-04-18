/* This file is part of OpenU's C project implementation, called assembler
 * Copyright (C) 2020 Arthur Zamarin, Norel Farjun */

#ifndef ASM_DATA_SEG_H
#define ASM_DATA_SEG_H

#include <stdio.h>
#include <stdint.h>

/**
 * Holds the data segment structure and content.
 * Uses linked list of fixed size slot array
 */
struct dataseg_node;
typedef struct {
    struct dataseg_node *head;
    struct dataseg_node *tail;
    uint16_t size;
} dataseg_t;

/**
 * create and return a new dataseg_t structure
 */
dataseg_t dataseg_new(void);
/**
 * free and clean the {seg} structure
 */
void dataseg_dealloc(dataseg_t *seg);

/**
 * append {number} as one slot at the end of the {seg} structure
 */
void dataseg_append_number(dataseg_t *seg, uint16_t number);
/**
 * append {str} as zero terminated slot array at the end of the {seg} structure
 */
void dataseg_append_string(dataseg_t *seg, const char *str);
/**
 * output the {seg} structure into {object_file}, while the addressing starts with {start_addr}
 */
void dataseg_output(dataseg_t *seg, unsigned start_addr, FILE *object_file);

#endif
