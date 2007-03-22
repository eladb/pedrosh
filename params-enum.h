/**
 * params-enum.h
 *
 * copyright (c) 2006 by embrix.
 * all rights reserved.
 * developed by embrix for corrigent systems ltd.
 *
 * @author eladb@embrix.com
 * @since july 29, 2006
 */

#ifndef __PARAMS_ENUM_H__
#define __PARAMS_ENUM_H__

#include <vector>
#include <string>
using namespace std;

#include <libtecla.h>

//
// function declarations
//

extern void enum_register();

extern int enum_complete(
	const string& currpath, WordCompletion *cpl, const char *line, int word_start, int word_end);

extern int enum_execute(const string& nodepath, const string& argname, string& token);

extern int enum_parse(
  const string& input,
	const vector<string>& values,
	int is_case_sensitive,
	int* out_index,
	string* out_err);

#endif /* __PARAMS_ENUM_H__ */
