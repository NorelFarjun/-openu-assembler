/* This file is part of OpenU's C project implementation, called assembler
 * Copyright (C) 2020 Arthur Zamarin, Norel Farjun */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "parser.h"

static FILE *get_assembly_path(const char *basename) {
    char *newName;
    FILE *asm_file;
    /* build asm's file path */ {
        size_t len = strlen(basename);
        if (!(newName = malloc(len + MAX_LEN_EXTENSION + 1)))
            return NULL;
        memcpy(newName, basename, len);
        strcpy(newName + len, INPUT_EXTENSION);
    }
    asm_file = fopen(newName, "r");
    free(newName);
    return asm_file;
}

int main(int argc, char *argv[])
{
    int i;
    struct parser_ctx_t *ctx;
    FILE *asm_file;
    if (argc == 1) {
        fprintf(ERR_STREAM, "no input files given\n");
        return 1;
    }
    for (i = 1; i < argc; i++) {
        if (!(asm_file = get_assembly_path(argv[i]))) {
            fprintf(ERR_STREAM, "unable to open \'%s.as\'\n", argv[i]);
            continue;
        }
        fprintf(ERR_STREAM, "*******************************************\n""file = %s\n", argv[i]);
        ctx = parser_new();
        if (!parser_parse(ctx, asm_file))
            fprintf(ERR_STREAM, "Bad input file - not outputting\n");
        else if (!parser_output(ctx, argv[i]))
            fprintf(ERR_STREAM, "Unable to output\n");
        else
            fprintf(ERR_STREAM, "All done\n");
        parser_dealloc(ctx);
        fclose(asm_file);
        fprintf(ERR_STREAM, "*******************************************\n");
    }
    return 0;
}
