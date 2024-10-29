bin_PROGRAMS = \
	gooroom-desktop-mode-switcher \
	gooroom-screen-keyboard-toggler

gooroom_desktop_mode_switcher_SOURCES = \
	gooroom-desktop-mode-switcher.c

gooroom_desktop_mode_switcher_CFLAGS = \
	-DLOCALEDIR=\""$(localedir)"\" \
	-DTABLET_MODE_CHANGE_HELPER=\"$(libexecdir)/tablet-mode-change-helper\" \
	$(GTK_CFLAGS) \
	$(GLIB_CFLAGS)

gooroom_desktop_mode_switcher_LDADD = \
	$(GTK_LIBS) \
	$(GLIB_LIBS)

gooroom_screen_keyboard_toggler_SOURCES = \
	gooroom-screen-keyboard-toggler.c

gooroom_screen_keyboard_toggler_CFLAGS = \
	-DLOCALEDIR=\""$(localedir)"\" \
	$(GTK_CFLAGS) \
	$(GLIB_CFLAGS)

gooroom_screen_keyboard_toggler_LDADD = \
	$(GTK_LIBS) \
	$(GLIB_LIBS)
