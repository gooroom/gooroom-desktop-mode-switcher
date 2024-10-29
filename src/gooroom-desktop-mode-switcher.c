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

#define ETC_GOOROOM_TABLET_MODE "/etc/gooroom/.tablet-mode"
#define DESKTOP_INTERFACE_SCHEMA_NAME "org.gnome.desktop.interface"
#define DESKTOP_APPLICATION_SCHEMA_NAME "org.gnome.desktop.a11y.applications"

static gboolean init_tablet_mode = FALSE;


static void
show_error_dialog (const gchar *title, const gchar *message)
{
	if (!message) return;

	GtkWidget *dialog;

	dialog = gtk_message_dialog_new (NULL,
                                     GTK_DIALOG_MODAL,
                                     GTK_MESSAGE_ERROR,
                                     GTK_BUTTONS_CLOSE,
                                     NULL);

	gtk_message_dialog_format_secondary_text (GTK_MESSAGE_DIALOG (dialog), "%s", message);
	gtk_window_set_title (GTK_WINDOW (dialog), title);
	gtk_dialog_run (GTK_DIALOG (dialog));
	gtk_widget_destroy (dialog);
}

static gboolean
launch_tablet_mode_switching_command (gboolean on)
{
	gboolean ret;
	GError *error = NULL;
	gchar *pkexec, *cmdline;

	pkexec = g_find_program_in_path ("pkexec");

	if (on) {
		cmdline = g_strdup_printf ("%s %s", pkexec, TABLET_MODE_CHANGE_HELPER);
	} else {
		cmdline = g_strdup_printf ("%s %s -d", pkexec, TABLET_MODE_CHANGE_HELPER);
	}

	ret = g_spawn_command_line_sync (cmdline, NULL, NULL, NULL, &error);
	if (!ret) {
		if (error) { 
			g_warning ("Error attempting to execute command: %s: %s", cmdline, error->message);
			g_error_free (error);
		} else {
			g_warning ("Error attempting to execute command: %s", cmdline);
		}
	}

	g_free (cmdline);

	return ret;
}

static void
screen_keyboard_toggled (gboolean on)
{
	gchar *cmdline = NULL;

	cmdline = g_strdup_printf ("/usr/bin/gsettings set %s screen-keyboard-enabled %s",
                               DESKTOP_APPLICATION_SCHEMA_NAME, on ? "true" : "false");
	g_spawn_command_line_sync (cmdline, NULL, NULL, NULL, NULL);
	g_free (cmdline);

	cmdline = g_strdup_printf ("/usr/bin/gsettings set %s toolkit-accessibility true",
                               DESKTOP_INTERFACE_SCHEMA_NAME);
	g_spawn_command_line_sync (cmdline, NULL, NULL, NULL, NULL);
	g_free (cmdline);
}

static gboolean
logout_idle_cb (gpointer user_data)
{
	gchar *message = NULL;
	gchar *logout_command = NULL, *cmdline = NULL;

	logout_command = g_find_program_in_path ("gooroom-logout-command");
	if (logout_command) {
		cmdline = g_strdup_printf ("%s --logout --delay=500", logout_command);
	} else {
		logout_command = g_find_program_in_path ("gnome-session-quit");
		if (logout_command) {
			cmdline = g_strdup_printf ("%s --logout --force --no-prompt", logout_command);
		}
	}

	if (cmdline) {
		GError *error = NULL;

		if (!g_spawn_command_line_sync (cmdline, NULL, NULL, NULL, &error)) {
			if (error) {
				g_warning ("Error attempting to execute command: %s: %s", cmdline, error->message);
				g_error_free (error);
			} else {
				g_warning ("Error attempting to execute command: %s", cmdline);
			}

			g_free (cmdline);

			message = g_strdup (_("Failed to system logout\n"
                                  "Please check gooroom-logout or gnome-session-quit program."));
			show_error_dialog (_("System Logout Error"), message);
			g_free (message);

			// restore
			goto restore;
		}
		goto quit;
	}

	message = g_strdup (_("Not found logout command.\n"
                          "Install gooroom-logout or gnome-session-bin packages."));
	show_error_dialog (_("System Logout Error"), message);
	g_free (message);


restore:
	if (!launch_tablet_mode_switching_command (init_tablet_mode)) {
		if (init_tablet_mode) {
			message = g_strdup_printf (_("Failed to restore Tablet Mode\n"
                                         "Please create %s manually."),
                                         ETC_GOOROOM_TABLET_MODE);
		} else {
			message = g_strdup_printf (_("Failed to restore Normal(PC) Mode\n"
                                         "Please delete %s manually."),
                                         ETC_GOOROOM_TABLET_MODE);
		}
		show_error_dialog (_("Desktop Mode Restore Failure"), message);
		g_free (message);
	}
	screen_keyboard_toggled (init_tablet_mode);


quit:
	gtk_main_quit ();

	return FALSE;
}

static void
desktop_mode_switching_dialog_response_cb (GtkDialog *dialog,
                                           gint       response_id,
                                           gpointer   user_data)
{
	gtk_widget_destroy (GTK_WIDGET (dialog));

	if (response_id == GTK_RESPONSE_YES) {
		if (!launch_tablet_mode_switching_command (!init_tablet_mode)) {
			gchar *msg = NULL;
			if (init_tablet_mode) {
				msg = g_strdup_printf (_("Failed to switch Normal(PC) Mode.\n"
                                         "Please check %s program"),
                                         TABLET_MODE_CHANGE_HELPER);
			} else {
				msg = g_strdup_printf (_("Failed to switch Tablet Mode.\n"
                                         "Please check %s program"),
                                         TABLET_MODE_CHANGE_HELPER);
			}
			show_error_dialog (_("Desktop Mode Switching"), msg);
			g_free (msg);
			goto quit;
		}
		screen_keyboard_toggled (!init_tablet_mode);
		g_idle_add ((GSourceFunc) logout_idle_cb, NULL);
		return;
	}

quit:
	gtk_main_quit ();
}

static gboolean
desktop_mode_switching_idle (gpointer user_data)
{
	gchar *msg = NULL;
	GtkWidget *dialog;

	if (init_tablet_mode) {
		msg = _("To switch to normal mode, you must log in again.\n"
                "Would you like to log in again now?");
	} else {
		msg = _("To switch to tablet mode, you must log in again.\n"
                "Would you like to log in again now?");
	}

	dialog = gtk_message_dialog_new (NULL,
                                     GTK_DIALOG_MODAL,
                                     GTK_MESSAGE_ERROR,
                                     GTK_BUTTONS_NONE,
                                     NULL);
	gtk_dialog_add_buttons (GTK_DIALOG (dialog),
                            _("Yes"), GTK_RESPONSE_YES,
                            _("No"), GTK_RESPONSE_NO,
                            NULL);
	gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_YES);

	gtk_message_dialog_format_secondary_text (GTK_MESSAGE_DIALOG (dialog), "%s", msg);
	gtk_window_set_title (GTK_WINDOW (dialog), _("Desktop Mode Switching"));
	gtk_widget_show_all (dialog);

	g_signal_connect (dialog, "response",
                      G_CALLBACK (desktop_mode_switching_dialog_response_cb), NULL);

	return FALSE;
}

int
main (int argc, char **argv)
{
	/* Initialize i18n */
	setlocale (LC_ALL, "");
	bindtextdomain (GETTEXT_PACKAGE, LOCALEDIR);
	bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
	textdomain (GETTEXT_PACKAGE);

	gtk_init (&argc, &argv);

	init_tablet_mode = g_file_test (ETC_GOOROOM_TABLET_MODE, G_FILE_TEST_EXISTS);

	g_idle_add ((GSourceFunc)desktop_mode_switching_idle, NULL);

	gtk_main ();

	return 0;
}
