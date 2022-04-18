/* This file is part of OpenU's C project implementation, called assembler
 * Copyright (C) 2020 Arthur Zamarin, Norel Farjun */

#ifndef ASM_PARSER_H
#define ASM_PARSER_H

#include <stdio.h>

#include "global.h"

#define MAX_INPUT_LEN 81

struct parser_ctx_t;

/**
 * create a new parser structure
 */
struct parser_ctx_t *parser_new(void);
/**
 * free and close {ctx} context
 */
void parser_dealloc(struct parser_ctx_t *ctx);

/**
 * parse {file} line by line and work on the {ctx} context
 * return true if input file was parsed successfully
 */
BOOL parser_parse(struct parser_ctx_t *ctx, FILE* file);
/**
 * output the {ctx} context using {basename} with all 3 extensions
 * return true if output was successful
 */
BOOL parser_output(struct parser_ctx_t *ctx, const char *basename);

#define INPUT_EXTENSION            ".as"
#define OUTPUT_OBJECT_EXTENSION    ".ob"
#define OUTPUT_ENTRIES_EXTENSION   ".ent"
#define OUTPUT_EXTERNALS_EXTENSION ".ext"
#define MAX_LEN_EXTENSION 4

#define OUTPUT_OBJECT_CODE_START 100

#endif
