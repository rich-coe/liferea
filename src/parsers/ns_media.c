/**
 * @file ns_media.c  Yahoo media namespace support
 *
 * Copyright (C) 2007-2010 Lars Windolf <lars.windolf@gmx.de>
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

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <string.h>

#include "ns_media.h"
#include "enclosure.h"
#include "xml.h"

/* a namespace documentation can be found at 
   http://search.yahoo.com/mrss   
*/

static void
parse_item_tag (feedParserCtxtPtr ctxt, xmlNodePtr cur)
{
	gchar *tmp, *tmp2;
	/*
	   Maximual definition could look like this:
	   
        	<media:content 
        	       url="http://www.foo.com/movie.mov" 
        	       fileSize="12216320" 
        	       type="video/quicktime"
        	       medium="video"
        	       isDefault="true" 
        	       expression="full" 
        	       bitrate="128" 
        	       framerate="25"
        	       samplingrate="44.1"
        	       channels="2"
        	       duration="185" 
        	       height="200"
        	       width="300" 
        	       lang="en" />
		       
	   (example quoted from specification)
	*/
  	if (xmlStrcmp(cur->name, BAD_CAST"content")) {
		tmp = xml_get_attribute (cur, "url");
		if (!tmp)
			return;

		/* the following code is duplicated from rss_item.c! */
		const gchar *feedURL = subscription_get_homepage (ctxt->subscription);
	
		gchar *type = xml_get_attribute (cur, "type");
		gchar *lengthStr = xml_get_attribute (cur, "length");
		gchar *medium = xml_get_attribute (cur, "medium");
		gssize length = 0;
		if (lengthStr)
			length = atol (lengthStr);
	
		if ((strstr (tmp, "://") == NULL) && feedURL && (feedURL[0] != '|') &&
		    (strstr (feedURL, "://") != NULL)) {
			/* add base URL if necessary and possible */
			 tmp2 = g_strdup_printf ("%s/%s", feedURL, tmp);
			 g_free (tmp);
			 tmp = tmp2;
		}
	
		/* gravatars are often supplied as media:content with medium='image'
		   so we treat do not treat such occurences as enclosures */
		if (medium && !strcmp (medium, "image") && strstr (tmp, "www.gravatar.com")) {
			metadata_list_set (&(ctxt->item->metadata), "gravatar", tmp);
		} else {
			/* Never add enclosures for images already contained in the description */
			if (!(ctxt->item->description && strstr (ctxt->item->description, tmp))) {
				gchar *encl_string = enclosure_values_to_string (tmp, type, length, FALSE /* not yet downloaded */);
				ctxt->item->metadata = metadata_list_append (ctxt->item->metadata, "enclosure", encl_string);
				g_free (encl_string);

				ctxt->item->hasEnclosure = TRUE;
			}
		}
		g_free (tmp);
		g_free (type);
		g_free (medium);
		g_free (lengthStr);
	}
	
	// FIXME: should we support media:player too?
}

static void
ns_media_register_ns (NsHandler *nsh, GHashTable *prefixhash, GHashTable *urihash)
{
	g_hash_table_insert (prefixhash, "media", nsh);
	g_hash_table_insert (urihash, "http://search.yahoo.com/mrss", nsh);
}

NsHandler *
ns_media_get_handler (void)
{
	NsHandler 	*nsh;
	
	nsh = g_new0 (NsHandler, 1);
	nsh->prefix		= "media";
	nsh->registerNs		= ns_media_register_ns;
	nsh->parseItemTag	= parse_item_tag;

	return nsh;
}
