all: parser

parser: parser_cli.o parser.o ../sim/libsim.a
	gcc -O2 -Wall -g -o $@ $^

.c.o:
	gcc -O2 -Wall -g -c $<

../sim/libsim.a:
	make -C ../sim libsim.a

clean:
	make -C ../sim clean
	rm -f run *.o

parser.o: parser.h
parser_test.o: parser.h

.PHONY: clean ../sim/libsim.a
