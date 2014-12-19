CC = gcc
CFLAGS = -DCGAMEDLL -Wall -fno-strict-aliasing -Wimplicit -Wstrict-prototypes \
-Wformat=2 -Wno-format-zero-length -Wformat-security -Wno-format-nonliteral \
-Wstrict-aliasing=2 -Wmissing-format-attribute -Wdisabled-optimization \
-Werror-implicit-function-declaration -MMD -DNDEBUG -O3 -march=i586 \
-fno-omit-frame-pointer -falign-loops=2 -funroll-loops -falign-jumps=2 \
-falign-functions=2 -fstrength-reduce -ffast-math

SOURCES = $(shell ls cg_*.c ../game/bg_animation.c ../game/bg_misc.c ../game/bg_pmove.c ../game/bg_slidemove.c ../game/q_math.c ../game/q_shared.c ../ui/ui_shared.c)
MODULES = $(SOURCES:.c=.o)

all: gameclean cgame.mp.i386.so

gameclean:
	rm -rf ../game/*.o ../ui/*.o

cgame.mp.i386.so: $(MODULES)
	$(CC) -shared -ldl -lm -o $@ $?

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f *.o *.so
