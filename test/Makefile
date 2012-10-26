

all :   sequence \
	random

sequence : sequence.o
	gcc -O2 -o sequence sequence.o

random   : random.o
	gcc -O2 -o random random.o

clean    :
	rm -f *.o
	rm -f sequence random
		