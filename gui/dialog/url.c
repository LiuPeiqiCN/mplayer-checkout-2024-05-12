/*
 * This file is part of MPlayer.
 *
 * MPlayer is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * MPlayer is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with MPlayer; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "url.h"
#include "dialog.h"
#include "tools.h"
#include "gui/gtk-compat.h"
#include "gui/interface.h"
#include "gui/app/app.h"
#include "gui/app/gui.h"
#include "gui/ui/actions.h"
#include "gui/util/list.h"

#include "help_mp.h"
#include "stream/stream.h"

GtkWidget *URLDialog;

static GtkWidget *urlCombo;

/**
 * @brief Add the entered URL to the URL list and stream it,
 *        if the button clicked is the OK button
 *
 * @note If the scheme is missing, http is assumed (and added).
 *
 * @param button object which received the signal
 * @param user_data user data set when the signal handler was connected
 *
 * @note The button is determined by checking @a user_data.
 */
static void button_clicked(GtkButton *button, gpointer user_data)
{
    (void)button;

    if (user_data) {
        char *str = strdup(gtk_entry_get_text(gtkEntry(urlCombo)));

        if (str) {
            if (!strstr(str, "://")) {
                char *tmp = malloc(strlen(str) + 8);

                if (tmp)
                    sprintf(tmp, "http://%s", str);

                free(str);
                str = tmp;
            }

            if (str) {
                urlItem *item;

                uiSetFile(NULL, str, STREAMTYPE_STREAM);
                listMgr(PLAYLIST_DELETE, 0);
                add_to_gui_playlist(str, PLAYLIST_ITEM_APPEND);

                item = calloc(1, sizeof(*item));

                if (item) {
                    item->url = str;
                    listMgr(URLLIST_ITEM_ADD, item);
                }

                guiInfo.MediumChanged = GUI_MEDIUM_NEW;
                uiEvent(evPlay, 0);
            }
        }
    }

    gtk_widget_destroy(URLDialog);
}

static GtkWidget *CreateURLDialog(void)
{
    GtkWidget *vbox1;
    GtkWidget *hbox1;
    GtkWidget *hbuttonbox1;
    GtkWidget *Ok;
    GtkWidget *Cancel;
    GtkAccelGroup *accel_group;
    GdkGeometry geometry;

    accel_group = gtk_accel_group_new();

    URLDialog = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_widget_set_size_request(URLDialog, 384, -1);
    gtk_window_set_title(GTK_WINDOW(URLDialog), _(MSGTR_GUI_NetworkStreaming));
    gtk_window_set_position(GTK_WINDOW(URLDialog), GTK_WIN_POS_CENTER);
    gtk_window_set_wmclass(GTK_WINDOW(URLDialog), "Network", MPlayer);

    gtk_widget_realize(URLDialog);
    gtkAddIcon(URLDialog);

    vbox1 = gtkAddVBox(gtkAddDialogFrame(URLDialog), 0);
    hbox1 = gtkAddHBox(vbox1, 1);
    gtkAddLabel("URL: ", hbox1);

    urlCombo = gtkAddCombo(hbox1);

    gtkAddHSeparator(vbox1);

    hbuttonbox1 = gtkAddHButtonBox(vbox1);
    gtk_button_box_set_layout(GTK_BUTTON_BOX(hbuttonbox1), GTK_BUTTONBOX_END);
    gtk_box_set_spacing(GTK_BOX(hbuttonbox1), 10);

    Ok     = gtkAddButton(_(MSGTR_GUI_Ok), hbuttonbox1);
    Cancel = gtkAddButton(_(MSGTR_GUI_Cancel), hbuttonbox1);

    geometry.max_width  = gdk_screen_get_width(gtk_widget_get_screen(URLDialog));
    geometry.max_height = -1;
    gtk_window_set_geometry_hints(GTK_WINDOW(URLDialog), NULL, &geometry, GDK_HINT_MAX_SIZE);

    gtk_widget_add_accelerator(Ok, "clicked", accel_group, GDK_KEY_Return, 0, GTK_ACCEL_VISIBLE);
    gtk_widget_add_accelerator(Cancel, "clicked", accel_group, GDK_KEY_Escape, 0, GTK_ACCEL_VISIBLE);

    g_signal_connect(G_OBJECT(URLDialog), "destroy", G_CALLBACK(gtk_widget_destroyed), &URLDialog);
    g_signal_connect(G_OBJECT(Ok), "clicked", G_CALLBACK(button_clicked), Ok);
    g_signal_connect(G_OBJECT(Cancel), "clicked", G_CALLBACK(button_clicked), NULL);

    gtk_widget_grab_focus(urlCombo);
    gtk_window_add_accel_group(GTK_WINDOW(URLDialog), accel_group);

    return URLDialog;
}

void ShowURLDialog(void)
{
    urlItem *item;

    if (URLDialog)
        gtkRaise(URLDialog);
    else
        URLDialog = CreateURLDialog();

    item = listMgr(URLLIST_GET, 0);

    if (item) {
        while (item) {
            gtk_combo_box_append_text(GTK_COMBO_BOX(urlCombo), item->url);
            item = item->next;
        }

        gtk_combo_box_set_active(GTK_COMBO_BOX(urlCombo), 0);
    }

    gtk_widget_show(URLDialog);
}
