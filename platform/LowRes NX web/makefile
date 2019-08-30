# Declaration of variables
CC = emcc
CC_FLAGS = -w -s USE_SDL=2 -I ../../core -I ../../core/machine -I ../../core/accessories -I ../../core/datamanager -I ../../core/interpreter -I ../../core/libraries -I ../../core/overlay
CC_FLAGS_MAIN = -s USE_SDL=2

# File names
EXEC = output/LowResNX016.js
SOURCES = $(wildcard ../../core/*.c) $(wildcard ../../core/*/*.c) $(wildcard ../../sdl/*.c)
OBJECTS = $(SOURCES:.c=.bc)
 
# Main target
$(EXEC): $(OBJECTS)
	@mkdir -p $(@D)
	$(CC) $(CC_FLAGS_MAIN) $(OBJECTS) -o $(EXEC)

# To obtain object files
%.bc: %.c
	$(CC) -c $(CC_FLAGS) $< -o $@

# To remove generated files
clean:
	rm -f $(OBJECTS)