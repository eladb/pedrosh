/**
 * params-ipv4.h
 *
 * copyright (c) 2006 by embrix.
 * all rights reserved.
 * developed by embrix for corrigent systems ltd.
 *
 * @author eladb@embrix.com
 * @since july 29, 2006
 */

#ifndef __PARAMS_IPV4_H__
#define __PARAMS_IPV4_H__

#include <string>
using namespace std;

#include <libtecla.h>

//
// type definitions
// 

typedef union 
{
	unsigned char as_bytes[4];
	unsigned int  as_int;
} ipv4_t;




//
// function declarations
//

extern void ipv4_register();

extern int ipv4_complete(
	const string& currpath, WordCompletion *cpl, const char *line, int word_start, int word_end);

extern int ipv4_execute(const string& nodepath, const string& argname, string& token);


extern int ipv4_parse(
	const string& input,
	ipv4_t* out_value,
	string* out_err);


#endif /* __PARAMS_IPV4_H__ */
