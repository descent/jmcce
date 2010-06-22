DEFS=@DEFS@ -I$(top_srcdir)/extras/chewing/include

bin_PROGRAMS = maketree sort_dic sort_word

maketree_SOURCES = maketree.c 
maketree_LDADD = $(top_srcdir)/extras/chewing/libchewing.la

sort_dic_SOURCES = sort_dic.c
sort_dic_LDADD = $(top_srcdir)/extras/chewing/libchewing.la

sort_word_SOURCES = sort_word.c
sort_word_LDADD = $(top_srcdir)/extras/chewing/libchewing.la


$(top_srcdir)/extras/chewing/libchewing.la:
	(cd $(top_srcdir)/extras/chewing && make libchewing.la)

