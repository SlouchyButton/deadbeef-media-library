# Thanks to Job Vranish (https://spin.atomicobject.com/2016/08/26/makefile-c-projects/)
TARGET_EXEC := ddb_misc_media_library.so

BUILD_DIR := ./buildMake
SRC_DIRS := ./src
INC_DIRS := ./include
SYS_INC_DIRS := /usr/include

# Find all the C and C++ files we want to compile
# Note the single quotes around the * expressions. Make will incorrectly expand these otherwise.
SRCS := $(shell find $(SRC_DIRS) -name '*.cpp' -or -name '*.c' -or -name '*.s')

# String substitution for every C/C++ file.
# As an example, hello.cpp turns into ./build/hello.cpp.o
OBJS := $(SRCS:%=$(BUILD_DIR)/%.o)

# String substitution (suffix version without %).
# As an example, ./build/hello.cpp.o turns into ./build/hello.cpp.d
DEPS := $(OBJS:.o=.d)

# Every folder in ./include will need to be passed to GCC so that it can find header files
INC_FILES := $(shell find $(INC_DIRS) -type d)

# Add a prefix to INC_DIRS. So moduleA would become -ImoduleA. GCC understands this -I flag
INC_FLAGS := $(addprefix -I,$(INC_FILES))

INC_FLAGS := $(INC_FLAGS) $(shell pkg-config --cflags gtkmm-3.0)
INC_FLAGS := $(INC_FLAGS) $(shell pkg-config --cflags taglib)
INC_FLAGS := $(INC_FLAGS) -I/usr/include/deadbeef
LDFLAGS := -Wl,--as-needed -Wl,--no-undefined -shared -fPIC -Wl,--start-group -Wl,-soname,ddb_misc_media_library.so 
LDFLAGS := $(LDFLAGS) -L/usr/lib -lboost_system -lboost_serialization -lboost_filesystem
LDFLAGS := $(LDFLAGS) $(shell pkg-config --libs gtkmm-3.0)
LDFLAGS := $(LDFLAGS) $(shell pkg-config --libs taglib) -Wl,--end-group

#INC_FLAGS := -I/usr/local/include -I./include

# The -MMD and -MP flags together generate Makefiles for us!
# These files will have .d instead of .o as the output.
CPPFLAGS := $(INC_FLAGS) -MMD -MP -Wall -Winvalid-pch -Wnon-virtual-dtor -O0 -g -fPIC

# The final build step.
$(BUILD_DIR)/$(TARGET_EXEC): $(OBJS)
	$(CXX) $(OBJS) -o $@ $(LDFLAGS)

# Build step for C source
$(BUILD_DIR)/%.c.o: %.c
	mkdir -p $(dir $@)
	$(CC) -c $(CPPFLAGS) $(CFLAGS) $< -o $@

# Build step for C++ source
$(BUILD_DIR)/%.cpp.o: %.cpp
	mkdir -p $(dir $@)
	$(CXX) -c $(CPPFLAGS) $(CXXFLAGS) $< -o $@


.PHONY: clean
clean:
	rm -r $(BUILD_DIR)

.PHONY: install
install:
	cp $(BUILD_DIR)/$(TARGET_EXEC) ~/.local/lib/deadbeef

.PHONY: buildinstall
buildinstall:
	make
	make install

.PHONY: buildinstallrun
buildinstallrun:
	make
	make install
	deadbeef

.PHONY: uninstall
uninstall:
	rm ~/.local/lib/deadbeef/$(TARGET_EXEC)

# Include the .d makefiles. The - at the front suppresses the errors of missing
# Makefiles. Initially, all the .d files will be missing, and we don't want those
# errors to show up.
-include $(DEPS)
