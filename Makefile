# Compiler and flags
CC = gcc
CFLAGS = -g -Iinclude -I../3_DataStructures1/include -I../0_Common/include
LDFLAGS = -lpthread -L../0_Common/build -lcommon

# Source files
SRC = src/threadcalc_main.c src/threadpool.c src/signals.c ../3_DataStructures1/src/queue.c
# Object files directory
OBJ_DIR = build_test
# Object files
OBJ = $(OBJ_DIR)/threadcalc_main.o $(OBJ_DIR)/threadpool.o $(OBJ_DIR)/signals.o $(OBJ_DIR)/queue.o
# Output executable
TARGET = $(OBJ_DIR)/my_program

# Default target
all: $(TARGET)

# Create build_test directory
$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

# Link the executable
$(TARGET): $(OBJ) | $(OBJ_DIR)
	$(CC) $(OBJ) -o $(TARGET) $(LDFLAGS)

# Compile source files into object files
$(OBJ_DIR)/threadcalc_main.o: src/threadcalc_main.c include/threadpool.h ../3_DataStructures1/include/queue.h | $(OBJ_DIR)
	$(CC) -c src/threadcalc_main.c -o $(OBJ_DIR)/threadcalc_main.o $(CFLAGS)

$(OBJ_DIR)/threadpool.o: src/threadpool.c include/threadpool.h ../3_DataStructures1/include/queue.h | $(OBJ_DIR)
	$(CC) -c src/threadpool.c -o $(OBJ_DIR)/threadpool.o $(CFLAGS)

$(OBJ_DIR)/signals.o: src/signals.c include/signals.h | $(OBJ_DIR)
	$(CC) -c src/signals.c -o $(OBJ_DIR)/signals.o $(CFLAGS)

$(OBJ_DIR)/queue.o: ../3_DataStructures1/src/queue.c ../3_DataStructures1/include/queue.h | $(OBJ_DIR)
	$(CC) -c ../3_DataStructures1/src/queue.c -o $(OBJ_DIR)/queue.o $(CFLAGS)

# Clean object files and executable
clean:
	rm -rf $(OBJ_DIR)
