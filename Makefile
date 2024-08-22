CC := gcc
CFLAGS := -Wall -Wextra -Wpedantic
LIBS := -lSDL2 -lcairo -lm

# Directories
SRCDIR := src
BUILDDIR := build

# Source files
SOURCES := $(wildcard $(SRCDIR)/*.c)
OBJECTS := $(patsubst $(SRCDIR)/%.c,$(BUILDDIR)/%.o,$(SOURCES))
DEPENDS := $(patsubst $(SRCDIR)/%.c,$(BUILDDIR)/%.d,$(SOURCES))

# Output executable
EXECUTABLE := sb

# Build target
build: $(BUILDDIR)/$(EXECUTABLE)

run: $(BUILDDIR)/$(EXECUTABLE)
	./$(BUILDDIR)/$(EXECUTABLE)

$(BUILDDIR)/$(EXECUTABLE): $(OBJECTS)
	$(CC) $(CFLAGS) $^ $(LIBS) -o $@

# Object file compilation rule
$(BUILDDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@

-include $(DEPENDS)

# Create build directory if it doesn't exist
$(shell mkdir -p $(BUILDDIR))

# Clean target
clean:
	rm -rf $(BUILDDIR)/*.{o,d} $(BUILDDIR)/$(EXECUTABLE)
	rmdir $(BUILDDIR)
