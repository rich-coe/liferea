/*
 * @file browser_tabs.h  internal browsing using multiple tabs
 *
 * Copyright (C) 2004-2008 Lars Windolf <lars.windolf@gmx.de>
 * Copyright (C) 2006 Nathan Conrad <conrad@bungled.net>
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

#ifndef _BROWSER_TABS_H
#define _BROWSER_TABS_H

#include <gtk/gtk.h>

#include "liferea_htmlview.h"

G_BEGIN_DECLS

#define BROWSER_TABS_TYPE		(browser_tabs_get_type ())
#define BROWSER_TABS(obj)		(G_TYPE_CHECK_INSTANCE_CAST ((obj), BROWSER_TABS_TYPE, BrowserTabs))
#define BROWSER_TABS_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST ((klass), BROWSER_TABS_TYPE, BrowserTabsClass))
#define IS_BROWSER_TABS(obj)		(G_TYPE_CHECK_INSTANCE_TYPE ((obj), BROWSER_TABS_TYPE))
#define IS_BROWSER_TABS_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), BROWSER_TABS_TYPE))

typedef struct BrowserTabs		BrowserTabs;
typedef struct BrowserTabsClass		BrowserTabsClass;
typedef struct BrowserTabsPrivate	BrowserTabsPrivate;

extern BrowserTabs *browser_tabs;

struct BrowserTabs
{
	GObject		parent;
	
	/*< private >*/
	BrowserTabsPrivate	*priv;
};

struct BrowserTabsClass 
{
	GObjectClass parent_class;
};

GType browser_tabs_get_type (void);

/**
 * browser_tabs_create: (skip)
 * @notebook:	GtkNotebook widget to use
 *
 * Returns: the singleton browser tabs instance.
 *
 */
BrowserTabs * browser_tabs_create (GtkNotebook *notebook);

/**
 * browser_tabs_add_new:
 * @url:	URL to be loaded in new tab (can be NULL to do nothing)
 * @title:	title of the tab to be created
 * @activate:   Should the new tab be put in the foreground?
 *
 * Adds a new tab with the specified URL and title.
 *
 * Returns: the newly created HTML view
 */
LifereaHtmlView * browser_tabs_add_new (const gchar *url, const gchar *title, gboolean activate);

/**
 * browser_tabs_show_headlines:
 *
 * makes the headline tab visible 
 */
void browser_tabs_show_headlines (void);

/**
 * browser_tabs_get_active_htmlview:
 *
 * Used to determine which HTML view (a tab or the headlines view)
 * is currently visible and can be used to display HTML that
 * is to be loaded
 *
 * Returns: (transfer none) (nullable): HTML view widget
 */
LifereaHtmlView * browser_tabs_get_active_htmlview (void);

/**
 * browser_tabs_do_zoom:
 * @in:	TRUE if zooming in, FALSE for zooming out
 *
 * Requests the tab to change zoom level.
 */
void browser_tabs_do_zoom (gboolean in);

/* All widget elements and state of a tab */
typedef struct _tabInfo tabInfo;
struct _tabInfo {
	GtkWidget	*label;		/*<< the tab label */
	GtkWidget	*widget;	/*<< the embedded child widget */
	LifereaHtmlView	*htmlview;	/*<< the tabs HTML view widget */
};

G_END_DECLS

#endif
