#include <webkit2/webkit-web-extension.h>
#define WEBKIT_DOM_USE_UNSTABLE_API
#include <webkitdom/WebKitDOMDOMWindowUnstable.h>

#include "liferea_web_extension.h"

struct _LifereaWebExtension {
	GObject 		parent;

	GDBusConnection 	*connection;
	WebKitWebExtension 	*webkit_extension;
	gboolean 		initialized;
};

G_DEFINE_TYPE (LifereaWebExtension, liferea_web_extension, G_TYPE_OBJECT)

static const char introspection_xml[] =
  "<node>"
  " <interface name='net.sf.liferea.WebExtension'>"
  "  <method name='ScrollPageDown'>"
  "   <arg type='t' name='page_id' direction='in'/>"
  "   <arg type='b' name='scrolled' direction='out'/>"
  "  </method>"
  "  <method name='HasSelection'>"
  "   <arg type='t' name='page_id' direction='in'/>"
  "   <arg type='b' name='has_selection' direction='out'/>"
  "  </method>"
  " </interface>"
  "</node>";

static void
liferea_web_extension_dispose (GObject *object)
{
	LifereaWebExtension *extension = LIFEREA_WEB_EXTENSION (object);

	g_clear_object (&extension->connection);
	g_clear_object (&extension->webkit_extension);
}

static void
liferea_web_extension_class_init (LifereaWebExtensionClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	object_class->dispose = liferea_web_extension_dispose;
}

static void
liferea_web_extension_init (LifereaWebExtension *self)
{
	self->webkit_extension = NULL;
	self->connection = NULL;
	self->initialized = FALSE;
}

static WebKitDOMDOMWindow*
liferea_web_extension_get_dom_window (LifereaWebExtension *self, guint64 page_id)
{
	WebKitWebPage *page;
	WebKitDOMDocument *document;
	WebKitDOMDOMWindow *window;

	page = webkit_web_extension_get_page (self->webkit_extension, page_id);
	document = webkit_web_page_get_dom_document (page);
	window = webkit_dom_document_get_default_view (document);

	return window;
}

/**
 * \returns TRUE if scrolling happened, FALSE if the end was reached
 */
static gboolean
liferea_web_extension_scroll_page_down (LifereaWebExtension *self, guint64 page_id)
{
	glong old_scroll_y, new_scroll_y, increment;
	WebKitDOMDOMWindow *window;

	window = liferea_web_extension_get_dom_window (self, page_id);

	old_scroll_y = webkit_dom_dom_window_get_scroll_y (window);
	increment = webkit_dom_dom_window_get_inner_height (window);
	webkit_dom_dom_window_scroll_by (window, 0, increment);
	new_scroll_y = webkit_dom_dom_window_get_scroll_y (window);

	return (new_scroll_y > old_scroll_y);
}

/**
 * \returns TRUE if text is selected.
 */
static gboolean
liferea_web_extension_has_selection (LifereaWebExtension *self, guint64 page_id)
{
	WebKitDOMDOMWindow *window;
	WebKitDOMDOMSelection *selection;
	gboolean is_collapsed;

	window = liferea_web_extension_get_dom_window (self, page_id);
	selection = webkit_dom_dom_window_get_selection (window);
	g_object_get (selection, "is_collapsed", &is_collapsed, NULL);
	g_object_unref (selection);

	return (!is_collapsed);
}

static gboolean
on_authorize_authenticated_peer (GDBusAuthObserver 	*observer,
				 GIOStream		*stream,
				 GCredentials		*credentials,
				 gpointer		extension)
{
	gboolean authorized = FALSE;
	GCredentials *own_credentials = NULL;
	GError *error = NULL;

	if (!credentials) {
		g_warning ("No credentials received from Liferea.\n");
		return FALSE;
	}

	own_credentials = g_credentials_new ();

	if (g_credentials_is_same_user (credentials, own_credentials, &error)) {
		authorized = TRUE;
	} else {
		g_warning ("Error authorizing connection to Liferea : %s\n", error->message);
		g_error_free (error);
	}
	g_object_unref (own_credentials);

	return authorized;
}

static void
handle_dbus_method_call (GDBusConnection 	*connection,
			 const gchar 		*sender,
			 const gchar 		*object_path,
			 const gchar 		*interface_name,
			 const gchar 		*method_name,
			 GVariant 		*parameters,
			 GDBusMethodInvocation 	*invocation,
			 gpointer 		user_data)
{
	if (g_strcmp0 (method_name, "ScrollPageDown") == 0) {
		guint64 page_id;
		gboolean scrolled;

		g_variant_get (parameters, "(t)", &page_id);
		scrolled = liferea_web_extension_scroll_page_down (LIFEREA_WEB_EXTENSION (user_data), page_id);
		g_dbus_method_invocation_return_value (invocation, g_variant_new ("(b)", scrolled));
	} else if (g_strcmp0 (method_name, "HasSelection") == 0) {
		guint64 page_id;
		gboolean has_selection;

		g_variant_get (parameters, "(t)", &page_id);
		has_selection = liferea_web_extension_has_selection (LIFEREA_WEB_EXTENSION (user_data), page_id);
		g_dbus_method_invocation_return_value (invocation, g_variant_new ("(b)", has_selection));
	}
}

static const GDBusInterfaceVTable interface_vtable = {
	handle_dbus_method_call,
	NULL,
	NULL
};

static void
on_dbus_connection_created (GObject 		*source_object,
			    GAsyncResult 	*result,
			    gpointer	 	user_data)
{
	GDBusNodeInfo *introspection_data = NULL;
	GDBusConnection *connection = NULL;
	guint registration_id = 0;
	GError *error = NULL;
	LifereaWebExtension *extension = LIFEREA_WEB_EXTENSION (user_data);

	introspection_data = g_dbus_node_info_new_for_xml (introspection_xml, NULL);

	connection = g_dbus_connection_new_for_address_finish (result, &error);
	if (error) {
		g_warning ("Extension failed to connect : %s", error->message);
		g_error_free (error);
		return;
	}

	registration_id = g_dbus_connection_register_object (connection,
		"/net/sf/liferea/WebExtension",
		introspection_data->interfaces[0],
		&interface_vtable,
		extension,
		NULL,
		&error);

	g_dbus_node_info_unref (introspection_data);
	if (!registration_id) {
		g_warning ("Failed to register web extension object: %s\n", error->message);
		g_error_free (error);
		g_object_unref (connection);
		return;
	}

	extension->connection = connection;
}

static gpointer
liferea_web_extension_new (gpointer data)
{
	return g_object_new (LIFEREA_TYPE_WEB_EXTENSION, NULL);
}

LifereaWebExtension *
liferea_web_extension_get (void)
{
	static GOnce init_once = G_ONCE_INIT;

	g_once (&init_once, liferea_web_extension_new, NULL);

	return init_once.retval;
}

void
liferea_web_extension_initialize (LifereaWebExtension 	*extension,
				  WebKitWebExtension 	*webkit_extension,
				  const gchar 		*server_address)
{

	if (extension->initialized)
		return;

	GDBusAuthObserver	*observer;

	extension->initialized = TRUE;
	extension->webkit_extension = g_object_ref (webkit_extension);

	observer = g_dbus_auth_observer_new ();

	g_signal_connect (
		observer,
		"authorize-authenticated-peer",
		G_CALLBACK (on_authorize_authenticated_peer),
		extension);

	g_dbus_connection_new_for_address (
		server_address,
		G_DBUS_CONNECTION_FLAGS_AUTHENTICATION_CLIENT,
		observer,
		NULL,
		(GAsyncReadyCallback)on_dbus_connection_created,
		extension);

	g_object_unref (observer);
}
