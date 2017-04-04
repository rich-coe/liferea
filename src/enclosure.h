/*
 * @file enclosure.h enclosure/podcast support
 *
 * Copyright (C) 2007-2012 Lars Windolf <lars.windolf@gmx.de>
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

#ifndef _ENCLOSURE_H
#define _ENCLOSURE_H

#include <glib.h>

/* structure describing the preferences for a MIME type or file extension */
typedef struct encType {
	gchar		*mime;		/*<< either mime or extension is set */
	gchar		*extension;
	gchar		*cmd;		/*<< the command to launch the enclosure type */
	gboolean	permanent;	/*<< if TRUE definition is deleted after opening and
					     not added to the permanent list of type configs */
} *encTypePtr;

/* structure describing an enclosure and its states */
typedef struct enclosure {
	gchar		*url;		/*<< enclosure download URI (absolute path) */
	gchar		*mime;		/*<< enclosure MIME type (optional, can be NULL) */
	gssize		size;		/*<< enclosure size (optional, can be 0, but also -1) */
	gboolean	downloaded;	/*<< flag indicating we have downloaded the enclosure */
} *enclosurePtr;

/**
 * enclosure_from_string: (skip)
 * @str:	the enclosure description
 *
 * Parses enclosure description.
 *
 * Returns: (transfer full): new enclosure structure (to be free'd using enclosure_free)
 */
enclosurePtr enclosure_from_string (const gchar *str);

/**
 * enclosure_values_to_string:
 * @url:		the enclosure URL
 * @mime:		the MIME type (optional, can be NULL)
 * @size:	  	the enclosure size (optional, can be 0, and also -1)
 * @downloaded:	downloading state (TRUE=downloaded)
 *
 * Serialize enclosure infos to string.
 *
 * Returns: (transfer full): new string (to be free'd using g_free)
 */
gchar * enclosure_values_to_string (const gchar *url, const gchar *mime, gssize size, gboolean downloaded);

/**
 * enclosure_to_string: (skip)
 * @enclosure:		the enclosure
 *
 * Serialize enclosure to string.
 *
 * Returns: (transfer full): new string (to be free'd using g_free)
 */
gchar * enclosure_to_string (const enclosurePtr enclosure);

/**
 * enclosure_get_url:
 * @str:	enclosure string to parse
 *
 * Get URL from enclosure string
 *
 * Returns: (transfer full): URL string, free after use
 */
gchar * enclosure_get_url (const gchar *str);

/**
 * enclosure_get_mime:
 * @str:	enclosure string to parse
 *
 * Get MIME type from enclosure string
 *
 * Returns: (transfer full): MIME type string, free after use
 */
gchar * enclosure_get_mime (const gchar *str);

/**
 * enclosure_free: (skip)
 * @enclosure:	the enclosure
 *
 * Free all memory associated with the enclosure.
 */
void enclosure_free (enclosurePtr enclosure);

/**
 * enclosure_mime_types_get: (skip)
 *
 * Returns all configured enclosure types.
 *
 * Returns: (transfer none): list of encType structures
 */
const GSList * enclosure_mime_types_get (void);

/**
 * enclosure_mime_type_add: (skip)
 * @type:	the new definition
 *
 * Adds a new MIME type handling definition.
 */
void enclosure_mime_type_add (encTypePtr type);

/**
 * enclosure_mime_type_remove: (skip)
 * @type:	the definition to remove
 *
 * Removes an existing MIME type handling definition.
 * The definition will be free'd by this function.
 */
void enclosure_mime_type_remove (encTypePtr type);

/**
 * enclosure_mime_types_save: (skip)
 *
 * Save all MIME type definitions.
 */
void enclosure_mime_types_save (void);

/**
 * enclosure_download: (skip)
 * @type:		ULL or pointer to type structure
 * @url:		valid HTTP URL
 * @interactive:	TRUE if triggered by user interaction
 *
 * Downloads a given enclosure URL into a file
 */
void enclosure_download (encTypePtr type, const gchar *url, gboolean interactive);

#endif
