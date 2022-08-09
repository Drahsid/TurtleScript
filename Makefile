CC = gcc
LD = ld

BUILD_DIR = build
SRC_DIR = src
INC_DIR = include
TESTS_DIR = tests

SRC_DIRS = $(SRC_DIR)
INCLUDE_DIRS = -I$(INC_DIR)

C_FILES = $(foreach dir,$(SRC_DIRS),$(wildcard $(dir)/*.c))
O_FILES = $(foreach file,$(C_FILES),$(BUILD_DIR)/$(file).o)

TARGET = $(BUILD_DIR)/turtle

CC_FLAGS = -m64 -w -g3 -Og $(INCLUDE_DIRS)

default: dirs $(TARGET)

clean:
	rm -rf $(BUILD_DIR)

dirs:
	$(foreach dir,$(SRC_DIRS),$(shell mkdir -p $(BUILD_DIR)/$(dir)))

$(TARGET):
	$(CC) $(CC_FLAGS) $(C_FILES) -o $@

run:
	cp $(TESTS_DIR)/** $(BUILD_DIR)
	$(TARGET)

.SECONDARY:
.PHONY: clean default dirs
SHELL = /bin/bash -e -o pipefail