TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

QMAKE_CFLAGS += -Wall -pedantic -ansi

QMAKE_CFLAGS += -m32
QMAKE_LFLAGS += -m32

SOURCES += \
        data_seg.c \
        instructions_list.c \
        labels_list.c \
        main.c \
        opcodes.c \
        parser.c

HEADERS += \
    data_seg.h \
    global.h \
    instructions_list.h \
    labels_list.h \
    opcodes.h \
    parser.h

OTHER_FILES += \
    tests/run_tests.sh
