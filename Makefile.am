#DEFS = -I$(top_srcdir)/src/include
SUBDIRS = extras src util inputref fonts conf doc

noinst_SCRIPT = genconf.sh


EXTRA_DIST = \
	genconf.sh \
	jmcce.spec.in

CLEANFILES = jmcce.spec

clean-jmcce.spec:
		rm -f $@
		
jmcce.spec: clean-jmcce.spec
	sed \
	  -e 's!@''prefix''@!$(prefix)!g' \
	  -e 's!@''mandir''@!$(mandir)!g' \
	  -e 's!@''pkglibdir''@!$(pkglibdir)!g' \
	  -e 's!@''JMCCE_CONFDIR''@!$(JMCCE_CONFDIR)!g' \
	  -e 's!@''JMCCE_DATADIR''@!$(JMCCE_DATADIR)!g' \
	  -e 's!@''JMCCE_FONTDIR''@!$(JMCCE_FONTDIR)!g' \
	  -e 's!@''VERSION''@!$(VERSION)!g' \
	  -e 's!@''PACKAGE''@!$(PACKAGE)!g' \
	$@.in > $@-t
	mv -f $@-t $@


