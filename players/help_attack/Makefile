all: run runc

run: main.o sim_util.o util.o ../../sim/libsim.a ../../compiler/libcompiler.a
	g++ -O3 -Wall -g -o $@ $^

runc: mainc.o sim_util.o util.o ../../sim/libsim.a ../../compiler/libcompiler.a
	g++ -O3 -Wall -g -o $@ $^

revive: revive_main.o sim_util.o util.o ../../sim/libsim.a
	g++ -O3 -Wall -g -o $@ $^

../../sim/libsim.a:
	make -C ../../sim libsim.a

../../compiler/libcompiler.a:
	make -C ../../compiler libcompiler.a

mainc.o: main.cc
	g++ -DUSE_COMPILER -O3 -Wall -g -c -o $@ $<

.cc.o:
	g++ -O3 -Wall -g -c $<

clean:
	make -C ../../sim clean
	rm -f run *.o

.PHONY: clean ../../sim/libsim.a ../../compiler/libcompiler.a
