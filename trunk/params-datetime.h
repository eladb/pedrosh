/**
 * params-datetime.h
 *
 * copyright (c) 2006 by embrix.
 * all rights reserved.
 * developed by embrix for corrigent systems ltd.
 *
 * @author eladb@embrix.com
 * @since july 29, 2006
 */

#ifndef __PARAMS_DATETIME_H__
#define __PARAMS_DATETIME_H__

#include <time.h>

#include <string>
using namespace std;

//
// function declarations
//

extern void datetime_register();

extern int datetime_complete(
	const string& currpath, WordCompletion *cpl, const char *line, int word_start, int word_end);

extern int datetime_execute(const string& nodepath, const string& argname, string& token);

extern int datetime_parse(
	const string& input,
	const string& format,
	struct tm* out_value,
	string* out_err);

#endif /* __PARAMS_DATETIME_H__ */
