# Program and compilation options
PROG = patch
CC = gcc
OPTIONS_OLD = -Wall -Iinc
OPTIONS = -Wall -Iinc

# List all .c source files and .o object files
SOURCES = $(wildcard src/*.c) # Use wildcard to find all .c files in src
OBJETS = $(SOURCES:src/%.c=bin/%.o) # Map source files to object files in bin

# Main rule - build the entire project
build: create_dir $(PROG)

# Create the directory for object files
create_dir:
	@mkdir -p bin

out_dir:
	@mkdir -p out

# Compile the final program
$(PROG): $(OBJETS)
	@echo "🔨 Building program $(PROG)..."
	$(CC) -o $(PROG) $(OBJETS) $(OPTIONS)
	@echo "✅ Program built successfully!"
	@echo " "

# Rule to compile each source file into an object file
bin/%.o: src/%.c
	@echo "🔄 Ensuring bin directory exists..."
	@mkdir -p bin
	@echo "📝 Compiling $<..."
	$(CC) $(OPTIONS) -c $< -o $@
	@echo "✅ $< compiled successfully!"
	@echo " "

# Run the program
run: $(PROG)
	@echo "🚀 Launching program $(PROG)..."
	./$(PROG)

# Clean compiled files
clean:
	@echo "🧹 Cleaning generated files..."
	rm -f bin/*.o $(PROG)
	rm -f out/*.dot out/*.pdf 2>/dev/null || true
	@echo "✨ Cleanup completed!"
	@echo " "

# Help - show available commands
help:
	@echo "Available commands:"
	@echo "  make build   - build the program"
	@echo "  make run   - run the program"
	@echo "  make clean - remove compiled files"
	@echo "  make help  - show this help"
	@echo "  make patch - build the patched program"


# These are phony targets (names, not files)
.PHONY: build create_dir run clean help