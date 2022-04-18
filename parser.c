/* This file is part of OpenU's C project implementation, called assembler
 * Copyright (C) 2020 Arthur Zamarin, Norel Farjun */

#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "parser.h"
#include "opcodes.h"
#include "data_seg.h"
#include "labels_list.h"
#include "instructions_list.h"

struct parser_ctx_t {
    instructions_list insts;
    labels_list_t labels;
    dataseg_t data_seg;
    uint16_t entry_cnt, extern_cnt;
};

struct parser_ctx_t *parser_new(void) {
    struct parser_ctx_t *ctx = malloc(sizeof(struct parser_ctx_t));
    if (!ctx)
        return NULL;

    ctx->entry_cnt = ctx->extern_cnt = 0;
    ctx->insts = instructions_list_new();
    ctx->labels = labels_list_new();
    ctx->data_seg = dataseg_new();
    return ctx;
}

void parser_dealloc(struct parser_ctx_t *ctx) {
    dataseg_dealloc(&ctx->data_seg);
    labels_list_dealloc(&ctx->labels);
    instructions_list_dealloc(&ctx->insts);
    free(ctx);
}

static BOOL check_good_label_name(const char *label) {
    const char *ptr;
    if (!isalpha(*label))
        return FALSE; /* first char must be a letter */

    for (ptr = label; *ptr; ++ptr)
        if (!isalnum(*ptr))
            return FALSE; /* other chars must be a alphanumeric */

    if (find_opcode(label))
        return FALSE; /* label is a opcode keyword */
    if (label[0] == 'r' && label[2] == '\0' && BITS_IS_IN_RANGE(DATA_DST_REG_RANGE, label[1] - '0'))
        return FALSE; /* label is a register keyword */
    return TRUE;
}

/**
 * parse {operand} string and return the calculated operand_t
 * if it is a label, it only sets type to OPERAND_LABEL and caller should find the label_node_object
 * if the input it mismatched, {error_text} is set to the error.
 */
static operand_t parser_parse_operand(const char *operand, const char **error_text) {
    char *endp;
    operand_t res;
    res.type = OPERAND_NONE;
    switch (*operand) {
        case '#':
            res.u.value = (int16_t)strtol(operand + 1, &endp, 10);
            if (endp && *endp)
                *error_text = "Incorrect immediate value - not a number";
            else {
                const int16_t UB = (1 << (DATA_IMMEDIATE_RANGE(BIT_RANGE_END) - DATA_IMMEDIATE_RANGE(BIT_RANGE_START)));
                if (res.u.value >= UB || res.u.value < -UB)
                    *error_text = "Incorrect immediate value - not in range";
                else {
                    res.type = OPERAND_IMMEDIATE;
                    res.u.value = (res.u.value + (UB << 1)) & ((UB << 1) - 1);
                }
            }
            break;
        case '*':
            res.u.value = (int16_t)strtol(operand + 2, &endp, 10);
            if (operand[1] != 'r')
                *error_text = "Incorrect indirect register format";
            else if ((endp && *endp) || !BITS_IS_IN_RANGE(DATA_DST_REG_RANGE, res.u.value))
                *error_text = "Incorrect indirect register value";
            else
                res.type = OPERAND_MEM_REG;
            break;
        case 'r':
            res.u.value = (int16_t)strtol(operand + 1, &endp, 10);
            if ((!endp || !*endp) && BITS_IS_IN_RANGE(DATA_DST_REG_RANGE, res.u.value)) {
                res.type = OPERAND_REG;
                break;
            }
            /* fall through */
        default:
            if (!check_good_label_name(operand))
                *error_text = "Incorrect label name";
            else
                res.type = OPERAND_LABEL;
            break;
    }
    return res;
}

static BOOL parser_parse_instuction(struct parser_ctx_t *ctx, unsigned linenum, const char *str) {
    char opcode_str[MAX_INPUT_LEN + 1], operands[MAX_CNT_OPERAND][MAX_INPUT_LEN + 1], sink[MAX_INPUT_LEN + 1];
    int line_parse_ret, oprn_i;

    opcode_t *opcode;

#define READ_STR "%" XSTR(MAX_INPUT_LEN) "[-#*A-Z0-9a-z]"
    line_parse_ret = sscanf(str, " " READ_STR " " READ_STR " , " READ_STR " %" XSTR(MAX_INPUT_LEN) "s",
                            opcode_str, operands[0], operands[1], sink);
#undef READ_STR
    if (line_parse_ret <= 0) {
        fprintf(ERR_STREAM, "%u: Incorrect instruction line\n", linenum);
        return FALSE;
    }
    --line_parse_ret;
    if (!(opcode = find_opcode(opcode_str))) {
        fprintf(ERR_STREAM, "%u: unknown instruction \'%s\'\n", linenum, opcode_str);
        return FALSE;
    } else if (line_parse_ret < INST_OPCODE_PARAM_COUNT(opcode)) {
        fprintf(ERR_STREAM, "%u: missing operands for instruction \'%s\'\n", linenum, opcode_str);
        return FALSE;
    } else if (line_parse_ret > INST_OPCODE_PARAM_COUNT(opcode)) {
        fprintf(ERR_STREAM, "%u: extra operands for instruction \'%s\'\n", linenum, opcode_str);
        return FALSE;
    } else {
        instruction_t *inst = malloc(sizeof(instruction_t));
        for (oprn_i = 0; oprn_i < line_parse_ret; ++oprn_i) {
            const char *error_text = NULL, *oprn_str = operands[line_parse_ret - 1 - oprn_i];
            inst->operands[oprn_i] = parser_parse_operand(oprn_str, &error_text);
            if (error_text) {
                fprintf(ERR_STREAM, "%u: error with \'%s\': %s\n", linenum, oprn_str, error_text);
                free(inst);
                return FALSE;
            } else if ((inst->operands[oprn_i].type & opcode->operands[oprn_i]) == 0) {
                fprintf(ERR_STREAM, "%u: \'%s\' is illegal as operand number %d for %s\n", linenum, oprn_str, line_parse_ret - oprn_i, opcode_str);
                free(inst);
                return FALSE;
            } else if (inst->operands[oprn_i].type == OPERAND_LABEL)
                inst->operands[oprn_i].u.label_ptr = labels_list_get_label(&ctx->labels, oprn_str);
        }
        for (; oprn_i < MAX_CNT_OPERAND; ++oprn_i)
            inst->operands[oprn_i].type = OPERAND_NONE;

        inst->linenum = linenum;
        inst->command = 0;
        BITS_SET(INST_ARE_RANGE, inst->command, INST_ARE_ABSOLUTE);
        BITS_SET(INST_OPCODE_RANGE, inst->command, opcode->opcode);
        BITS_SET(INST_OPR1_ACCS_RANGE, inst->command, (uint16_t)inst->operands[0].type); /* dst */
        BITS_SET(INST_OPR2_ACCS_RANGE, inst->command, (uint16_t)inst->operands[1].type); /* src */
        instructions_list_add(&ctx->insts, inst);
        return TRUE;
    }
}

static BOOL parser_parse_definition_data(struct parser_ctx_t *ctx, unsigned linenum, const char *str) {
    char data[MAX_INPUT_LEN + 1] = {0}, delim[2], sink[MAX_INPUT_LEN + 1];
    char *endp;
    int line_parse_ret, pos1, pos2;
    int16_t number;
    const int16_t UB = (1 << (DATASEG_VALUE(BIT_RANGE_END) - DATASEG_VALUE(BIT_RANGE_START)));

    while ((line_parse_ret = sscanf(str, " %"XSTR(MAX_INPUT_LEN)"[^, \t\n] %n %1[,] %n %"XSTR(MAX_INPUT_LEN)"s",
                                    data, &pos1, delim, &pos2, sink)) > 0) {
        number = (int16_t)strtol(data, &endp, 10);
        if ((line_parse_ret > 1) ? (delim[0] != ',') : (str[pos1] != '\0')) {
            fprintf(ERR_STREAM, "%u: Missing comma after \'%s\'\n", linenum, data);
            return FALSE;
        } else if (endp && *endp) {
            fprintf(ERR_STREAM, "%u: Incorrect value \'%s\'\n", linenum, data);
            return FALSE;
        } else if (number >= UB || number < -UB) {
            fprintf(ERR_STREAM, "%u: Value \'%s\' not in range\n", linenum, data);
            return FALSE;
        }
        dataseg_append_number(&ctx->data_seg, (number + (UB << 1)) & ((UB << 1) - 1));
        str += pos2;
        if (line_parse_ret == 1)
            return TRUE;
    }
    fprintf(ERR_STREAM, "%u: Incorrect line\n", linenum);
    return FALSE;
}

static BOOL parser_parse_definition_string(struct parser_ctx_t *ctx, unsigned linenum, const char *str) {
    char data[MAX_INPUT_LEN + 1] = {0}, sink[MAX_INPUT_LEN + 1];
    int line_parse_ret;

    line_parse_ret = sscanf(str, " \"%" XSTR(MAX_INPUT_LEN) "[A-Z0-9a-z]\" %" XSTR(MAX_INPUT_LEN) "s",
                            data, sink);
    if (line_parse_ret == 0) {
        fprintf(ERR_STREAM, "%u: missing string\n", linenum);
        return FALSE;
    } else if (line_parse_ret == 2) {
        fprintf(ERR_STREAM, "%u: extra objects with string definition\n", linenum);
        return FALSE;
    }
    dataseg_append_string(&ctx->data_seg, data);
    return TRUE;
}

/**
 * small helper function to parse the label string from {str} and return the label_node_object
 * {type_name} should be the definition type ("entry" or "external") and used for pretty printing
 */
static labels_list_node_t *parser_parse_definition_label(struct parser_ctx_t *ctx, unsigned linenum, const char *str, const char *type_name) {
    char label[MAX_INPUT_LEN + 1] = {0}, sink[MAX_INPUT_LEN + 1];
    int line_parse_ret;

    line_parse_ret = sscanf(str, " %" XSTR(MAX_INPUT_LEN) "[A-Z0-9a-z] %" XSTR(MAX_INPUT_LEN) "s",
                            label, sink);
    if (line_parse_ret == 0)
        fprintf(ERR_STREAM, "%u: missing label\n", linenum);
    else if (line_parse_ret == 2)
        fprintf(ERR_STREAM, "%u: extra objects with %s definition\n", linenum, type_name);
    else if (!check_good_label_name(label))
        fprintf(ERR_STREAM, "%u: Incorrect label name \'%s\'\n", linenum, label);
    else
        return labels_list_get_label(&ctx->labels, label);
    return NULL;
}

static BOOL parser_parse_definition_entry(struct parser_ctx_t *ctx, unsigned linenum, const char *str) {
    labels_list_node_t *node = parser_parse_definition_label(ctx, linenum, str, "entry");
    if (node)
        node->isEntr = TRUE;
    ctx->entry_cnt++;
    return !!node;
}

static BOOL parser_parse_definition_extern(struct parser_ctx_t *ctx, unsigned linenum, const char *str) {
    labels_list_node_t *node = parser_parse_definition_label(ctx, linenum, str, "external");
    if (node) {
        if (node->isSet) {
            fprintf(ERR_STREAM, "%u: label \'%s\' was already set previously\n", linenum, labels_listnode_get_label(node));
            return FALSE;
        }
        node->isExtr = TRUE;
        node->isSet = TRUE;
        node->addr = 0;
    }
    ctx->extern_cnt++;
    return !!node;
}

BOOL parser_parse(struct parser_ctx_t *ctx, FILE* file) {
    unsigned linenum;
    char buffer[MAX_INPUT_LEN + 1] = {0}, label[MAX_INPUT_LEN + 1], definition[MAX_INPUT_LEN + 1];
    char label_delim[2];
    BOOL flag = TRUE;
    for (linenum = 0; fgets(buffer, MAX_INPUT_LEN, file); ++linenum) {
        char *buf_ptr = buffer;
        int line_parse_ret, pos;
        BOOL (*parse_func)(struct parser_ctx_t *, unsigned, const char *) = parser_parse_instuction;

        while (isspace(*buf_ptr))
            buf_ptr++;
        if (*buf_ptr == '\0' || *buf_ptr == ';')
            continue; /* blank line or comment line */

        line_parse_ret = sscanf(buf_ptr, " %" XSTR(MAX_INPUT_LEN) "[A-Z0-9a-z] %1[:] %n", label, label_delim, &pos);
        if (line_parse_ret == 2) { /* found label */
            if (!check_good_label_name(label)) {
                fprintf(ERR_STREAM, "%u: bad label name \'%s\'\n", linenum, label);
                flag = FALSE;
                continue;
            }
            buf_ptr += pos;
        } else
            label[0] = '\0';

        line_parse_ret = sscanf(buf_ptr, " .%" XSTR(MAX_INPUT_LEN) "[A-Z0-9a-z] %n", definition, &pos);
        if (line_parse_ret == 1) { /* found definition */
            if (!strcmp(definition, "data"))
                parse_func = parser_parse_definition_data;
            else if (!strcmp(definition, "string"))
                parse_func = parser_parse_definition_string;
            else if (!strcmp(definition, "entry"))
                parse_func = parser_parse_definition_entry;
            else if (!strcmp(definition, "extern"))
                parse_func = parser_parse_definition_extern;
            else {
                fprintf(ERR_STREAM, "%u: incorrect definition \'%s\'\n", linenum, definition);
                flag = FALSE;
                continue;
            }
            buf_ptr += pos;
        }
        if (*label) {
            if (parse_func == parser_parse_definition_entry || parse_func == parser_parse_definition_extern)
                fprintf(ERR_STREAM, "%u: useless label definition with %s definition\n", linenum, definition);
            else {
                labels_list_node_t *node = labels_list_get_label(&ctx->labels, label);
                if (node->isSet) {
                    fprintf(ERR_STREAM, "%u: label \'%s\' address had been already set\n", linenum, label);
                    flag = FALSE;
                } else {
                    node->isSet = TRUE;
                    node->isDS = parse_func != parser_parse_instuction;
                    node->addr = ((node->isDS) ? ctx->data_seg.size : ctx->insts.size);
                }
            }
        }
        flag &= parse_func(ctx, linenum, buf_ptr);
    }
    if (flag && ctx->insts.size == 0 && ctx->data_seg.size == 0) {
        fprintf(ERR_STREAM, "No declaration in file\n");
        flag = FALSE;
    }
    flag &= labels_list_check_and_fix(&ctx->labels, ctx->insts.size);
    return flag;
}

BOOL parser_output(struct parser_ctx_t *ctx, const char *basename) {
    FILE *object, *entries, *externals = NULL;

    /* open all files */
    char *newName;
    size_t len = strlen(basename);
    if (!(newName = malloc(len + MAX_LEN_EXTENSION + 1)))
        return FALSE;
    memcpy(newName, basename, len);

    strcpy(newName + len, OUTPUT_OBJECT_EXTENSION);
    if (!(object = fopen(newName, "w"))) {
        free(newName);
        return FALSE;
    }
    fprintf(object, "%4d %d\n", ctx->insts.size, ctx->data_seg.size);

    if (ctx->extern_cnt > 0) {
        strcpy(newName + len, OUTPUT_EXTERNALS_EXTENSION);
        if (!(externals = fopen(newName, "w"))) {
            fclose(object);
            free(newName);
            return FALSE;
        }
    }
    instructions_list_output(&ctx->insts, OUTPUT_OBJECT_CODE_START, object, externals);
    if (externals)
        fclose(externals);
    dataseg_output(&ctx->data_seg, OUTPUT_OBJECT_CODE_START + ctx->insts.size, object);
    fclose(object);

    if (ctx->entry_cnt > 0) {
        strcpy(newName + len, OUTPUT_ENTRIES_EXTENSION);
        if (!(entries = fopen(newName, "w"))) {
            free(newName);
            return FALSE;
        }
        labels_list_output_entries(&ctx->labels, entries);
        fclose(entries);
    }

    free(newName);
    return TRUE;
}
