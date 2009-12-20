/*
 * Bit.ly URL Shortener for libpurple
 *
 * Copyright (C) 2009, John Bellone <jb@thunkbrightly.com>
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
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#ifndef PURPLE_PLUGINS
# define PURPLE_PLUGINS
#endif

#include "internal.h"
#include "plugin.h"
#include "pluginpref.h"
#include "prefs.h"
#include "version.h"

#include <time.h>

#include <cmds.h>
#include <conversation.h>
#include <signals.h>
#include <debug.h>

#include <stdio.h>
#include <stdlib.h>

#include <curl/curl.h>
#include <curl/types.h>
#include <curl/easy.h>

#include <glib.h>

static PurpleCmdId bitly_cmd_id;
static GString * bitly_url;
static CURL * curl_handle;

static PurplePluginPrefFrame *
get_plugin_pref_frame (PurplePlugin * plugin) {
	PurplePluginPrefFrame * frame = purple_plugin_pref_frame_new();
	PurplePluginPref * pref;

	/* Bit.ly Account */
	pref = purple_plugin_pref_new_with_name_and_label ("/plugins/core/bitly_urlshort/account", "Account:");
	purple_plugin_pref_frame_add (frame, pref);

	/* Bit.ly API Key */
	pref = purple_plugin_pref_new_with_name_and_label ("/plugins/core/bitly_urlshort/apikey", "API Key:");
	purple_plugin_pref_frame_add (frame, pref);
	
	return frame;
}

static size_t
curl_write_cb (char * data, size_t size, size_t nmemb, void * buffer) {
	GString * string = (GString *)buffer;
	g_string_append (string, data);
	return size * nmemb;						
}

static GString *
process_url (const char * login, const char * apiKey, GString * longUrl) {
	char * url = NULL, * end = NULL;
	
	g_string_printf (bitly_url, "http://api.bit.ly/shorten?version=2.0.1&login=%s&apiKey=%s&longUrl=%s",
						  login, apiKey, longUrl->str);

	/* Use libcurl to do all the fancy HTTP requests that we need in order for this bad boy
	 * to work correctly. */
	curl_easy_setopt (curl_handle, CURLOPT_USERAGENT, "purple-bitly/0.1");
	curl_easy_setopt (curl_handle, CURLOPT_URL, bitly_url->str);
	curl_easy_setopt (curl_handle, CURLOPT_WRITEFUNCTION, curl_write_cb);
	curl_easy_setopt (curl_handle, CURLOPT_WRITEDATA, (void *)longUrl);
	curl_easy_perform (curl_handle);
	curl_easy_cleanup (curl_handle);
	
	/* Extract the http://bit.ly URL from the JSON returned back. Add four characters for the
	 * offset, e.g. "shortUrl": "http://bit.ly/x1059" and then find position of the quote at end. */
	if (NULL != (url = strstr (longUrl->str, "shortUrl"))) {
		if (NULL != (url = strstr (url, "http://"))) {
			g_string_assign (longUrl, url);

			end = strstr (longUrl->str, "\"");

			g_string_truncate (longUrl, (end - longUrl->str));
		}
	}

	return longUrl;
}


static PurpleCmdRet
purple_cmd_bitly (PurpleConversation * conv, const char * cmd, char ** args, char * error, void * data) {
	PurpleCmdStatus ret;
	GString * str = g_string_new (args[0]);
	char * newcmd = NULL;

	if (args[0]) {
		const char * login = purple_prefs_get_string ("/plugins/core/bitly_urlshort/account");
		const char * apiKey = purple_prefs_get_string ("/plugins/core/bitly_urlshort/apikey");
		
		str = process_url (login, apiKey, str);

		newcmd = g_strdup_printf ("say %s", str->str);
	
		ret = purple_cmd_do_command (conv, newcmd, newcmd, &error);

		free (newcmd);
		g_string_free (str, TRUE);
	}
	return ret;
}

static gboolean
plugin_load (PurplePlugin * plugin) {
		const char * help = _("bitly: replaces the argument URL with one through the bit.ly service");

		curl_global_init (CURL_GLOBAL_ALL);
		
		bitly_url = g_string_new("");
		curl_handle = curl_easy_init();
		bitly_cmd_id = purple_cmd_register ("bitly", "wws", PURPLE_CMD_P_PLUGIN,
														PURPLE_CMD_FLAG_IM | PURPLE_CMD_FLAG_CHAT |
														PURPLE_CMD_FLAG_ALLOW_WRONG_ARGS,
														NULL, PURPLE_CMD_FUNC (purple_cmd_bitly), help, NULL);
		return TRUE;
}

static gboolean
plugin_unload (PurplePlugin * plugin) {
	purple_cmd_unregister (bitly_cmd_id);

	curl_global_cleanup();
	
	free (curl_handle);
	g_string_free (bitly_url, TRUE);
	
	return TRUE;
}

static PurplePluginUiInfo prefs_info = {
	get_plugin_pref_frame,
	0,
	NULL,
	/* Padding */
	NULL,
	NULL,
	NULL,
	NULL
};

static PurplePluginInfo info = {
	PURPLE_PLUGIN_MAGIC,
	PURPLE_MAJOR_VERSION,
	PURPLE_MINOR_VERSION,
	PURPLE_PLUGIN_STANDARD,
	NULL,
	0,
	NULL,
	PURPLE_PRIORITY_DEFAULT,
	"core-bitly_urlshort",
	"Bit.ly URL Shortener",
	DISPLAY_VERSION,
	"URL shortener plugin which utilizes the bit.ly API",
	"URL shortener plugin which utilizes the bit.ly API",
	"John Bellone <jb@thunkbrightly.com>",
	"http://thunkbrightly.com/",

	plugin_load,                                                
	plugin_unload,
	NULL,

	NULL,
	NULL,
	&prefs_info,
	NULL,
	/* Padding */
	NULL,
	NULL,
	NULL,
	NULL
};

static void
init_plugin (PurplePlugin * plugin) {
	purple_prefs_add_none ("/plugins/core/bitly_urlshort");
	purple_prefs_add_string ("/plugins/core/bitly_urlshort/account", "");
	purple_prefs_add_string ("/plugins/core/bitly_urlshort/apikey", "");
}

PURPLE_INIT_PLUGIN (bitlyurlshort, init_plugin, info)
