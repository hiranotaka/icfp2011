run: main.o sim_util.o util.o ../../sim/libsim.a ../../compiler/libcompiler.a
	g++ -O3 -Wall -g -o $@ $^

revive: revive_main.o sim_util.o util.o ../../sim/libsim.a
	g++ -O3 -Wall -g -o $@ $^

../../sim/libsim.a:
	make -C ../../sim libsim.a

../../compiler/libcompiler.a:
	make -C ../../compiler libcompiler.a

.cc.o:
	g++ -O3 -Wall -g -c $<

clean:
	make -C ../../sim clean
	rm -f run *.o

.PHONY: clean ../../sim/libsim.a ../../compiler/libcompiler.a
