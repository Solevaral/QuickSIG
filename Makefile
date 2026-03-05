CXX ?= c++
CXXFLAGS := -std=c++17 -O2 -Wall -Wextra -pedantic
TARGET := pkill-smart

SRC := src/main.cpp src/process_manager.cpp

.PHONY: all clean install uninstall

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -o $@ $(SRC)

install: $(TARGET)
	install -m 0755 $(TARGET) /usr/local/bin/$(TARGET)
	install -m 0755 scripts/fzf_gui.sh /usr/local/bin/pkill-smart-gui

uninstall:
	rm -f /usr/local/bin/$(TARGET) /usr/local/bin/pkill-smart-gui

clean:
	rm -f $(TARGET)
