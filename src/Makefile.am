
bin_PROGRAMS = lightdm-matrix-greeter

light_matrix_greeter_built_sources =\
				    lightdm-matrix-greeter-ui.h\
				    lightdm-matrix-greeter-css.h

lightdm_matrix_greeter_SOURCES =\
			       	$(light_matrix_greeter_built_sources)\
			       	matrix_greeter.c
AM_CPPFLAGS = \
	-DGREETER_DATA_DIR=\""$(datadir)/lightdm-matrix-greeter"\" \
	-DCONFIG_FILE=\""$(sysconfdir)/lightdm/lightdm-matrix-greeter.conf"\"      


lightdm_matrix_greeter_CFLAGS = $(GTK_CFLAGS) $(GMODULE_CFLAGS) $(LIGHTDMGOBJECT_CFLAGS)
lightdm_matrix_greeter_LDADD = $(GTK_LIBS) $(GMODULE_LIBS) $(LIGHTDMGOBJECT_LIBS)

lightdm-matrix-greeter-ui.h: $(srcdir)/matrix_lightdm_greeter.ui Makefile
	$(AM_V_GEN) exo-csource --static --strip-comments --strip-content --name=lightdm_gtk_greeter_ui $< >$@
lightdm-matrix-greeter-css.h: $(srcdir)/matrix_lightdm_greeter.css Makefile
	$(AM_V_GEN) exo-csource --static --name=matrix_lightdm_css $< >$@
BUILT_SOURCES = $(light_matrix_greeter_built_sources)

