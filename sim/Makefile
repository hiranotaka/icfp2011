all: sim test

sim: interactive.o libsim.a
	gcc -O2 -Wall -g -o $@ $^

libsim.a: types.o sim.o debug.o
	ar r $@ $^

test: test.o libsim.a
	gcc -O2 -Wall -g -o $@ $^

.c.o:
	gcc -O2 -Wall -g -c $<

clean:
	rm -f sim *.a *.o

sim.o: types.h sim.h
debug.o: types.h sim.h debug.h
interactive.o: sim.h debug.h

.PHONY: clean
