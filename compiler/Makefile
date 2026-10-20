
CXX = g++
FLEX = flex
BISON = bison

BUILD_DIR = build
TARGET = $(BUILD_DIR)/compiler

BISON_CPP = $(BUILD_DIR)/parser.cpp
BISON_HPP = $(BUILD_DIR)/parser.hpp
LEXER_CPP = $(BUILD_DIR)/lexer.cpp

CXXFLAGS = -std=c++17 -Wall -Wextra -I$(BUILD_DIR) -Iinclude

.PHONY: all clean

all: $(TARGET)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BISON_CPP): src/parser.y | $(BUILD_DIR)
	$(BISON) --defines=$(BISON_HPP) --output=$@ $<

$(LEXER_CPP): src/lexer.l $(BISON_HPP) | $(BUILD_DIR)
	$(FLEX) --outfile=$@ $<

$(TARGET): $(BISON_CPP) $(LEXER_CPP) src/main.cpp src/semantic.cpp include/ast.hpp include/semantic.hpp
	$(CXX) $(CXXFLAGS) $(BISON_CPP) $(LEXER_CPP) src/main.cpp src/semantic.cpp -o $@

clean:
	rm -rf $(BUILD_DIR)
