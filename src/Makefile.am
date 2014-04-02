INCLUDES = \
	-I. \
	-I$(top_srcdir)/src \
	-DPACKAGE_DATA_DIR=\""$(datadir)"\" \
	$(PACKAGE_CFLAGS) \
	$(G_CAST_CHECKS)

module_LTLIBRARIES = ping.la

moduledir = $(libdir)/lxpanel/plugins

ping_la_SOURCES = \
	ping.c

ping_la_LIBADD = \
	$(PACKAGE_LIBS)

ping_la_LDFLAGS = \
	-module \
	@LXPANEL_MODULE@
