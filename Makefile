CC = gcc
CFLAGS = -std=c99 -Wall -Wshadow -Werror -Wvla -O3 -pedantic -fstack-protector-strong --param ssp-buffer-size=1

# Define source code files required
PROJECT_SOURCE_FILES ?= \
	assembler.o \
	cpu.o \
	graphics.o \
	mem.o \
	tests.o \
	util.o \
	screens.o \
	cJSON.o

# Windows raylib paths
RAYLIB_PATH = C:/raylib/raylib
RAYLIB_RELEASE_PATH ?= $(RAYLIB_PATH)/src
# Linux raylib paths
RAYLIB_LIB_PATH = /usr/local/lib
RAYLIB_INCLUDE_PATH = /usr/local/include

### INCLUDE DIRECTORY ###
INCLUDE_DIR = ./INCLUDES
INCLUDE_PATHS = -I. -I$(INCLUDE_DIR) 

### DEFINE OBJECT FILE NAMES AND PATHS ###
OBJ_DIR = ./OBJ
OBJS = $(patsubst %, $(OBJ_DIR)/%, $(PROJECT_SOURCE_FILES))


### LIBRARY LOCATIONS + NAMES ###
LDFLAGS = -L.
LDLIBS = -lraylib

ifeq ($(OS), Windows_NT)
# Windows libraries
	LDLIBS += -lopengl32 -lgdi32 -lwinmm -static -lpthread 
	LDFLAGS += -L$(RAYLIB_PATH)/src
# Windows include paths
	INCLUDE_PATHS += -I$(RAYLIB_PATH)/src -I$(RAYLIB_PATH)/src/external
# Windows executable name
	EXEC_NAME = win-CTRL_6502
	
else
# Linux libraries
	LDFLAGS += -L$(RAYLIB_RELEASE_PATH)
	LDLIBS += -lGL -lm -lpthread -ldl -lrt -lX11
# Linux include paths
	INCLUDE_PATHS += -I$(RAYLIB_INCLUDE_PATH)
# Linux executable name
	EXEC_NAME = linux-CTRL_6502
endif

all: $(OBJ_DIR)/assembler.o $(OBJ_DIR)/cpu.o $(OBJ_DIR)/graphics.o $(OBJ_DIR)/mem.o $(OBJ_DIR)/tests.o $(OBJ_DIR)/cJSON.o $(EXEC_NAME)

$(OBJ_DIR)/assembler.o: assembler.c
	$(CC) -c $(INCLUDE_PATHS) -o $@ $< $(CFLAGS) -DPLATFORM_DESKTOP

$(OBJ_DIR)/cpu.o: cpu.c
	$(CC) -c $(INCLUDE_PATHS) -o $@ $< $(CFLAGS) -DPLATFORM_DESKTOP

$(OBJ_DIR)/graphics.o: graphics.c
	$(CC) -c $(INCLUDE_PATHS) -o $@ $< $(CFLAGS) -DPLATFORM_DESKTOP

$(OBJ_DIR)/mem.o: mem.c
	$(CC) -c $(INCLUDE_PATHS) -o $@ $< $(CFLAGS) -DPLATFORM_DESKTOP

$(OBJ_DIR)/tests.o: tests.c
	$(CC) -c $(INCLUDE_PATHS) -o $@ $< $(CFLAGS) -DPLATFORM_DESKTOP

$(OBJ_DIR)/util.o: util.c
	$(CC) -c $(INCLUDE_PATHS) -o $@ $< $(CFLAGS) -DPLATFORM_DESKTOP

$(OBJ_DIR)/cJSON.o: ./INCLUDES/cJSON.c
	$(CC) -c $(INCLUDE_PATHS) -o $@ $< $(CFLAGS) -DPLATFORM_DESKTOP

$(OBJ_DIR)/screens.o: screens.c
	$(CC) -c $(INCLUDE_PATHS) -o $@ $< $(CFLAGS) -DPLATFORM_DESKTOP


#%.o: %.c $(INCLUDE_DIR)/%.c
#	$(CC) $(CFLAGS) $(INCLUDE_PATHS) -c -o $^ $@ -DPLATFORM_DESKTOP


$(EXEC_NAME): $(OBJS)
	$(CC) -o $@ $(OBJS) $(CFLAGS) $(INCLUDE_PATHS) $(LDFLAGS) $(LDLIBS) -DPLATFORM_DESKTOP

.PHONY: clean_windows

clean_windows:
	del *.o /s
	del *.exe

clean_linux:
	rm -f *.o
	rm -f $(OBJ_DIR)/*.o