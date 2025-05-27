# Makefile for KSH - Simple Shell

CC = gcc
CFLAGS = -Wall -Wextra -std=c2x
TARGET = k2sh
SRCDIR = .
OBJDIR = obj

# Source files
SOURCES = main.c builtins.c parser.c executor.c shell.c
OBJECTS = $(SOURCES:%.c=$(OBJDIR)/%.o)

# Build mode flags
DEBUG_FLAGS = -g -DDEBUG -O0
RELEASE_FLAGS = -O2 -DNDEBUG

# Default target (debug build)
all: debug

# Create object directory
$(OBJDIR):
	mkdir -p $(OBJDIR)

# Debug build
debug: CFLAGS += $(DEBUG_FLAGS)
debug: $(TARGET)

# Release build
release: CFLAGS += $(RELEASE_FLAGS)
release: clean-obj $(TARGET)

# Build the main executable
$(TARGET): $(OBJDIR) $(OBJECTS)
	$(CC) $(OBJECTS) -o $(TARGET)
	@echo "Built $(TARGET) successfully!"

# Compile source files
$(OBJDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean object files only (preserves executable)
clean-obj:
	rm -rf $(OBJDIR)

# Clean build files
clean:
	rm -rf $(OBJDIR) $(TARGET)

# Rebuild everything
rebuild: clean all

# Install (optional)
install: release
	cp $(TARGET) /usr/local/bin/
	@echo "Installed $(TARGET) to /usr/local/bin/"

# Uninstall (optional)
uninstall:
	rm -f /usr/local/bin/$(TARGET)
	@echo "Uninstalled $(TARGET) from /usr/local/bin/"

# Run the shell (debug version)
run: debug
	./$(TARGET)

# Run release version
run-release: release
	./$(TARGET)

# Show build info
info:
	@echo "Build Information:"
	@echo "  Compiler: $(CC)"
	@echo "  Debug flags: $(DEBUG_FLAGS)"
	@echo "  Release flags: $(RELEASE_FLAGS)"
	@echo "  Sources: $(SOURCES)"
	@echo "  Target: $(TARGET)"

# Show help
help:
	@echo "Available targets:"
	@echo "  all         - Build debug version (default)"
	@echo "  debug       - Build with debug symbols and no optimization"
	@echo "  release     - Build optimized version without debug info"
	@echo "  clean       - Remove all build files"
	@echo "  clean-obj   - Remove only object files"
	@echo "  rebuild     - Clean and build debug version"
	@echo "  run         - Build and run debug version"
	@echo "  run-release - Build and run release version"
	@echo "  install     - Install release version to /usr/local/bin"
	@echo "  uninstall   - Remove from /usr/local/bin"
	@echo "  info        - Show build information"
	@echo "  help        - Show this help"

.PHONY: all debug release clean clean-obj rebuild install uninstall run run-release info help