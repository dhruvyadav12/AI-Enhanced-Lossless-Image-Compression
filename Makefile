CXX = g++
CXXFLAGS = -O3 -std=c++11 -Wall -Wextra
INCLUDES = -Icpp/include
SRC_DIR = cpp/src

# Binary names based on OS
ifeq ($(OS),Windows_NT)
    TARGET_ENCODER = $(SRC_DIR)/encoder.exe
    TARGET_DECODER = $(SRC_DIR)/decoder.exe
    RM = del /Q
else
    TARGET_ENCODER = $(SRC_DIR)/encoder
    TARGET_DECODER = $(SRC_DIR)/decoder
    RM = rm -f
endif

all: $(TARGET_ENCODER) $(TARGET_DECODER)

$(TARGET_ENCODER): $(SRC_DIR)/encoder.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< -o $@

$(TARGET_DECODER): $(SRC_DIR)/decoder.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< -o $@

clean:
	$(RM) $(SRC_DIR)/encoder $(SRC_DIR)/decoder $(SRC_DIR)/*.exe

run: all
	python app.py

.PHONY: all clean run
