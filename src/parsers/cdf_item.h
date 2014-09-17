/**
 * @file cdf_item.h CDF item tag parsing 
 *
 * Copyright (C) 2003-2006 Lars Windolf <lars.windolf@gmx.de>
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

#ifndef _CDF_ITEM_H
#define _CDF_ITEM_H

#include <libxml/tree.h>

#include "cdf_channel.h"

itemPtr parseCDFItem(feedParserCtxtPtr ctxt, xmlNodePtr cur, CDFChannelPtr cp);

#endif
