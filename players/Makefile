all: run nothing random inc inc_fast

run: ../sim/sim.o ltg_help_attack.o
	g++ -O3 -Wall -g -o $@ $^

nothing: ltg_do_nothing.o
	g++ -O3 -Wall -g -o $@ $^

random: ltg_random.o
	g++ -O3 -Wall -g -o $@ $^

inc: ltg_inc.o
	g++ -O3 -Wall -g -o $@ $^

inc_fast: ltg_inc_fast.o
	g++ -O3 -Wall -g -o $@ $^


sim.o:
	make ../sim

.cc.o:
	g++ -O3 -Wall -g -c $<

run.o: ltg_help_attack.cc

clean:
	rm -f run *.o

.PHONY: clean
