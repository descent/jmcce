#CFLAGS =-DVGALIB -DNEW_CHEWING -g -Iinclude -I../include -I../libchewing/include/ -fpermissive -I/usr/include/chewing

CFLAGS =-DLINUXFB -DNEW_CHEWING -g -Iinclude -I../include -I../libchewing/include/ -fpermissive -I/usr/include/chewing
CC=g++
CXX=g++

#pkg-config --variable=datadir chewing
#pkg-config chewing --cflags --libs

#-I/usr/local/include/chewing  -L/usr/local/lib -lchewing  


OBJS = c_routine.o  error.o  hbffont.o  hzinput.o  jmcce.o console.o  draw.o       fb.o     hist.o     hztty.o    newimp.o new_chewing.o convertencode.o

#jmcce: chewing.o  c_routine.o  error.o  hbffont.o  hzinput.o  jmcce.o console.o  draw.o       fb.o     hist.o     hztty.o    newimp.o asm_routine.o
jmcce.fb:
	make -f makefile.lfb clean
	make -f makefile.lfb

jmcce: 
	make -f makefile.svga clean
	make -f makefile.svga

vgatest: vgatest.o
	$(CC) -o $@ $^ -lvgagl -lvga 

%.d: %.c
	set -e; rm -f $@ ; $(CC) -MM $(CFLAGS) $< > $@.$$$$ ; sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@ ; rm -f $@.$$$$

#%.dpp: %.cpp
#	set -e; rm -f $@ ; $(CC) -MM $(CFLAGS) $< > $@.$$$$ ; sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@ ; rm -f $@.$$$$

asm_routine.o: asm_routine.S
	$(CC) $(CFLAGS) -c $<

sources = c_routine.c  error.c  hbffont.c  hzinput.c  jmcce.c console.c  draw.c       fb.c     hist.c     hztty.c    newimp.c vgatest.c new_chewing.c
include $(sources:.c=.d)
#include $(sources:.cpp=.dpp)

convertencode.o: convertencode.cpp
	$(CXX) -c $<

clean:
	rm -rf *.o *.d jmcce.fb jmcce
