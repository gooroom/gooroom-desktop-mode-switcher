/*
 * Copyright (c) 2018-2023 Gooroom <gooroom@gooroom.kr>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <gtk/gtk.h>
#include <glib/gi18n.h>
#include <syslog.h>

#define ETC_GOOROOM_TABLET_MODE "/etc/gooroom/.tablet-mode"
#define DESKTOP_INTERFACE_SCHEMA_NAME "org.gnome.desktop.interface"
#define DESKTOP_APPLICATION_SCHEMA_NAME "org.gnome.desktop.a11y.applications"


int
main (int argc, char **argv)
{
	gchar *cmd = NULL;
	GSettings *settings = NULL;
	GSettingsSchema *schema = NULL;
	gboolean screen_keyboard_enabled = FALSE;

	/* Initialize i18n */
	setlocale (LC_ALL, "");
	bindtextdomain (GETTEXT_PACKAGE, LOCALEDIR);
	bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
	textdomain (GETTEXT_PACKAGE);

	gtk_init (&argc, &argv);

	if (!g_file_test (ETC_GOOROOM_TABLET_MODE, G_FILE_TEST_EXISTS)) {
		syslog (LOG_ERR, "gooroom-screen-keyboard-toggler: No Tablet Mode");
		return 0;
	}

	schema = g_settings_schema_source_lookup (g_settings_schema_source_get_default (),
                                              DESKTOP_APPLICATION_SCHEMA_NAME, TRUE);
	if (schema) {
		settings = g_settings_new_full (schema, NULL, NULL);
		g_settings_schema_unref (schema);
    } else {
		syslog (LOG_ERR, "gooroom-screen-keyboard-toggler: Couldn't get schema: 'org.gnome.desktop.a11y.applications'");
	}

	if (settings) {
		screen_keyboard_enabled = g_settings_get_boolean (settings, "screen-keyboard-enabled");
		g_object_unref (settings);

		// 가상키보드 온/오프
		cmd = g_strdup_printf ("/usr/bin/gsettings set %s screen-keyboard-enabled %s",
                               DESKTOP_APPLICATION_SCHEMA_NAME,
                               screen_keyboard_enabled ? "false": "true");
		g_spawn_command_line_sync (cmd, NULL, NULL, NULL, NULL);
		g_clear_pointer (&cmd, g_free);

		// 접근성 온/오프
		cmd = g_strdup_printf ("/usr/bin/gsettings set %s toolkit-accessibility true",
                               DESKTOP_INTERFACE_SCHEMA_NAME);
		g_spawn_command_line_sync (cmd, NULL, NULL, NULL, NULL);
		g_clear_pointer (&cmd, g_free);

	} else {
		syslog (LOG_ERR, "gooroom-screen-keyboard-toggler: Couldn't create gsettings from schema: org.gnome.desktop.a11y.applications");
	}

	return 0;
}
