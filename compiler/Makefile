all: parser compiler

libcompiler.a: parser.o compiler.o ../sim/libsim.a
	ar r $@ $^

parser: parser_cli.o parser.o ../sim/libsim.a
	gcc -O2 -Wall -g -o $@ $^

compiler: compiler_cli.o compiler.o parser.o ../sim/libsim.a
	gcc -O2 -Wall -g -o $@ $^

.c.o:
	gcc -O2 -Wall -g -c $<

../sim/libsim.a:
	make -C ../sim libsim.a

clean:
	make -C ../sim clean
	rm -f run *.o

parser.o: parser.h
parser_cli.o: parser.h
compiler.o: compiler.h
compiler_cli.o: parser.h compiler.h

.PHONY: clean ../sim/libsim.a
