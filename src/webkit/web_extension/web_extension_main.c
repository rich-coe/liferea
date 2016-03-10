#include <webkit2/webkit-web-extension.h>

#include "liferea_web_extension.h"

static LifereaWebExtension *extension = NULL;

G_MODULE_EXPORT void
webkit_web_extension_initialize_with_user_data (WebKitWebExtension *webkit_extension,
						const GVariant *userdata)
{
	extension = liferea_web_extension_get ();
	liferea_web_extension_initialize (extension, webkit_extension, g_variant_get_string (userdata, NULL));
}

static void __attribute__((destructor))
web_extension_shutdown (void)
{
	if (extension)
		g_object_unref (extension);
}
