CC := gcc
CFLAGS := -Wall -Wextra
LIBS := -lSDL2 -lcairo -lm

# Directories
SRCDIR := src
BUILDDIR := build

# Source files
SOURCES := $(wildcard $(SRCDIR)/*.c)
OBJECTS := $(patsubst $(SRCDIR)/%.c,$(BUILDDIR)/%.o,$(SOURCES))

# Output executable
EXECUTABLE := sb

# Build target
build: $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(CFLAGS) $^ $(LIBS) -o $(BUILDDIR)/$@

# Object file compilation rule
$(BUILDDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Create build directory if it doesn't exist
$(shell mkdir -p $(BUILDDIR))

# Clean target
clean:
	rm -rf $(BUILDDIR)/*.o $(BUILDDIR)/$(EXECUTABLE)
	rmdir $(BUILDDIR)
