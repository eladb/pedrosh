/**
 * params-ipv6.h
 *
 * copyright (c) 2006 by embrix.
 * all rights reserved.
 * developed by embrix for corrigent systems ltd.
 *
 * @author eladb@embrix.com
 * @since july 29, 2006
 */

#ifndef __PARAMS_IPV6_H__
#define __PARAMS_IPV6_H__

#include <string>
using namespace std;

//
// type definitions
// 
extern void ipv6_register();

extern int ipv6_complete(
	const string& currpath, WordCompletion *cpl, const char *line, int word_start, int word_end);

extern int ipv6_execute(const string& nodepath, const string& argname, string& token);

typedef union
{
  unsigned short as_shorts[8];
	unsigned char as_bytes[16];
} ipv6_t;

//
// function declarations
//

extern int ipv6_parse(
	const string& input,
	ipv6_t* out_value,
	string* out_err);

#endif /* __PARAMS_IPV6_H__ */
