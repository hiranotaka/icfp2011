sim: sim.o debug.o interactive.o
	gcc -O2 -Wall -g -o $@ $^

.c.o:
	gcc -O2 -Wall -g -c $<

clean:
	rm -f sim *.o

sim.o: types.h sim.h
debug.o: types.h sim.h debug.h
interactive.o: types.h sim.h debug.h

.PHONY: clean
