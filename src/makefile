FB=1
#SVGA=1
CC=g++
CXX=g++

FT2_CFLAGS=`freetype-config --cflags`
FT2_LIBS=`freetype-config --libs`

CFLAGS=-DNEW_CHEWING -g -Iinclude -I../include -I../libchewing/include/ -fpermissive -I/usr/include/chewing -I/usr/include/x86_64-linux-gnu/chewing/ -Wall -W ${FT2_CFLAGS}
LDFLAGS = -lncurses -lutil -L/usr/local/lib -lchewing $(FT2_LIBS)

ifdef FB
CFLAGS += -DLINUXFB 
endif

ifdef SVGA
CFLAGS += -DVGALIB 
LDFLAGS += -lvgagl -lvga
endif

OBJS = c_routine.o  error.o  hbffont.o  hzinput.o  jmcce.o console.o  draw.o       fb.o     hist.o     hztty.o    newimp.o new_chewing.o convertencode.o ft2.o

ifdef SVGA
jmcce: $(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)
endif

ifdef FB
jmcce.fb: $(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)
endif

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
ft2.o: ft2.cpp
	$(CXX) $(CFLAGS) -c $<
clean:
	rm -rf *.o
