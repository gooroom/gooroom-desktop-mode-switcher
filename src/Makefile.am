bin_PROGRAMS = gooroom-desktop-mode-switcher

gooroom_desktop_mode_switcher_SOURCES = \
	main.c

gooroom_desktop_mode_switcher_CFLAGS = \
	-DLOCALEDIR=\""$(localedir)"\" \
	-DTABLET_MODE_CHANGE_HELPER=\"$(libexecdir)/tablet-mode-change-helper\" \
	$(GTK_CFLAGS) \
	$(GLIB_CFLAGS)

gooroom_desktop_mode_switcher_LDADD = \
	$(GTK_LIBS) \
	$(GLIB_LIBS)
