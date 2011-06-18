all: run nothing random

run: ../sim/sim.o ltg_help_attack.o
	g++ -O3 -Wall -g -o $@ $^

nothing: ../sim/sim.o ltg_do_nothing.o
	g++ -O3 -Wall -g -o $@ $^

random: ../sim/sim.o ltg_random.o
	g++ -O3 -Wall -g -o $@ $^

sim.o:
	make ../sim

.cc.o:
	g++ -O3 -Wall -g -c $<

run.o: ltg_help_attack.cc

clean:
	rm -f run *.o

.PHONY: clean
