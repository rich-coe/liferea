/**
 * @file webkit.c  WebKit browser module for Liferea
 *
 * Copyright (C) 2007-2010 Lars Windolf <lars.windolf@gmx.de>
 * Copyright (C) 2008 Lars Strojny <lars@strojny.net>
 * Copyright (C) 2009-2012 Emilio Pozuelo Monfort <pochu27@gmail.com>
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

#include <libsoup/soup.h>
#include <webkit2/webkit2.h>
#include <string.h>

#include "browser.h"
#include "conf.h"
#include "common.h"
#include "ui/browser_tabs.h"
#include "ui/liferea_htmlview.h"

static WebKitSettings *settings = NULL;

/**
 * Update the settings object if the preferences change.
 * This will affect all the webviews as they all use the same
 * settings object.
 */
static void
liferea_webkit_disable_javascript_cb (GSettings *gsettings,
				      gchar *key,
				      gpointer user_data)
{
	g_return_if_fail (key != NULL);

	g_object_set (
		settings,
		"enable-javascript",
		!g_settings_get_boolean (gsettings, key),
		NULL
	);
}

/**
 * Update the settings object if the preferences change.
 * This will affect all the webviews as they all use the same
 * settings object.
 */
static void
liferea_webkit_enable_plugins_cb (GSettings *gsettings,
				  gchar *key,
				  gpointer user_data)
{
	g_return_if_fail (key != NULL);

	g_object_set (
		settings,
		"enable-plugins",
		g_settings_get_boolean (gsettings, key),
		NULL
	);
}

static gchar *
webkit_get_font (guint *size)
{
	gchar *font = NULL;

	*size = 11;	/* default fallback */

	/* font configuration support */
	conf_get_str_value (USER_FONT, &font);
	if (NULL == font || 0 == strlen (font)) {
		if (NULL != font) {
			g_free (font);
			font = NULL;
		}
		conf_get_default_font_from_schema (DEFAULT_FONT, &font);
	}

	if (font) {
		/* The GTK2/GNOME font name format is "<font name> <size>" */
		gchar *tmp = strrchr(font, ' ');
		if (tmp) {
			*tmp++ = 0;
			*size = atoi(tmp);
		}
	}

	return font;
}

/**
 * HTML renderer init method
 */
static void
liferea_webkit_init (void)
{
	gboolean	disable_javascript, enable_plugins;
	gchar		*font;
	guint		fontSize;

	g_assert (!settings);

	settings = webkit_settings_new ();
	font = webkit_get_font (&fontSize);

	if (font) {
		g_object_set (
			settings,
			"default-font-family",
			font,
			NULL
		);
		g_object_set (
			settings,
			"default-font-size",
			fontSize,
			NULL
		);
		g_free (font);
	}
	g_object_set (
		settings,
		"minimum-font-size",
		7,
		NULL
	);
	conf_get_bool_value (DISABLE_JAVASCRIPT, &disable_javascript);
	g_object_set (
		settings,
		"enable-javascript",
		!disable_javascript,
		NULL
	);
	conf_get_bool_value (ENABLE_PLUGINS, &enable_plugins);
	g_object_set (
		settings,
		"enable-plugins",
		enable_plugins,
		NULL
	);

	conf_signal_connect (
		"changed::" DISABLE_JAVASCRIPT,
		G_CALLBACK (liferea_webkit_disable_javascript_cb),
		NULL
	);

	conf_signal_connect (
		"changed::" ENABLE_PLUGINS,
		G_CALLBACK (liferea_webkit_enable_plugins_cb),
		NULL
	);
}

/**
 * Load HTML string into the rendering scrollpane
 *
 * Load an HTML string into the web view. This is used to render
 * HTML documents created internally.
 */
static void
liferea_webkit_write_html (
	GtkWidget *webview,
	const gchar *string,
	const guint length,
	const gchar *base,
	const gchar *content_type
)
{
	// FIXME Avoid doing a copy ?
	GBytes *string_bytes = g_bytes_new (string, length);
	/* Note: we explicitely ignore the passed base URL
	   because we don't need it as Webkit supports <div href="">
	   and throws a security exception when accessing file://
	   with a non-file:// base URL */
	webkit_web_view_load_bytes (WEBKIT_WEB_VIEW (webview), string_bytes, content_type, "UTF-8", "file://");
	g_bytes_unref (string_bytes);
}

static void
liferea_webkit_title_changed (WebKitWebView *view, GParamSpec *pspec, gpointer user_data)
{
	LifereaHtmlView	*htmlview;
	gchar *title;

	htmlview = g_object_get_data (G_OBJECT (view), "htmlview");
	g_object_get (view, "title", &title, NULL);

	liferea_htmlview_title_changed (htmlview, title);
	g_free (title);
}

static void
liferea_webkit_location_changed (WebKitWebView *view, GParamSpec *pspec, gpointer user_data)
{
	LifereaHtmlView	*htmlview;
	gchar *location;

	htmlview = g_object_get_data (G_OBJECT (view), "htmlview");
	g_object_get (view, "uri", &location, NULL);

	liferea_htmlview_location_changed (htmlview, location);
	g_free (location);
}

/*
 *  Callback for the mouse-target-changed signal.
 *
 *  Updates selected_url with hovered link.
 */
static void
liferea_webkit_on_mouse_target_changed (WebKitWebView 	    *view,
					WebKitHitTestResult *hit_result,
					guint                modifiers,
					gpointer             user_data)
{
	LifereaHtmlView	*htmlview;
	gchar *selected_url;

	htmlview = g_object_get_data (G_OBJECT (view), "htmlview");
	selected_url = g_object_get_data (G_OBJECT (view), "selected_url");
	if (selected_url)
		g_free (selected_url);

	if (webkit_hit_test_result_context_is_link (hit_result))
	{
		g_object_get (hit_result, "link-uri", &selected_url, NULL);
	} else {
		selected_url = g_strdup ("");
	}

	/* overwrite or clear last status line text */
	liferea_htmlview_on_url (htmlview, selected_url);

	g_object_set_data (G_OBJECT (view), "selected_url", selected_url);
}

/**
 * A link has been clicked
 *
 * When a link has been clicked the link management is dispatched to Liferea
 * core in order to manage the different filetypes, remote URLs.
 */
static gboolean
liferea_webkit_link_clicked (WebKitWebView *view,
			     WebKitPolicyDecision *policy_decision)
{
	const gchar			*uri;
	WebKitNavigationAction 		*navigation_action;
	WebKitURIRequest		*request;
	WebKitNavigationType		reason;
	gboolean			url_handled;

	g_return_val_if_fail (WEBKIT_IS_WEB_VIEW (view), FALSE);
	g_return_val_if_fail (WEBKIT_IS_POLICY_DECISION (policy_decision), FALSE);

	navigation_action = webkit_navigation_policy_decision_get_navigation_action (WEBKIT_NAVIGATION_POLICY_DECISION (policy_decision));
	reason = webkit_navigation_action_get_navigation_type (navigation_action);

	/* iframes in items return WEBKIT_WEB_NAVIGATION_REASON_OTHER
	   and shouldn't be handled as clicks                          */
	if (reason != WEBKIT_NAVIGATION_TYPE_LINK_CLICKED)
		return FALSE;

	request = webkit_navigation_action_get_request (navigation_action);
	uri = webkit_uri_request_get_uri (request);

	if (webkit_navigation_action_get_mouse_button (navigation_action) == 2) { /* middle click */
		browser_tabs_add_new (uri, uri, FALSE);
		webkit_policy_decision_ignore (policy_decision);
		return TRUE;
	}

	url_handled = liferea_htmlview_handle_URL (g_object_get_data (G_OBJECT (view), "htmlview"), uri);

	if (url_handled)
		webkit_policy_decision_ignore (policy_decision);

	return url_handled;
}

/**
 * A new window was requested. This is the case e.g. if the link
 * has target="_blank". In that case, we don't open the link in a new
 * tab, but do what the user requested as if it didn't have a target.
 */
static gboolean
liferea_webkit_new_window_requested (WebKitWebView *view,
				     WebKitPolicyDecision *policy_decision)
{
	WebKitNavigationAction 		*navigation_action;
	WebKitURIRequest		*request;
	const gchar 			*uri;

	navigation_action = webkit_navigation_policy_decision_get_navigation_action (WEBKIT_NAVIGATION_POLICY_DECISION (policy_decision));
	request = webkit_navigation_action_get_request (navigation_action);
	uri = webkit_uri_request_get_uri (request);

	if (webkit_navigation_action_get_mouse_button (navigation_action) == 2) {
		/* middle-click, let's open the link in a new tab */
		browser_tabs_add_new (uri, uri, FALSE);
	} else if (liferea_htmlview_handle_URL (g_object_get_data (G_OBJECT (view), "htmlview"), uri)) {
		/* The link is to be opened externally, let's do nothing here */
	} else {
		/* If the link is not to be opened in a new tab, nor externally,
		 * it was likely a normal click on a target="_blank" link.
		 * Let's open it in the current view to not disturb users */
		webkit_web_view_load_uri (view, uri);
	}

	/* We handled the request ourselves */
	webkit_policy_decision_ignore (policy_decision);
	return TRUE;
}


static gboolean
liferea_webkit_decide_policy (WebKitWebView *view,
			      WebKitPolicyDecision *decision,
			      WebKitPolicyDecisionType type)
{
	switch (type)
	{
		case WEBKIT_POLICY_DECISION_TYPE_NAVIGATION_ACTION:
			return liferea_webkit_link_clicked (view, decision);
		case WEBKIT_POLICY_DECISION_TYPE_NEW_WINDOW_ACTION:
			return liferea_webkit_new_window_requested(view, decision);

		case WEBKIT_POLICY_DECISION_TYPE_RESPONSE:
		default:
			return FALSE;
	}
	return FALSE;
}

/**
 *  e.g. after a click on javascript:openZoom()
 */
static WebKitWebView*
webkit_create_web_view (WebKitWebView *view, WebKitNavigationAction *action, gpointer user_data)
{
	LifereaHtmlView *htmlview;
	GtkWidget	*container;
	GtkWidget	*htmlwidget;
	GList 		*children;

	htmlview = browser_tabs_add_new (NULL, NULL, TRUE);
	container = liferea_htmlview_get_widget (htmlview);

	/* Ugly lookup of the webview. LifereaHtmlView uses a GtkBox
	   with first a URL bar (sometimes invisble) and the HTML renderer
	   as 2nd child */
	children = gtk_container_get_children (GTK_CONTAINER (container));
	htmlwidget = children->next->data;

	return WEBKIT_WEB_VIEW (htmlwidget);
}

struct FullscreenData {
	GtkWidget *me;
	gboolean visible;
};

/**
 * callback for fullscreen mode gtk_container_foreach()
 */
static void
fullscreen_toggle_widget_visible(GtkWidget *wid, gpointer user_data) {
	gchar* data_label;
	struct FullscreenData *fdata;
	gboolean old_v;
	gchar *propName;

	fdata = user_data;

	// remove shadow of scrolled window
	if (GTK_IS_SCROLLED_WINDOW(wid)) {
		GtkShadowType shadow_type;

		data_label = "fullscreen_shadow_type";
		propName = "shadow-type";

		if (fdata->visible == FALSE) {
			g_object_get(G_OBJECT(wid),
					propName, &shadow_type, NULL);
			g_object_set(G_OBJECT(wid),
					propName, GTK_SHADOW_NONE, NULL);
			g_object_set_data(G_OBJECT(wid), data_label,
					GINT_TO_POINTER(shadow_type));
		} else {
			shadow_type = GPOINTER_TO_INT(g_object_steal_data(
						G_OBJECT(wid), data_label));
			if (shadow_type && shadow_type != GTK_SHADOW_NONE) {
				g_object_set(G_OBJECT(wid),
						propName, shadow_type, NULL);
			}
		}
	}

	if (wid == fdata->me && !GTK_IS_NOTEBOOK(wid)) {
		return;
	}

	data_label = "fullscreen_visible";
	if (GTK_IS_NOTEBOOK(wid)) {
		propName = "show-tabs";
	} else {
		propName = "visible";
	}

	if (fdata->visible == FALSE) {
		g_object_get(G_OBJECT(wid), propName, &old_v, NULL);
		g_object_set(G_OBJECT(wid), propName, FALSE, NULL);
		g_object_set_data(G_OBJECT(wid), data_label,
				GINT_TO_POINTER(old_v));
	} else {
		old_v = GPOINTER_TO_INT(g_object_steal_data(
					G_OBJECT(wid), data_label));
		if (old_v == TRUE) {
			g_object_set(G_OBJECT(wid), propName, TRUE, NULL);
		}
	}
}

/**
 * For fullscreen mode, hide everything except the current webview
 */
static void
fullscreen_toggle_parent_visible(GtkWidget *me, gboolean visible) {
	GtkWidget *parent;
	struct FullscreenData *fdata;
	fdata = (struct FullscreenData *)g_new0(struct FullscreenData, 1);

	// Flag fullscreen status
	g_object_set_data(G_OBJECT(me), "fullscreen_on",
			GINT_TO_POINTER(!visible));

	parent = gtk_widget_get_parent(me);
	fdata->visible = visible;
	while (parent != NULL) {
		fdata->me = me;
		gtk_container_foreach(GTK_CONTAINER(parent),
				(GtkCallback)fullscreen_toggle_widget_visible,
				(gpointer)fdata);
		me = parent;
		parent = gtk_widget_get_parent(me);
	}
	g_free(fdata);
}

/**
 * WebKitWebView "enter-fullscreen" signal
 * Hide all the widget except current WebView
 */
static gboolean
webkit_entering_fullscreen (WebKitWebView *view, gpointer user_data)
{
	fullscreen_toggle_parent_visible(GTK_WIDGET(view), FALSE);
	return FALSE;
}

/**
 * WebKitWebView "leave-fullscreen" signal
 * Restore visibility of hidden widgets
 */
static gboolean
webkit_leaving_fullscreen (WebKitWebView *view, gpointer user_data)
{
	fullscreen_toggle_parent_visible(GTK_WIDGET(view), TRUE);
	return FALSE;
}

// Hack to force webview exit from fullscreen mode on new page
static void
liferea_webkit_load_status_changed (WebKitWebView *view, WebKitLoadEvent event, gpointer user_data)
{
	if (event == WEBKIT_LOAD_STARTED) {
		gboolean isFullscreen;
		isFullscreen = GPOINTER_TO_INT(g_object_steal_data(
					G_OBJECT(view), "fullscreen_on"));
		if (isFullscreen == TRUE) {
			webkit_web_view_run_javascript (view, "document.webkitExitFullscreen();", NULL, NULL, NULL);
		}
	}
}

/**
 * Callback for WebKitWebView::context-menu signal.
 *
 * @view: the object on which the signal is emitted
 * @context_menu: the context menu proposed by WebKit
 * @event: the event that triggered the menu
 * @hit_result: result of hit test at that location.
 *
 * When a context menu is about to be displayed this signal is emitted.
 *
 */
static gboolean
liferea_webkit_on_menu (WebKitWebView 	    *view,
			WebKitContextMenu   *context_menu,
			GdkEvent            *event,
			WebKitHitTestResult *hit_result,
			gpointer             user_data)
{
	LifereaHtmlView			*htmlview;
	GtkMenu 			*menu;
	gchar				*image_uri = NULL;
	gchar				*link_uri = NULL;

	if (webkit_hit_test_result_context_is_link (hit_result))
		g_object_get (hit_result, "link-uri", &link_uri, NULL);
	if (webkit_hit_test_result_context_is_image (hit_result))
		g_object_get (hit_result, "image-uri", &image_uri, NULL);
	if (webkit_hit_test_result_context_is_media (hit_result))
		g_object_get (hit_result, "media-uri", &link_uri, NULL);		/* treat media as normal link */
		
	htmlview = g_object_get_data (G_OBJECT (view), "htmlview");
	
	menu = gtk_menu_new();
	liferea_htmlview_prepare_context_menu (htmlview, menu, link_uri, image_uri);
	gtk_menu_popup(menu, NULL,NULL,NULL,NULL, ((GdkEventButton *)event)->button, ((GdkEventButton*)event)->time);

	return TRUE; // TRUE to ignore WebKit's menu as we make our own menu.
}

/**
 * Initializes WebKit
 *
 * Initializes the WebKit HTML rendering engine. Creates a WebKitWebView.
 */
static GtkWidget *
liferea_webkit_new (LifereaHtmlView *htmlview)
{
	WebKitWebView *view;

	view = WEBKIT_WEB_VIEW (webkit_web_view_new_with_settings (settings));

	/** Pass LifereaHtmlView into the WebKitWebView object */
	g_object_set_data (
		G_OBJECT (view),
		"htmlview",
		htmlview
	);

	/** Connect signal callbacks */
	g_signal_connect (
		view,
		"notify::title",
		G_CALLBACK (liferea_webkit_title_changed),
		view
	);
	g_signal_connect (
		view,
		"mouse-target-changed",
		G_CALLBACK (liferea_webkit_on_mouse_target_changed),
		view
	);
	g_signal_connect (
		view,
		"decide-policy",
		G_CALLBACK (liferea_webkit_decide_policy),
		view
	);
	g_signal_connect (
		view,
		"context-menu",
		G_CALLBACK (liferea_webkit_on_menu),
		view
	);
	g_signal_connect (
		view,
		"notify::uri",
		G_CALLBACK (liferea_webkit_location_changed),
		view
	);
	g_signal_connect (
		view,
		"create",
		G_CALLBACK (webkit_create_web_view),
		view
	);

	g_signal_connect (
		view,
		"enter-fullscreen",
		G_CALLBACK (webkit_entering_fullscreen),
		view
	);

	g_signal_connect (
		view,
		"leave-fullscreen",
		G_CALLBACK (webkit_leaving_fullscreen),
		view
	);
	g_signal_connect (
		view,
		"load-changed",
		G_CALLBACK (liferea_webkit_load_status_changed),
		view
	);
	gtk_widget_show (GTK_WIDGET (view));
	return GTK_WIDGET (view);
}

/**
 * Launch URL
 */
static void
liferea_webkit_launch_url (GtkWidget *webview, const gchar *url)
{
	// FIXME: hack to make URIs like "gnome.org" work
	// https://bugs.webkit.org/show_bug.cgi?id=24195
	gchar *http_url;
	if (!strstr (url, "://")) {
		http_url = g_strdup_printf ("http://%s", url);
	} else {
		http_url = g_strdup (url);
	}

	webkit_web_view_load_uri (
		WEBKIT_WEB_VIEW (webview),
		http_url
	);

	g_free (http_url);
}

/**
 * Change zoom level of the HTML scrollpane
 */
static void
liferea_webkit_change_zoom_level (GtkWidget *webview, gfloat zoom_level)
{
	webkit_web_view_set_zoom_level (WEBKIT_WEB_VIEW (webview), zoom_level);
}

/**
 * Return whether text is selected
 */
static gboolean
liferea_webkit_has_selection (GtkWidget *webview)
{
	/* 
	   Currently (libwebkit-1.0 1.2.0) this doesn't work:

		return webkit_web_view_has_selection (view);

	   So we use *_can_copy_clipboard() as a workaround.
	*/
	// FIXME : Those functions are not available anymore.
	return TRUE;
}

/**
 * Copy selected text to the clipboard
 */
static void
liferea_webkit_copy_selection (GtkWidget *webview)
{
	webkit_web_view_execute_editing_command (WEBKIT_WEB_VIEW (webview), WEBKIT_EDITING_COMMAND_COPY);
}

/**
 * Return current zoom level as a float
 */
static gfloat
liferea_webkit_get_zoom_level (GtkWidget *webview)
{
	return webkit_web_view_get_zoom_level (WEBKIT_WEB_VIEW (webview));
}

/**
 * Scroll page down (via shortcut key)
 *
 * Copied from gtkhtml/gtkhtml.c
 */
// FIXME : WebKitWebView manages its own scrolling, and no longer implements
//         GtkScrollable.
static gboolean
liferea_webkit_scroll_pagedown (GtkWidget *scrollpane)
{
/*
	GtkScrolledWindow *itemview;
	GtkAdjustment *vertical_adjustment;
	gdouble old_value;
	gdouble	new_value;
	gdouble	limit;

	itemview = GTK_SCROLLED_WINDOW (scrollpane);
	g_assert (NULL != itemview);
	vertical_adjustment = gtk_scrolled_window_get_vadjustment (itemview);
	old_value = gtk_adjustment_get_value (vertical_adjustment);
	new_value = old_value + gtk_adjustment_get_page_increment (vertical_adjustment);
	limit = gtk_adjustment_get_upper (vertical_adjustment) - gtk_adjustment_get_page_size (vertical_adjustment);
	if (new_value > limit) {
		new_value = limit;
	}
	gtk_adjustment_set_value (vertical_adjustment, new_value);
	gtk_scrolled_window_set_vadjustment (
		GTK_SCROLLED_WINDOW (itemview),
		vertical_adjustment
	);
	return (new_value > old_value);
*/
  return FALSE;
}

static void
liferea_webkit_set_proxy (const gchar *host, guint port, const gchar *user, const gchar *pwd)
{
	/*
	 * FIXME
	 *  Webkit2 uses global proxy settings :
	 *  https://bugs.webkit.org/show_bug.cgi?id=128674
	 *  https://bugs.webkit.org/show_bug.cgi?id=113663
	 */
}

static struct
htmlviewImpl webkitImpl = {
	.init		= liferea_webkit_init,
	.create		= liferea_webkit_new,
	.write		= liferea_webkit_write_html,
	.launch		= liferea_webkit_launch_url,
	.zoomLevelGet	= liferea_webkit_get_zoom_level,
	.zoomLevelSet	= liferea_webkit_change_zoom_level,
	.hasSelection	= liferea_webkit_has_selection,
	.copySelection	= liferea_webkit_copy_selection,
	.scrollPagedown	= liferea_webkit_scroll_pagedown,
	.setProxy	= liferea_webkit_set_proxy,
	.setOffLine	= NULL // FIXME: blocked on https://bugs.webkit.org/show_bug.cgi?id=18893
};

DECLARE_HTMLVIEW_IMPL (webkitImpl);
