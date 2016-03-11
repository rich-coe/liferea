#ifndef _LIFEREA_WEB_EXTENSION_H
#define _LIFEREA_WEB_EXTENSION_H

#include <glib-object.h>
#include <webkit2/webkit-web-extension.h>

#define LIFEREA_TYPE_WEB_EXTENSION liferea_web_extension_get_type ()

G_DECLARE_FINAL_TYPE (LifereaWebExtension, liferea_web_extension, LIFEREA, WEB_EXTENSION, GObject)

LifereaWebExtension* liferea_web_extension_get (void);
void liferea_web_extension_initialize (LifereaWebExtension *extension, WebKitWebExtension *webkit_extension,  const gchar *server_address);

#endif
