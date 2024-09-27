# -Werror -ansi -pedantic -Wall -Wextra -Wno-unused-parameter
CC = g++  # notice CFLAGS contains -g which will compile everything in debug mode!
CFLAGS = -g --std=c++11 -Wall -Wextra -Wno-unused-parameter
DEPS = object.h
OBJ = object.o 

ifdef OS # windows defines this environment variable
	LDFLAGS = -L. -lSDL2 -lSDL2_image -lSDL2_ttf -static-libgcc -static-libstdc++
else # assume a posix OS
    LDFLAGS = -lSDL2 -lSDL2_image -lSDL2_ttf
endif

%.o: %.cpp $(DEPS)
	$(CC) $(CFLAGS) -c -o $@ $<

asmcad: $(OBJ)
	$(CC) -o $@ $^ $(LDFLAGS)

clean:
	/bin/rm -f $(OBJ)
