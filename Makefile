# Compiler and flags
CXX = g++
CXXFLAGS = -O3 `pkg-config --cflags opencv4`
LDFLAGS = `pkg-config --libs opencv4`

# Directories
SRC_DIR = src
TEST_DIR = test_programs

# Source files
SRC = $(SRC_DIR)/dwt.cpp \
      $(SRC_DIR)/quantizer.cpp \
      $(SRC_DIR)/tiling.cpp \
      $(SRC_DIR)/code_block.cpp \
      $(SRC_DIR)/mq_coder.cpp \
      $(SRC_DIR)/bp_coder.cpp \
      $(SRC_DIR)/img_proc.cpp \
      $(TEST_DIR)/test_img_proc.cpp # Change this file to test other program

# Output binary
TARGET = $(TEST_DIR)/test_img_proc # Change this to write to other target executable

# Default target
all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) $(SRC) -o $(TARGET) $(LDFLAGS)

clean:
	rm -f $(TARGET)
