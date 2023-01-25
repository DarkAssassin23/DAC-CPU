TARGET = dac-cpu
LIBS = -lm
CC = gcc
CFLAGS = -g -Wall

INCLUDES = -I headers
OBJDIR = obj

.PHONY: default all clean

default: $(TARGET)
all: default

OBJECTS = $(patsubst src/%.c, $(OBJDIR)/%.o, $(wildcard src/*.c))
HEADERS = $(wildcard headers/*.h)

$(OBJDIR)/%.o: src/%.c $(HEADERS)
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

.PRECIOUS: $(TARGET) $(OBJECTS)

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -g -Wall $(LIBS) -o $@

clean:
	-rm -rf $(OBJDIR) $(TARGET) example_programs/*.o
