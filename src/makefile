CFLAGS =-g -Iinclude -I../include -I../extras/chewing/include
CC=gcc

jmcce: chewing.o  c_routine.o  error.o  hbffont.o  hzinput.o  jmcce.o console.o  draw.o       fb.o     hist.o     hztty.o    newimp.o
	$(CC) -o $@ $^ -lvga -lvgagl

%.d: %.c
	set -e; rm -f $@ ; gcc -MM $(CFLAGS) $< > $@.$$$$ ; sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@ ; rm -f $@.$$$$

sources = chewing.c  c_routine.c  error.c  hbffont.c  hzinput.c  jmcce.c console.c  draw.c       fb.c     hist.c     hztty.c    newimp.c
include $(sources:.c=.d)

clean:
	rm -rf *.o
