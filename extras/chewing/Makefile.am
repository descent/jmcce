SUBDIRS=util

chewing_headers = \
	include/chewingio.h \
	include/char.h \
	include/chewingutil.h \
	include/dict.h \
	include/global.h \
	include/hash.h \
	include/userphrase.h \
	include/zuin.h 

chewing_sources = \
	char.c \
	chewingio.c \
	chewingutil.c \
	choice.c \
	dict.c \
	key2pho.c \
	tree.c \
	zuin.c \
	hash.c \
	userphrase.c

chewing_docs = \
	doc/FILES \
	doc/README \
	BUGS \
	CHANGES

DEFS=@DEFS@ -I$(top_srcdir)/extras/chewing/include


word_data = us_freq.dat ch_index.dat
dict_data = dict.dat ph_index.dat fonetree.dat


pkglib_LTLIBRARIES=libchewing.la

libchewing_la_SOURCES = \
	$(chewing_sources) 
#	$(chewing_headers)


libchewing_la_LDFLAGS = -version-info 1:0:0

EXTRA_DIST = \
	$(chewing_headers) \
	$(chewing_sources) \
	$(chewing_docs) \
	phone.cin tsi.src
	
$(word_data): util/sort_word phone.cin
	util/sort_word

$(dict_data): tsi.src util/sort_dic util/maketree
	util/sort_dic tsi.src
	util/maketree
	rm -f phoneid.dic
	

pkgdatadir = $(datadir)/$(PACKAGE)/inputref/chewing
pkgdata_DATA = $(word_data) $(dict_data)

CLEANFILES=$(word_data) $(dict_data)

