CC = gcc
CFLAGS = -Wall -Wextra -g
SRCDIR = src
OBJDIR = obj
BINDIR = bin

# Source files
SRCS = $(SRCDIR)/execute.c $(SRCDIR)/parsing.c
OBJS = $(SRCS:$(SRCDIR)/%.c=$(OBJDIR)/%.o)

# Main target
TARGET = $(BINDIR)/quash

.PHONY: all clean test doc

all: $(TARGET)

# Create necessary directories
$(OBJDIR):
	mkdir -p $(OBJDIR)

$(BINDIR):
	mkdir -p $(BINDIR)

# Compile source files to object files
$(OBJDIR)/%.o: $(SRCDIR)/%.c | $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Link object files to create executable
$(TARGET): $(OBJS) | $(BINDIR)
	$(CC) $(OBJS) -o $@

# Run the shell
test: $(TARGET)
	./$(TARGET)
	
# Clean build files
clean:
	rm -rf $(OBJDIR) $(BINDIR)
	rm -f *~ *.o
