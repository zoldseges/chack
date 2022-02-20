SRC_FILES := $(wildcard *.c)
SRC_FILES := $(filter-out test.c, $(SRC_FILES))
TEST_FILES := $(wildcard *.c)
TEST_FILES := $(filter-out main.c, $(TEST_FILES))

debug: $(SRC_FILES) $(wildcard *.h)
	gcc -o chack $(SRC_FILES) -g

release: $(SRC_FILES) $(wildcard *.h)
	gcc -o chack $(SRC_FILES) -O3

test: $(TEST_FILES)
	gcc -o $@ $^ -g
