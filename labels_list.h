/* This file is part of OpenU's C project implementation, called assembler
 * Copyright (C) 2020 Arthur Zamarin, Norel Farjun */

#ifndef ASM_LABELS_LIST_H
#define ASM_LABELS_LIST_H

#include <stdio.h>
#include <stdint.h>

#include "global.h"

/**
 * Holds information about a one label
 * Note that the label string goes tightly after the node itself - use labels_listnode_get_label to the the label string
 * Also acts as a node in the linked list
 */
typedef struct labels_list_node {
    struct labels_list_node *next;
    unsigned addr  :12; /* relative address to the segment */
    unsigned isSet :1;  /* was the node already set */
    unsigned isDS  :1;  /* is in data segment, otherwise code segment */
    unsigned isExtr:1;  /* is external label */
    unsigned isEntr:1;  /* is flagged as entry to be outputted */
    /* here goes tightly the label */
} labels_list_node_t;

/**
 * return the label's string from {node}
 */
#define labels_listnode_get_label(node) (const char *)((const uint8_t *)(node) + sizeof(labels_list_node_t))

typedef labels_list_node_t *labels_list_t;
/**
 * create and return a new labels_list_t structure
 */
labels_list_t labels_list_new(void);
/**
 * free and clean the {list} structure
 */
void labels_list_dealloc(labels_list_t *list);

/**
 * search {list} structure for label matching {label} and return it if found
 *     if not found, create new unset node and return it.
 * Always returns a node, which is promised to not change it address until deallocation
 */
labels_list_node_t *labels_list_get_label(labels_list_t *list, const char *label);

/**
 * check for correct address for every label in {list} structure
 * also fixes the relative segment addressing to absolute addressing using {codeseg_size}
 */
BOOL labels_list_check_and_fix(labels_list_t *list, unsigned codeseg_size);
/**
 * output all labels in {list} structure marked as entry into {entries_file}
 */
void labels_list_output_entries(labels_list_t *list, FILE *entries_file);

#endif
