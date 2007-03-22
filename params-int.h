/**
 * params-int.h
 *
 * copyright (c) 2006 by embrix.
 * all rights reserved.
 * developed by embrix for corrigent systems ltd.
 *
 * @author eladb@embrix.com
 * @since july 29, 2006
 */

#ifndef __PARAMS_INT_H__
#define __PARAMS_INT_H__

#include <string>
using namespace std;

#include <libtecla.h>

//
// function declarations
//

extern void int_register();

extern int int_complete(
	const string& currpath, WordCompletion *cpl, const char *line, int word_start, int word_end);

extern int int_execute(const string& nodepath, const string& argname, string& token);

extern int int_parse(const string& str, long* out_value, 
										 int use_min, long min, int use_max, long max,
										 string* out_err);

#endif /* __PARAMS_INT_H__ */
