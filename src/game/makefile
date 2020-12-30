CC = gcc
CFLAGS = -DGAMEDLL -Wall -fno-strict-aliasing -Wimplicit -Wstrict-prototypes \
-Wformat=2 -Wno-format-zero-length -Wformat-security -Wno-format-nonliteral \
-Wstrict-aliasing=2 -Wmissing-format-attribute -Wdisabled-optimization \
-Werror-implicit-function-declaration -MMD -DNDEBUG -O3 -march=i586 \
-fno-omit-frame-pointer -falign-loops=2 -funroll-loops -falign-jumps=2 \
-falign-functions=2 -fstrength-reduce -ffast-math

SOURCES = $(shell ls g_*.c q_*.c ai_*.c bg_animation.c bg_misc.c bg_pmove.c bg_slidemove.c ../botai/*.c)
MODULES = $(SOURCES:.c=.o)

all: gameclean qagame.mp.i386.so

gameclean:
	rm -rf *.o ../botai/*.o

qagame.mp.i386.so: $(MODULES)
	$(CC) -s -shared -ldl -lm -o $@ $?

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f *.o *.so ../botai/*.o
