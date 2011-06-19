all: run nothing random inc inc_fast

run: ltg_help_attack.o ../sim/libsim.a
	g++ -O3 -Wall -g -o $@ $^

nothing: ltg_do_nothing.o
	g++ -O3 -Wall -g -o $@ $^

random: ltg_random.o
	g++ -O3 -Wall -g -o $@ $^

inc: ltg_inc.o
	g++ -O3 -Wall -g -o $@ $^

inc_fast: ltg_inc_fast.o
	g++ -O3 -Wall -g -o $@ $^

../sim/libsim.a:
	make -C ../sim libsim.a

.cc.o:
	g++ -O3 -Wall -g -c $<

run.o: ltg_help_attack.cc

clean:
	make -C ../sim clean
	rm -f run *.o

.PHONY: clean ../sim/libsim.a
