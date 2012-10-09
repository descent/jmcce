dnl
dnl
dnl


AC_DEFUN(AC_WITH_JMCCE_FONTDIR,[
	AC_ARG_WITH(
	jmcce_fontdir,
	[  --with-jmcce-fontdir    set the font files path. ],
	jmcce_fontdir="$with_val")
	JMCCE_FONTDIR=${jmcce_fontdir:-${datadir}/fonts/chinese}
	AC_SUBST(JMCCE_FONTDIR)
])

