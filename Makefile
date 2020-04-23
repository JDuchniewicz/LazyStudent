# Linux makefile which keeps track of dependencies
# automatically creates BUILD_DIR folder if needed

TARGET_EXEC ?= LazyStudent_genetic
BUILD_DIR ?= ./build
SRC_DIRS ?= ./src
INC_DIRS ?= ./headers

SRCS := $(shell find $(SRC_DIRS) -name *.cpp)
OBJS := $(SRCS:%=$(BUILD_DIR)/%.o)
DEPS := $(OBJS:.o=.d)

INC_FLAGS := $(addprefix -I,$(INC_DIRS))

CXXFLAGS := $(INC_FLAGS) $(CXXFLAGS) -MMD -MP -g -Wall -Wformat -std=c++17 -ggdb3

$(BUILD_DIR)/$(TARGET_EXEC): $(OBJS)
	$(CXX) $(OBJS) -o $@

$(BUILD_DIR)/%.cpp.o: %.cpp
	$(MKDIR_P) $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

.PHONY: clean

clean:
	$(RM) -r $(BUILD_DIR)

-include $(DEPS)

MKDIR_P ?= mkdir -p