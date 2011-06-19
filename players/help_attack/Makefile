run: main.o sim_util.o util.o ../../sim/libsim.a
	g++ -O3 -Wall -g -o $@ $^

revive: revive_main.o sim_util.o util.o ../../sim/libsim.a
	g++ -O3 -Wall -g -o $@ $^

../../sim/libsim.a:
	make -C ../../sim libsim.a

.cc.o:
	g++ -O3 -Wall -g -c $<

clean:
	make -C ../../sim clean
	rm -f run *.o

.PHONY: clean ../../sim/libsim.a
