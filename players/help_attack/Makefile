run: ../../sim/sim.o ../../sim/debug.o main.o sim_util.o util.o
	g++ -O3 -Wall -g -o $@ $^

revive: ../../sim/sim.o ../../sim/debug.o revive_main.o sim_util.o util.o
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
