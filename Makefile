ARCH ?= x86
ifeq ($(ARCH),x86)
	CC = gcc #arm-linux-gnueabihf-gcc
else
	CC = gcc #arm-linux-gnueabihf-gcc
endif
TARGET = smartSpeaker
BUILD_DIR = build

SRC_DIR = source
SOURCE = $(foreach dir,$(SRC_DIR),$(wildcard $(dir)/*.c))

INC_DIR = include
INCLUDES = $(foreach dir,$(INC_DIR),$(wildcard $(dir)/*.h))

OBJS = $(patsubst %.c,$(BUILD_DIR)/%.o,$(notdir $(SOURCE)))
VPATH = $(SRC_DIR)

CFLAGS = $(patsubst %,-I%,$(INC_DIR))


$(BUILD_DIR)/$(TARGET) : $(OBJS)
	$(CC) $^ -o $@ -lpthread -lcurl -lm
$(BUILD_DIR)/%.o : %.c $(INCLUDES) | create_build
	$(CC) -c $< -o $@ $(CFLAGS)

.PHONY:clean create_build
clean:
	rm -rf $(BUILD_DIR)
create_build:
	mkdir -p $(BUILD_DIR)
