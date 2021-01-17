# Declaration of variables
CC = gcc
CC_FLAGS =  -D __LINUX__ -w -I ../../core -I ../../core/machine -I ../../core/accessories -I ../../core/datamanager -I ../../core/interpreter -I ../../core/libraries -I ../../core/overlay -lSDL2main -lSDL2 -lm
CC_FLAGS_MAIN =  -D __LINUX__ -lSDL2main -lSDL2 -lm `sdl2-config --cflags --static-libs`

# File names
EXEC = output/LowResNX
SOURCES = $(wildcard ../../core/*.c) $(wildcard ../../core/*/*.c) $(wildcard ../../frontend/*.c)
OBJECTS = $(SOURCES:.c=.o)
 
# Main target
$(EXEC): $(OBJECTS)
	@mkdir -p $(@D)
	$(CC) $(OBJECTS) -o $(EXEC) $(CC_FLAGS_MAIN) 

# To obtain object files
%.o: %.c
	$(CC) -c $(CC_FLAGS) $< -o $@

# To remove generated files
clean:
	rm -f $(OBJECTS)
