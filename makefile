C=gcc
LINK=gcc

C_FLAGS=-ansi -Wall -pedantic
LINK_FLAGS=
EXE_FILE=assembler
TESTS_DIR=tests

all: $(EXE_FILE)

assembler: data_seg.o main.o instructions_list.o labels_list.o opcodes.o parser.o
	$(LINK) $(LINK_FLAGS) -o $(EXE_FILE) data_seg.o main.o instructions_list.o labels_list.o opcodes.o parser.o

data_seg.o: data_seg.c data_seg.h global.h
	$(C) $(C_FLAGS) -c data_seg.c

main.o: data_seg.c global.h parser.h
	$(C) $(C_FLAGS) -c main.c

instructions_list.o: instructions_list.c instructions_list.h global.h opcodes.h labels_list.h
	$(C) $(C_FLAGS) -c instructions_list.c

labels_list.o: labels_list.c labels_list.h global.h parser.h
	$(C) $(C_FLAGS) -c labels_list.c

opcodes.o: opcodes.c opcodes.h global.h
	$(C) $(C_FLAGS) -c opcodes.c

parser.o: parser.c parser.h global.h instructions_list.h labels_list.h data_seg.h opcodes.h
	$(C) $(C_FLAGS) -c parser.c

clean: tests-clean
	rm -f $(EXE_FILE) data_seg.o main.o instructions_list.o labels_list.o opcodes.o parser.o

tests: $(EXE_FILE) $(TESTS_DIR)/run_tests.sh FORCE
	./$(TESTS_DIR)/run_tests.sh ./$(EXE_FILE) $(TESTS_DIR)

FORCE: ;

tests-clean:
	find $(TESTS_DIR) \( -name "*.ob" -o -name "*.ent" -o -name "*.ext" \) -delete
