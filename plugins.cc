/**
 * plugins.c
 * handles the execution of commands, along with the registration of plugins. 
 *
 * copyright (c) 2006 by embrix.
 * all rights reserved.
 * developed by embrix for corrigent systems ltd.
 *
 * @author eladb@embrix.com
 * @since july 18, 2006
 */

#include <config.h>     // autoconf

#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <strings.h>

#include "plugins.h"

// mgmt_path
#include "pedrosh-i.h"
#include "debug.h"

//
// type definitions
// 

typedef struct
{
	const char* name;
	plugin_handler_t handler;
} _plugin_handler_reg_t;


//
// static variables
//

static _plugin_handler_reg_t _plugins[MAX_PLUGINS];
static int _plugins_count = 0;
static plugin_handler_t _default_plugin = NULL;

//
// function definitions
//

extern void plugins_set_default_handler(plugin_handler_t default_plugin)
{
	_default_plugin = default_plugin;
}

/**
 * registers a pedro shell plugin.
 * @param name - the name of the plugin.
 * @param handler - the handler that should be executed when a command with .plugin=name is executed.
 * @return the index of the plugin or -1 if plugin could not register.
 */
extern int plugins_register(const char* name, plugin_handler_t handler)
{
	if (_plugins_count == MAX_PLUGINS)
	{
		return -1;
	}
				
	_plugins[_plugins_count].name = name;
	_plugins[_plugins_count].handler = handler;
	_plugins_count++;
	
	return (_plugins_count - 1);	
}

/**
 * looks for the file .plugin under the command's path and returns a handler function
 * that should be invoked to execute this command.
 * @param command_path - the path of the command, relative to the root of the management filesystem (argv[0]).
 * @return a command handler function pointer to NULL if there's none.
 */
plugin_handler_t _find_handler(const char* command_path)
{
	char plugin_defs_fn[MAX_PATH+1];
	char plugin_name[MAX_PLUGIN_NAME+1];
	FILE* plugin_defs_file;
	plugin_handler_t handler = _default_plugin;
	int found = 0;
	int i;
				
	snprintf(plugin_defs_fn, MAX_PATH, "%s/%s/%s", g_mgmt_path.c_str(),
		       command_path, PEDRO_META_PLUGIN);

	plugin_defs_file = fopen(plugin_defs_fn, "r");
	if (plugin_defs_file == NULL)
	{
		trace("no plugin definition file (%s). using default plugin.\n", plugin_defs_fn);
		goto cleanup;
	}
	
	if (fgets(plugin_name, MAX_PLUGIN_NAME, plugin_defs_file) == NULL)
	{
		trace("unable to retrieve plugin name from '%s' for command <%s>. using default plugin.\n",
			plugin_defs_fn, command_path);
		goto cleanup;	
	}

	// trim ending newline.
	if (plugin_name[strlen(plugin_name) - 1] == '\n')
	{
		plugin_name[strlen(plugin_name) - 1] = '\0';
	}

	trace("plugin-name=<%s>\n", plugin_name);

	// look for the registration of the plugin.
	for (i = 0; i < _plugins_count; ++i)
	{
		if (strcasecmp(_plugins[i].name, plugin_name) == 0)
		{
			handler = _plugins[i].handler;
			found = 1;
			break;
		}
	}	

	if (!found)
	{
		trace("unable to find registrant for plugin '%s'. using default plugin.\n", plugin_name);
	}
	
cleanup:
	if (plugin_defs_file != NULL) fclose(plugin_defs_file);
	return handler;	
}

/**
 * executes a command.
 * @param argc the number of arguments.
 * @param argv[] the arguments passed along to the command, with the command's path as the first member.
 * @return 0 if the command succeeded or -1 if it had failed.
 */
extern int plugins_execute(int argc, char* argv[])
{
	plugin_handler_t handler;

	// at least one argument (argv[0] should contain the command's path).
	assert(argc > 0);
	
	handler = _find_handler(argv[0]);

	if (handler == NULL)
	{
		fprintf(stderr, "error: unable to execute <%s>, no plugin specified.\n",
			argv[0]);
		
		return -1;
	}

	return handler(argc, argv);
}
