CC := gcc
INCLUDE_DIR = -I include/

OBJECTS = src/clkgen.o \
		  src/timer_gen.o

TESTOBJ = src/test.o

LIBOBJ := libclkgen.a

LDFLAGS := -L./
CFLAGS := -Wall -std=gnu11 -O2
LIBS := -lclkgen -lpthread

all : $(LIBOBJ)

test: $(TESTOBJ) $(LIBOBJ)
	$(CC) $(TESTOBJ) $(LDFLAGS) -o test $(LIBS)
	./test

$(LIBOBJ): $(OBJECTS)
	ar -crus $(LIBOBJ) $(OBJECTS)

%.o : %.c
	$(CC) -c $(CFLAGS) $(INCLUDE_DIR) -o $@ -c $<

.PHONY: clean

clean:
	rm -rf $(OBJECTS) $(TESTOBJ) test $(LIBOBJ)

