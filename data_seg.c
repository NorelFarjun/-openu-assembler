/* This file is part of OpenU's C project implementation, called assembler
 * Copyright (C) 2020 Arthur Zamarin, Norel Farjun */

#include "data_seg.h"
#include "global.h"

#include <stdlib.h>
#include <string.h>

struct dataseg_node {
    struct dataseg_node *next;
    uint16_t data[128];
};

dataseg_t dataseg_new() {
    dataseg_t seg = {NULL, NULL, 0};
    return seg;
}

void dataseg_dealloc(dataseg_t *seg) {
    struct dataseg_node *iter = seg->head, *tmp;
    while (iter) {
        tmp = iter->next;
        free(iter);
        iter = tmp;
    }
}

/**
 * allocate a new node at the tail of {seg}
 */
static void dataseg_allocate_node(dataseg_t *seg) {
    struct dataseg_node *node = malloc(sizeof(struct dataseg_node));
    if (seg->head == NULL)
        seg->head = node;
    else
        seg->tail->next = node;
    seg->tail = node;
    node->next = NULL;
}

void dataseg_append_number(dataseg_t *seg, uint16_t number) {
    unsigned pos = seg->size % ARR_SIZE(seg->tail->data);
    if (pos == 0)
        dataseg_allocate_node(seg);
    seg->tail->data[pos] = number;
    seg->size += 1;
}

void dataseg_append_string(dataseg_t *seg, const char *str) {
    for (; *str; ++str)
        dataseg_append_number(seg, (uint16_t)(unsigned char)*str);
    dataseg_append_number(seg, 0);
}

void dataseg_output(dataseg_t *seg, unsigned start_addr, FILE *object_file) {
    unsigned i, remaining = seg->size % ARR_SIZE(seg->head->data);
    struct dataseg_node *iter;
    for (iter = seg->head; iter != seg->tail; iter = iter->next)
        for (i = 0; i < ARR_SIZE(iter->data); ++i)
            fprintf(object_file, OBJECT_FILE_OUTPUT_FORMAT, start_addr++, iter->data[i]);
    for (i = 0; i < remaining; ++i)
        fprintf(object_file, OBJECT_FILE_OUTPUT_FORMAT, start_addr++, seg->tail->data[i]);
}
