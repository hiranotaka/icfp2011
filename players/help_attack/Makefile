run: ../../sim/sim.o ../../sim/debug.o ltg_help_attack.o sim_util.o
	g++ -O3 -Wall -g -o $@ $^

sim.o:
	make ../../sim

debug.o:
	make ../../sim

.cc.o:
	g++ -O3 -Wall -g -c $<

clean:
	rm -f run *.o

.PHONY: clean
