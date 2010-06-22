JMCCE_EXTRAS_SOURCES=chewing.c 

CFLAGS=-D@JMCCE_ARCH@ -DJMCCE_FONTDIR=\"${JMCCE_FONTDIR}\" \
     -DJMCCE_CONFDIR=\"${JMCCE_CONFDIR}\" \
     -DJMCCE_DATADIR=\"${JMCCE_DATADIR}\" \
     -I${JMCCE_EXTRAS_DIR}/chewing/include

AM_INSTALL_PROGRAM_FLAGS=-m 4755 -o root -g root

bin_PROGRAMS = jmcce

jmcce_headers = \
	$(top_srcdir)/include/gvfont.h \
	$(top_srcdir)/include/phone.h \
	$(top_srcdir)/include/newimp.h \
	$(top_srcdir)/include/jmcce.h \
	$(top_srcdir)/include/hztty.h \
	$(top_srcdir)/include/hzinput.h \
	$(top_srcdir)/include/hbffont.h \
	$(top_srcdir)/include/gb2big.h \
	$(top_srcdir)/include/error.h \
	$(top_srcdir)/include/fb.h \
	$(top_srcdir)/include/draw.h \
	$(top_srcdir)/include/c_routine.h \
	$(top_srcdir)/include/big2gb.h \
	$(top_srcdir)/include/asm_routine.h \
	$(top_srcdir)/include/associate.h

jmcce_sources = \
	hztty.c jmcce.c draw.c error.c hbffont.c hzinput.c \
	console.c hist.c asm_routine.S c_routine.c fb.c \
	newimp.c $(JMCCE_EXTRAS_SOURCES) #chewing.c

jmcce_SOURCES = \
	$(jmcce_sources)

jmcce_LDADD = $(JMCCE_EXTRAS_DIR)/chewing/libchewing.la

EXTRA_DIST = $(jmcce_headers)

$(JMCCE_EXTRAS_DIR)/chewing/libchewing.la:
	$(MAKE) -C $(JMCCE_EXTRAS_DIR)/chewing libchewing.la

