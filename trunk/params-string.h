/**
 * params-string.h
 *
 * copyright (c) 2006 by embrix.
 * all rights reserved.
 * developed by embrix for corrigent systems ltd.
 *
 * @author eladb@embrix.com
 * @since july 29, 2006
 */

#ifndef __PARAMS_STRING_H__
#define __PARAMS_STRING_H__

#include <string>
using namespace std;

#include <libtecla.h>

//
// function declarations
//

extern void string_register();

extern int string_complete(
	const string& currpath, WordCompletion *cpl, const char *line, int word_start, int word_end);

extern int string_execute(const string& nodepath, const string& argname, string& token);

extern int string_parse(
	const string& str,
	string* out_value,
	int use_min, int min,
	int use_max, int max,
	string* out_err);


#endif /* __PARAMS_STRING_H__ */
