/**
 * @file cdf_channel.h  CDF channel parsing
 *
 * Copyright (C) 2003-2008 Lars Windolf <lars.windolf@gmx.de>
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

#ifndef _CDF_CHANNEL_H
#define _CDF_CHANNEL_H

#include <time.h>

#include "feed_parser.h"

typedef struct CDFChannel {
	/* other information */
	time_t		time;		/* last feed build/creation time */	
} *CDFChannelPtr;

feedHandlerPtr cdf_init_feed_handler(void);

#endif
