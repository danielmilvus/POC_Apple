TARGET_EXEC ?= POC
CXX = g++
CC = g++
BUILD_DIR ?= ./build
SRC_DIRS ?= ./src
LIBS ?= -lstdc++ -lc -lproc #-lcrypto -lpthread -lcpprest -lstdc++ -lsqlite3 -lusb -lstdc++fs -lprocps -lm
#LDFLAGS ?= -L/opt/homebrew/Cellar/boost/1.76.0/lib -L/opt/homebrew/Cellar/cpprestsdk/2.10.18/lib -L/opt/homebrew/Cellar/openssl@1.1/1.1.1k/lib


SRCS := $(shell find $(SRC_DIRS) -name *.cpp -or -name *.c -or -name *.s)
OBJS := $(SRCS:%=$(BUILD_DIR)/%.o)
DEPS := $(OBJS:.o=.d)

INC_DIRS := $(shell find $(SRC_DIRS) -type d) #/opt/homebrew/Cellar/openssl@1.1/1.1.1k/include /opt/homebrew/Cellar/cpprestsdk/2.10.18/include/ /opt/homebrew/Cellar/boost/1.76.0/include/
INC_FLAGS := $(addprefix -I,$(INC_DIRS))

CPPFLAGS ?= $(INC_FLAGS) -MMD -MP -Wall -ansi -g3 -pedantic -O0 -c -std=c++17
#CPPFLAGS ?= $(INC_FLAGS) -MMD -MP -Wall -ansi -pedantic -O3 -c -std=c++17 -Wno-deprecated

$(BUILD_DIR)/$(TARGET_EXEC): $(OBJS)
	$(CC) $(OBJS) -o $@ $(LDFLAGS) $(LIBS)

# c++ source
$(BUILD_DIR)/%.cpp.o: %.cpp
	$(MKDIR_P) $(dir $@)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@


.PHONY: clean

clean:
	$(RM) -r $(BUILD_DIR)

-include $(DEPS)

MKDIR_P ?= mkdir -p
