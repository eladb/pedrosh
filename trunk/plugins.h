/**
 * plugins.h
 *
 * copyright (c) 2006 by embrix.
 * all rights reserved.
 * developed by embrix for corrigent systems ltd.
 *
 * @author eladb@embrix.com
 * @since july 18, 2006
 */

#ifndef __PLUGINS_H__
#define __PLUGINS_H__

//
// defines
// 

#define MAX_PLUGINS 50
#define MAX_PLUGIN_NAME	100

//
// type definitions
//

typedef int (* plugin_handler_t)(int argc, char* argv[]);

//
// function declarataions
// 

extern void plugins_set_default_handler(plugin_handler_t handler);
extern int plugins_register(const char* name, plugin_handler_t handler);
extern int plugins_execute(int argc, char* argv[]);

#endif /* __PLUGINS_H__ */
