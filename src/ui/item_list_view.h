/*
 * @file item_list_view.h  presenting items in a GtkTreeView
 *
 * Copyright (C) 2004-2015 Lars Windolf <lars.windolf@gmx.de>
 * Copyright (C) 2004-2005 Nathan J. Conrad <t98502@users.sourceforge.net>
 *	      
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 * 
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifndef _ITEM_LIST_VIEW_H
#define _ITEM_LIST_VIEW_H

#include <glib-object.h>
#include <glib.h>
#include <gtk/gtk.h>

#include "item.h"
#include "node_view.h"

/* This class realizes an GtkTreeView based item view. Instances
   of ItemListView are managed by the ItemListView. This class hides
   the GtkTreeView and implements performance optimizations. */

G_BEGIN_DECLS

#define ITEM_LIST_VIEW_TYPE		(item_list_view_get_type ())
#define ITEM_LIST_VIEW(obj)		(G_TYPE_CHECK_INSTANCE_CAST ((obj), ITEM_LIST_VIEW_TYPE, ItemListView))
#define ITEM_LIST_VIEW_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST ((klass), ITEM_LIST_VIEW_TYPE, ItemListViewClass))
#define IS_ITEM_LIST_VIEW(obj)		(G_TYPE_CHECK_INSTANCE_TYPE ((obj), ITEM_LIST_VIEW_TYPE))
#define IS_ITEM_LIST_VIEW_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), ITEM_LIST_VIEW_TYPE))

typedef struct ItemListView		ItemListView;
typedef struct ItemListViewClass	ItemListViewClass;
typedef struct ItemListViewPrivate	ItemListViewPrivate;

struct ItemListView
{
	GObject		parent;
	
	/*< private >*/
	ItemListViewPrivate	*priv;
};

struct ItemListViewClass 
{
	GObjectClass parent_class;	
};

GType item_list_view_get_type (void);

/**
 * item_list_view_create: (skip)
 * @wide:	TRUE if ItemListView should be optimized for wide view(itemview->priv->currentLayoutMode == NODE_VIEW_MODE_WIDE)
 *
 * Create a new ItemListView instance.
 *
 * Returns: (transfer none):	the ItemListView instance
 */
ItemListView * item_list_view_create (gboolean wide);

/**
 * item_list_view_get_widget:
 *
 * Returns the GtkTreeView used by the ItemListView instance.
 *
 * Returns: (transfer none): a GtkTreeView
 */
GtkTreeView * item_list_view_get_widget (ItemListView *ilv);

/**
 * item_list_view_contains_id:
 * @ilv:	the ItemListView
 * @id: 	the item id
 *
 * Checks whether the given id is in the ItemListView.
 *
 * Returns: TRUE if the item is in the ItemListView
 */
gboolean item_list_view_contains_id (ItemListView *ilv, gulong id);

/**
 * item_list_view_set_sort_column:
 * @ilv:		the ItemListView
 * @sortType:   	new sort type
 * @sortReversed:	TRUE for ascending order
 *
 * Changes the sorting type (and direction).
 */
void item_list_view_set_sort_column (ItemListView *ilv, nodeViewSortType sortType, gboolean sortReversed);

/**
 * item_list_view_select: (skip)
 * @ilv:	the ItemListView
 * @item:	the item to select
 *
 * Selects the given item (if it is in the ItemListView).
 */
void item_list_view_select (ItemListView *ilv, itemPtr item);

/**
 * item_list_view_add_item: (skip)
 * @ilv:	the ItemListView
 * @item:	the item to add
 *
 * Add an item to an ItemListView. This method is expensive and
 * is to be used only for new items that need to be inserted
 * by background updates.
 */
void item_list_view_add_item (ItemListView *ilv, itemPtr item);

/**
 * item_list_view_remove_item: (skip)
 * @ilv:	the ItemListView
 * @item:	the item to remove
 *
 * Remove an item from an ItemListView. This method is expensive
 * and is to be used only for items removed by background updates
 * (usually cache drops).
 */
void item_list_view_remove_item (ItemListView *ilv, itemPtr item);

/**
 * item_list_view_enable_favicon:
 * @ilv:		the ItemListView
 * @enabled:    	TRUE if column is to be visible
 *
 * Enable the favicon column of the currently displayed itemlist.
 */
void item_list_view_enable_favicon_column (ItemListView *ilv, gboolean enabled);

/**
 * item_list_view_clear: (skip)
 * @ilv:	the ItemListView
 *
 * Remove all items and resets a ItemListView.
 */
void item_list_view_clear (ItemListView *ilv);

/**
 * item_list_view_update: (skip)
 * @ilv:	        the ItemListView
 * @hasEnclosures:	TRUE if at least one item has an enclosure
 *
 * Update the ItemListView with the newly added items. To be called
 * after doing a batch of item_list_view_add_item() calls.
 */
void item_list_view_update (ItemListView *ilv, gboolean hasEnclosures);

/* menu callbacks */

/*
 * Toggles the unread status of the selected item. This is called from
 * a menu.
 */
void on_toggle_unread_status (GtkMenuItem *menuitem, gpointer user_data);

/*
 * Toggles the flag of the selected item. This is called from a menu.
 */
void on_toggle_item_flag (GtkMenuItem *menuitem, gpointer user_data);

/*
 * Opens the selected item in a browser.
 */
void on_popup_launch_item_selected (void);

/*
 * Opens the selected item in a browser.
 */
void on_popup_launch_item_in_tab_selected (void);

/*
 * Opens the selected item in a browser.
 */
void on_popup_launch_item_external_selected (void);

/*
 * Toggles the read status of right-clicked item.
 */
void on_popup_toggle_read (void);

/*
 * Toggles the flag of right-clicked item.
 */
void on_popup_toggle_flag (void);

/**
 * on_remove_items_activate: (skip)
 * @menuitem: The menuitem that was selected.
 * @user_data: Unused.
 *
 * Removes all items from the selected feed.
 */
void on_remove_items_activate (GtkMenuItem *menuitem, gpointer user_data);

/**
 * on_remove_item_activate: (skip)
 * @menuitem: The menuitem that was selected.
 * @user_data: Unused.
 *
 * Removes the selected item from the selected feed.
 */  
void on_remove_item_activate (GtkMenuItem *menuitem, gpointer user_data);

void on_popup_remove_selected (void);

/**
 * item_list_view_find_unread_item: (skip)
 * @ilv:		the ItemListView
 * @startId:		0 or the item id to start from
 *
 * Finds and selects the next unread item starting at the given
 * item in a ItemListView according to the current GtkTreeView sorting order.
 *
 * Returns: (nullable): unread item (or NULL)
 */
itemPtr item_list_view_find_unread_item (ItemListView *ilv, gulong startId);

/**
 * on_next_unread_item_activate: (skip)
 * @menuitem: The menuitem that was selected.
 * @user_data: Unused.
 *
 * Searches the displayed feed and then all feeds for an unread
 * item. If one it found, it is displayed.
 */
void on_next_unread_item_activate (GtkMenuItem *menuitem, gpointer user_data);

/**
 * item_list_view_update_item: (skip)
 * @ilv:	the ItemListView
 * @item:	the item
 *
 * Update a single item of a ItemListView
 */
void item_list_view_update_item (ItemListView *ilv, itemPtr item);

/**
 * item_list_view_update_all_items: (skip)
 * @ilv:	the ItemListView
 *
 * Update all items of the ItemListView. To be used after 
 * initial batch loading.
 */
void item_list_view_update_all_items (ItemListView *ilv);

/**
 * on_popup_copy_URL_clipboard: (skip)
 *
 * Copies the selected items URL to the clipboard.
 */
void on_popup_copy_URL_clipboard (void);

void on_popup_social_bm_item_selected (void);

#endif
