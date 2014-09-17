/**
 * @file popup_menu.h popup menus
 *
 * Copyright (C) 2003-2011 Lars Windolf <lars.windolf@gmx.de>
 * Copyright (C) 2004-2005 Nathan J. Conrad <t98502@users.sourceforge.net>
 * Copyright (C) 2009 Adrian Bunk <bunk@users.sourceforge.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA 
 */

#ifndef _POPUP_MENU_H
#define _POPUP_MENU_H

#include <gtk/gtk.h>

#include "item.h"
#include "enclosure.h"

/**
 * Shows a popup menu with options for the item list and the
 * given selected item.
 * (Open Link, Copy Item, Copy Link...)
 *
 * @param item	the selected item
 * @param button	the mouse button which was pressed to initiate the event
 * @param activate_time	the time at which the activation event occurred
 */
void ui_popup_item_menu (itemPtr item, guint button, guint32 activate_time);

/**
 * Shows a popup menu for the systray icon.
 * (Offline mode, Close, Minimize...)
 *
 * @param func  the function to calculate the position to popup
 * @param button	the mouse button which was pressed to initiate the event
 * @param activate_time	the time at which the activation event occurred
 * @param user_data  the user data to pass to func.
 */
void ui_popup_systray_menu (GtkMenuPositionFunc func, guint button, guint32 activate_time, gpointer user_data);

/**
 * Shows a popup menu for the enclosure list view.
 * (Save As, Open With...)
 *
 * @param enclosure	the enclosure
 * @param button	the mouse button which was pressed to initiate the event
 * @param activate_time	the time at which the activation event occurred
 */
void ui_popup_enclosure_menu (enclosurePtr enclosure, guint button,
			      guint32 activate_time);

/* GUI callbacks */

gboolean
on_mainfeedlist_button_press_event     (GtkWidget       *widget,
                                        GdkEventButton  *event,
                                        gpointer         user_data);

gboolean
on_mainfeedlist_popup_menu (GtkWidget *widget,
                            gpointer   user_data);

#endif
