INCLUDES = \
	-I. \
	-I$(top_srcdir)/src \
	-DPACKAGE_DATA_DIR=\""$(datadir)"\" \
	$(PACKAGE_CFLAGS) \
	$(G_CAST_CHECKS)

module_LTLIBRARIES = pm.la

moduledir = $(libdir)/lxpanel/plugins

pm_la_SOURCES = \
	pm.c

pm_la_LIBADD = \
	$(PACKAGE_LIBS)

pm_la_LDFLAGS = \
	-module \
	@LXPANEL_MODULE@
