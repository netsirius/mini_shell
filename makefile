CC=gcc
CFLAGS=-Wall
LDFLAGS=-lreadline    #solo si se utiliza la librería, si no comentar esta linea

SOURCES=fase1.c fase2.c fase3.c fase4.c  fase5.c  fase6.c  shell.c fase7.c  shell_rl.c 
LIBRARIES=
INCLUDES= fase1.h fase2.h fase3.h fase4.h fase5.h fase6.h fase7.h shell.h shell_rl.h
PROGRAMS=fase1 fase2 fase3 fase4 fase5 fase6  shell fase7 shell_rl
OBJS=$(SOURCES:.c=.o)

all: $(OBJS) $(PROGRAMS)

#$(PROGRAMS): $(LIBRARIES) $(INCLUDES)
#   $(CC) $(LDFLAGS) $(LIBRARIES) $@.o -o $@

fase1: fase1.o
	$(CC) $@.o -o $@  $(LIBRARIES)
fase2: fase2.o
	$(CC) $@.o -o $@ $(LIBRARIES)
fase3: fase3.o
	$(CC) $@.o -o $@  $(LIBRARIES)
fase4: fase4.o
	$(CC) $@.o -o $@  $(LIBRARIES)
fase5: fase5.o
	$(CC) $@.o -o $@  $(LIBRARIES)
fase6: fase6.o
	$(CC) $@.o -o $@ $(LIBRARIES)
fase7: fase7.o
	$(CC) $@.o -o $@ $(LDFLAGS) $(LIBRARIES)
shell: shell.o
	$(CC) $@.o -o $@ $(LIBRARIES)
shell_rl: shell_rl.o
	$(CC) $@.o -o $@ $(LDFLAGS) $(LIBRARIES)

%.o: %.c $(INCLUDES)
	$(CC) $(CFLAGS) -o $@ -c $<

.PHONY: clean

clean:
	rm -rf *.o *~ *.tmp $(PROGRAMS)
