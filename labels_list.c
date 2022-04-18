/* This file is part of OpenU's C project implementation, called assembler
 * Copyright (C) 2020 Arthur Zamarin, Norel Farjun */

#include <stdlib.h>
#include <string.h>

#include "labels_list.h"
#include "parser.h"

labels_list_t labels_list_new(void) {
    return NULL;
}

void labels_list_dealloc(labels_list_t *list) {
    labels_list_node_t *iter = *list, *tmp;
    while (iter) {
        tmp = iter->next;
        free (iter);
        iter = tmp;
    }
}

/**
 * allocate a new node and put the {label} tightly at the end
 * also default sets needed variables
 */
static labels_list_node_t *labels_list_node_alloc(const char *label) {
    size_t len = strlen(label);
    labels_list_node_t *node = malloc(sizeof(labels_list_node_t) + len + 1);
    if (node) {
        node->next = NULL;
        node->isSet = FALSE;
        node->isExtr = FALSE;
        node->isEntr = FALSE;
        memcpy((char *)node + sizeof(labels_list_node_t), label, len + 1);
    }
    return node;
}

labels_list_node_t *labels_list_get_label(labels_list_t *list, const char *label) {
    labels_list_node_t *iter, *prev = NULL;
    if (!*list)
        return (*list = labels_list_node_alloc(label));
    for (iter = *list; iter; prev = iter, iter = iter->next)
        if (!strcmp(labels_listnode_get_label(iter), label))
            return iter;
    return (prev->next = labels_list_node_alloc(label));
}

BOOL labels_list_check_and_fix(labels_list_t *list, unsigned codeseg_size) {
    labels_list_node_t *iter;
    BOOL flag = TRUE;
    for (iter = *list; iter; iter = iter->next) {
        if (!iter->isSet) {
            flag = FALSE;
            fprintf(ERR_STREAM, "address for label \'%s\' not found in assembly file\n", labels_listnode_get_label(iter));
        } else if (iter->isExtr);
        else if (iter->isDS)
            iter->addr += OUTPUT_OBJECT_CODE_START + codeseg_size;
        else
            iter->addr += OUTPUT_OBJECT_CODE_START;
    }
    return flag;
}

void labels_list_output_entries(labels_list_t *list, FILE *entries_file) {
    labels_list_node_t *iter;
    for (iter = *list; iter; iter = iter->next)
        if (iter->isEntr)
            fprintf(entries_file, ENTRIES_FILE_OUTPUT_FORMAT, labels_listnode_get_label(iter), iter->addr);
}
