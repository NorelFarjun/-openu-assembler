#!/bin/bash

# This file is part of OpenU's C project implementation, called assembler
# Copyright (C) 2020 Arthur Zamarin */

# You shouldn't cal this script by itself, but by calling `make tests`

# $1 - executable file
# $2 - basedir
_SKIP_FIRST_LINES_CNT=3 # remove (cnt-1) lines from start
_SKIP_LAST_LINES_CNT=1  # remove cnt     lines from end

_diff_unsorted_columns_file() {
    # $1 - first file
    # $2 - second file
    diff -q <(awk '$1=$1' < "$1" | sort -n -k2,2) <(awk '$1=$1' < "$2" | sort -n -k2,2)
}

_diff_sorted_columns_file() {
    # $1 - first file
    # $2 - second file
    diff -q <(awk '$1=$1' < "$1") <(awk '$1=$1' < "$2")
}

_test_case() {
    # $1 - executable file
    # $2 - basedir
    # $3 - testcase name
    local _basename
    _basename="${2}/${testcase}/${testcase}"
    [[ -f "${_basename}.as" ]] || return # no as file

    if [[ -f "${_basename}.error.expected" ]]; then
        if "$1" "${_basename}" | tail -n +${_SKIP_FIRST_LINES_CNT}  | head -n -${_SKIP_LAST_LINES_CNT} | diff -q - "${_basename}.error.expected"; then
            echo "[OK] ${testcase}: match with errors file"
        else
            echo "[FAIL] ${testcase}: mismatch with errors file"
        fi
    else
        "$1" "${_basename}" >&/dev/null || { echo "${testcase}: exited with error"; return; }
    fi

    for ext in ob ent ext; do
        if [[ -f "${_basename}.${ext}.expected" ]]; then
            if [[ -f "${_basename}.${ext}" ]]; then
                if _diff_sorted_columns_file "${_basename}.${ext}" "${_basename}.${ext}.expected"; then
                    echo "[OK] ${testcase}: match with ${ext} file"
                else
                    echo "[FAIL] ${testcase}: mismatch with ${ext} file"
                fi
            else
                echo "[FAIL] ${testcase}: ${ext} file hadn't been generated"
            fi
        elif [[ -f "${_basename}.${ext}" ]]; then
            echo "[FAIL] ${testcase}: ${ext} file generated when it shouldn't"
        fi
    done
}

find "$2" \( -name "*.ob" -o -name "*.ent" -o -name "*.ext" \) -delete # clean old generated files
for testcase in $(ls "$2"); do
    [[ -f "${2}/${testcase}" ]] && continue
    _test_case "$1" "$2" "$testcase"
done
