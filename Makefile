SRC_FILES := $(wildcard src/*.c)
SRC_FILES := $(filter-out src/test.c, $(SRC_FILES))
TEST_FILES := $(wildcard src/*.c)
TEST_FILES := $(filter-out src/main.c, $(TEST_FILES))

debug: $(SRC_FILES) $(wildcard *.h)
	gcc -o chack $(SRC_FILES) -g

release: $(SRC_FILES) $(wildcard *.h)
	gcc -o chack $(SRC_FILES) -O3

test: $(TEST_FILES)
	gcc -o $@ $^ -g
