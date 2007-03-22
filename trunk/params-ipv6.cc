/**
 * params-ipv6.c
 *
 * copyright (c) 2006 by embrix.
 * all rights reserved.
 * developed by embrix for corrigent systems ltd.
 *
 * @author eladb@embrix.com
 * @since july 29, 2006
 */

#include <config.h>     // autoconf

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>
#include <assert.h>
#include <regex.h>
#include <string.h>

// stl
#include <string>
using namespace std;

// pedro
#include "pedrosh-i.h"
#include "utils.h"
#include "debug.h"
#include "params.h"
#include "params-ipv6.h"

//
// defines
// 

#define IPV6_PATTERN "^((([0-9A-Fa-f]{1,4}:){7}[0-9A-Fa-f]{1,4})|(([0-9A-Fa-f]{1,4}:){6}:[0-9A-Fa-f]{1,4})|(([0-9A-Fa-f]{1,4}:){5}:([0-9A-Fa-f]{1,4}:)?[0-9A-Fa-f]{1,4})|(([0-9A-Fa-f]{1,4}:){4}:([0-9A-Fa-f]{1,4}:){0,2}[0-9A-Fa-f]{1,4})|(([0-9A-Fa-f]{1,4}:){3}:([0-9A-Fa-f]{1,4}:){0,3}[0-9A-Fa-f]{1,4})|(([0-9A-Fa-f]{1,4}:){2}:([0-9A-Fa-f]{1,4}:){0,4}[0-9A-Fa-f]{1,4})|(([0-9A-Fa-f]{1,4}:){6}((\\b((25[0-5])|(1[0-9]{2})|(2[0-4][0-9])|([0-9]{1,2}))\\b)\\.){3}(\\b((25[0-5])|(1[0-9]{2})|(2[0-4][0-9])|([0-9]{1,2}))\\b))|(([0-9A-Fa-f]{1,4}:){0,5}:((\\b((25[0-5])|(1[0-9]{2})|(2[0-4][0-9])|([0-9]{1,2}))\\b)\\.){3}(\\b((25[0-5])|(1[0-9]{2})|(2[0-4][0-9])|([0-9]{1,2}))\\b))|(::([0-9A-Fa-f]{1,4}:){0,5}((\\b((25[0-5])|(1[0-9]{2})|(2[0-4][0-9])|([0-9]{1,2}))\\b)\\.){3}(\\b((25[0-5])|(1[0-9]{2})|(2[0-4][0-9])|([0-9]{1,2}))\\b))|([0-9A-Fa-f]{1,4}::([0-9A-Fa-f]{1,4}:){0,5}[0-9A-Fa-f]{1,4})|(::([0-9A-Fa-f]{1,4}:){0,6}[0-9A-Fa-f]{1,4})|(([0-9A-Fa-f]{1,4}:){1,7}:))$"

#define IPV6_MAX_MATCH	256

//
// internal functions
//

static void _string_to_ipv6(const char* str, ipv6_t* out_value);

//
// function definitions
//


/**
 * parses a string as an ip version y string.
 * @param input - the input string.
 * @param out_value - the output ipvy address.
 * @param out_err - if not null, returns a descriptive error.
 * @param err_len - size of out_err.
 * @return 0 upon success, -1 on failure.
 */
int ipv6_parse(
	const string& str,
	ipv6_t* out_value,
	string* out_err)
{
	regmatch_t matches[IPV6_MAX_MATCH];
	int ret;
	regex_t re;
	int reg_ret;
	int i;
	const char* input;
	
	assert(out_value != NULL);
	
	// reset output value and error.
	for (i = 0; i < 8; ++i) out_value->as_shorts[i] = 0;
	if (out_err) out_err->clear();

	input = str.c_str();
	
	// compile the regular expression.
	reg_ret = regcomp(&re, IPV6_PATTERN, REG_EXTENDED);
	if (reg_ret != 0)
	{
		if (out_err) stl_sprintf(out_err, "unable to parse ipv6 address. invalid regular expression used");
		ret = -1;
		goto cleanup;
	}

	// execute the regular expression on the input string.
	reg_ret = regexec(&re, input, IPV6_MAX_MATCH, matches, 0);
	if (reg_ret == REG_NOMATCH)
	{
		if (out_err) stl_sprintf(out_err, "'%s': invalid ipv6 address", input);
		ret = -1;
		goto cleanup;
	}

	_string_to_ipv6(input, out_value);
	
	ret = 0;
	
cleanup:
	return ret;
}

//
// internal functions definitions
// 

/**
 * converts a string to ipv6 type. assume string is a valid ipv6 string.
 */
static void _string_to_ipv6(const char* str, ipv6_t* out_value)
{
	char* lasts;
	char* tok;
	char* input_copy;
	char* p;
	char* last_tok_end;
	long parts[9];
	int part;
	int i;
	int comp_size;
	int comp_start = -1;

	assert(str != NULL);
	assert(out_value != NULL);
				
	input_copy = (char*) malloc(strlen(str) + 1);
	strcpy(input_copy, str);
	
	p = input_copy;
	
	// reset parts
	for (part = 0; part < 6; ++part) parts[part] = -1;

	part = 0;
	last_tok_end = p - 1;
	while ((tok = strtok_r(p, ":", &lasts)) != NULL)
	{
		int sep_len;
		
		p = NULL;
		sep_len = tok - last_tok_end;

		// if we had '::', we are putting a marker to indicate compression.
		if (sep_len > 1)
		{
			parts[part] = -2;
			comp_start = part;
			part++;
		}
		
		// adding the part.
		parts[part] = strtol(tok, NULL, 16);
		part++;
		
		last_tok_end = tok + strlen(tok);
	}

	// calculate compression size.
	if (comp_start != -1)	comp_size = 9 - part;
	else comp_size = -1;
	
	// store parts in out_value, while storing 0x0000 in compression part.
	for (part = 0, i = 0; i < 8; ++i)
	{
		if (i >= comp_start && i < comp_start + comp_size)
			out_value->as_shorts[i] = 0x0000;
		else
			out_value->as_shorts[i] = parts[part++];

		if (i == comp_start) part++;
	}
	
	free(input_copy);
}

/**
 * completes an ipv6 argument.
 */
int ipv6_complete(
 const string& currpath, WordCompletion *cpl, const char *line, int word_start, int word_end)
{
	// we don't complete this token type.
	// we do check, however, if it contains an illegal value.
	// if it does - simply print an error message.
	// extract last token from line.
	
  string token(line + word_start, word_end - word_start);
	trace("currpath=<%s> line=<%s> token=<%s>\n", currpath.c_str(), line, token.c_str());

	if (token == "")
		return 0;

	string err;
	ipv6_t out_value;
	int    ret = ipv6_parse(token, &out_value, &err);

	// print error, if we had one.
	if (ret != 0)
		fprintf(stderr, "warning: %s\n", err.c_str());
	
	return ret;
}

/**
 * called when the user executes a command with an ipv6 argument.
 */
int ipv6_execute(const string& nodepath, const string& argname, string& token)
{
	int ret;
	string err;

	trace("nodepath=%s, token=%s\n", nodepath.c_str(), token.c_str());

	ipv6_t out_value;
	ret = ipv6_parse(token, &out_value, &err);

	// print error, if we had one.
	if (ret != 0)
		fprintf(stderr, "error: %s: %s\n", argname.c_str(), err.c_str());
	
	return ret ? PEDRO_EXEC_ERROR : PEDRO_EXEC_SUCCESS;
}

/**
 * registers the ipv6 type.
 */
void ipv6_register()
{
	params_register("ipv6", ipv6_complete, ipv6_execute);	
}
