# Makefile for quote - cross-platform stock quote tool

# Detect operating system
UNAME_S := $(shell uname -s)

# Compiler
CXX = g++

# Compiler flags
CXXFLAGS = -std=c++17 -Wall -O2

# Output binary name
TARGET = quote

# Source files
SOURCES = quote.cpp
HEADERS = quote.h

# Platform-specific settings
ifeq ($(UNAME_S),Linux)
    LDFLAGS = -lcurl
    PLATFORM = Linux
endif
ifeq ($(UNAME_S),Darwin)
    LDFLAGS = -lcurl
    PLATFORM = macOS
    # Check if curl is installed via Homebrew
    HOMEBREW_PREFIX := $(shell brew --prefix 2>/dev/null)
    ifneq ($(HOMEBREW_PREFIX),)
        CXXFLAGS += -I$(HOMEBREW_PREFIX)/include
        LDFLAGS += -L$(HOMEBREW_PREFIX)/lib
    endif
endif

# Default target
all: $(TARGET)

# Build target
$(TARGET): $(SOURCES) $(HEADERS)
	@echo "Building for $(PLATFORM)..."
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SOURCES) $(LDFLAGS)
	@echo "Build complete: $(TARGET)"

# Install target
install: $(TARGET)
	@echo "Installing $(TARGET)..."
ifeq ($(shell id -u),0)
	# Running as root
	cp $(TARGET) /usr/local/bin/
	chmod +x /usr/local/bin/$(TARGET)
	@echo "Installed to /usr/local/bin/$(TARGET)"
else
	# Running as user
	mkdir -p $(HOME)/.local/bin
	cp $(TARGET) $(HOME)/.local/bin/
	chmod +x $(HOME)/.local/bin/$(TARGET)
	@echo "Installed to $(HOME)/.local/bin/$(TARGET)"
	@echo "Make sure $(HOME)/.local/bin is in your PATH"
endif
	@echo "Installation complete!"

# Uninstall target
uninstall:
	@echo "Uninstalling $(TARGET)..."
	rm -f /usr/local/bin/$(TARGET)
	rm -f $(HOME)/.local/bin/$(TARGET)
	@echo "Uninstalled $(TARGET)"

# Clean target
clean:
	rm -f $(TARGET)
	@echo "Cleaned build artifacts"

# Check dependencies
check-deps:
	@echo "Checking dependencies for $(PLATFORM)..."
ifeq ($(UNAME_S),Linux)
	@command -v g++ >/dev/null 2>&1 || (echo "Error: g++ not found. Install with: sudo apt-get install g++" && exit 1)
	@ldconfig -p | grep -q libcurl || (echo "Error: libcurl not found. Install with: sudo apt-get install libcurl4-openssl-dev" && exit 1)
	@echo "All dependencies satisfied!"
endif
ifeq ($(UNAME_S),Darwin)
	@command -v g++ >/dev/null 2>&1 || (echo "Error: g++ not found. Install Xcode Command Line Tools" && exit 1)
	@command -v curl-config >/dev/null 2>&1 || (echo "Error: curl not found. Install with: brew install curl" && exit 1)
	@echo "All dependencies satisfied!"
endif

# Help target
help:
	@echo "Available targets:"
	@echo "  make           - Build the project"
	@echo "  make install   - Build and install to system"
	@echo "  make uninstall - Remove installed binary"
	@echo "  make clean     - Remove build artifacts"
	@echo "  make check-deps - Check if dependencies are installed"
	@echo "  make help      - Show this help message"

.PHONY: all install uninstall clean check-deps help
