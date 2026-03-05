CXX ?= c++
CXXFLAGS := -std=c++17 -O2 -Wall -Wextra -pedantic
TARGET := quicksig

SRC := src/main.cpp src/process_manager.cpp

.PHONY: all clean install uninstall

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -o $@ $(SRC)

install: $(TARGET)
	install -m 0755 $(TARGET) /usr/local/bin/$(TARGET)
	install -m 0755 scripts/fzf_gui.sh /usr/local/bin/quicksig-gui

uninstall:
	rm -f /usr/local/bin/$(TARGET) /usr/local/bin/quicksig-gui

clean:
	rm -f $(TARGET)
