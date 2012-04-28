/*
 * Bit.ly URL Shortener Plugin for libpurple
 *
 * http://github.com/johnbellone/purple-bitly
 *
 * Copyright (C) 2009, 2010 John Bellone <jb@thunkbrightly.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02111-1301, USA.
 */
#import "BitlyWrapper.h"

@implementation BitlyWrapper
- (void) installLibpurplePlugin
{
}

- (void) loadLibpurplePlugin
{
  purple_init_bitlyurlshort_plugin();
}
@end