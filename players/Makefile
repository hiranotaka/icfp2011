sim:
	cp ../sim/sim.o .
run: sim.o ltg_help_attack.o
	g++ -O3 -Wall -g -o $@ $^
.cc.o:
	g++ -O3 -Wall -g -c $<

run.o: ltg_help_attack.cc

clean:
	rm -f run *.o

.PHONY: clean
