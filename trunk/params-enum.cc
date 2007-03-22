/**
 * params-enum.c
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
#include <string.h>
#include <dirent.h>

// stl
#include <string>
#include <vector>
#include <map>
#include <functional>
#include <sstream>    // stringstream, istringstream, ostringstream
using namespace std;

// libtecla
#include <libtecla.h>

// pedro
#include "pedrosh-i.h"
#include "debug.h"
#include "params.h"
#include "utils.h"

#include "params-enum.h"

//
// function declarations
// 

void _enum_register();

//
// function definitions
//

/**
 * registers the enum type.
 */
void enum_register()
{
	params_register("enum", enum_complete, enum_execute);	
}

// enum values are stored in ./@values file.
// each line contains an enum value, followed by an optional space+description:
//   "val1\n"
//   "val2 description up to the end of the line\n"
//
// helps  - if non null, returns a map of <enum value, help string>
// return - array of enum values.
vector<string> _get_enum_values(const string& path, map<string,string> *helps=NULL)
{
	map<string,string> __helps;
	if (!helps) helps = &__helps;
	
	vector<string> lines = read_file_lines(path + "/@values");
	
	// if the last line is empty (the user pressed ENTER at the last line entered)
	//  - then delete it.
	if (lines.size() > 0 && lines[lines.size()-1] == "")
		lines.pop_back();

	//separate lines vector into two lists: 1) enum strings ; 2) descriptions
	for (unsigned int i=0; i<lines.size(); i++)
	{
		string            help;
		string&           line = lines[i];
		string::size_type space_index = line.find(' ');
		if (space_index != string::npos)
		{
//			assert(space_index != 0); // space as first character is not allowed!
			help = line.substr(space_index+1);
			
			line.erase(space_index);
		}
		(*helps)[line] = help;
	}

	return lines;
}

/**
 * completes an enum argument.
 */
extern int enum_complete(
	const string& currpath, WordCompletion *cpl, const char *line, int word_start, int word_end)
{
	int ret = 0;

	// extract last token from line.
  string token(line + word_start, word_end - word_start);
	trace("currpath=<%s> line=<%s> token=<%s>\n", currpath.c_str(), line, token.c_str());

	typedef vector<string> vs_t;
	vs_t files, matches;
	vs_t* display;

	map<string, string> helps; // list of enum values with their help lines
	
	files = _get_enum_values(currpath, &helps);
	
	// remove all un-matched files from the list.
	matches = filter_matches(files, token);
	display = &matches;

	// if there are no matches, show an error message, whchi will be followed by the list
	// of all possible selections (files).
	if (matches.size() == 0)
	{
		printf("error: invalid options '%s'.\n", token.c_str());
		display = &files;
		ret = -1;
	}
	
	// display the list of possible options only if there are 0 or more than 1.
	if (matches.size() != 1)
	{
		printf("possible options:\n");
	
		for (vs_t::iterator it = display->begin(); it != display->end(); it++)
		{
			string line_to_print;
			
			// print the name of the option.
			line_to_print = "  " + *it;
			
			// if there's a helpline, print it followed by the option.
			if (helps[*it].size())
				line_to_print += " - " + helps[*it];
			
			line_to_print  += '\n';

			printf(line_to_print.c_str());
		}

		printf("\n");
	}

	// add vector of possible options to completions object.
	add_completions_vector(cpl, line, word_start, word_end, matches);
	return ret;
}

/**
 * called when the user executes a command with an enum argument.
 */
extern int enum_execute(const string& nodepath, const string& argname, string& token)
{
	typedef vector<string> vs_t;
	int index;
	int ret;
	
	string err;

	trace("nodepath=%s, token=%s\n", nodepath.c_str(), token.c_str());
	vs_t values = _get_enum_values(nodepath);
	ret = enum_parse(token, values, false, &index, &err);

	// print error, if we had one.
	if (ret != 0)
	{
		fprintf(stderr, "error: %s: %s\n", argname.c_str(), err.c_str());
		return PEDRO_EXEC_ERROR;
	}

	// change token, the value passed to the command handler, to the integer index
	ostringstream os;
	os << index;
	token = os.str();

	return PEDRO_EXEC_SUCCESS;
}

/**
 * parses a string and checks if it corresponds to some predetermined list of values.
 * @param input - the string to parse
 * @param values - an array of strings that contains the list of values.
 * @param value_count - the number of elements in values.
 * @param is_case_sensitive - set to true if you want to comparison to be case sensitive.
 * @param out_index - the index, in values, of the input (if matched).
 * @param out_err - if not null, will contain a descriptive error.
 * @param err_len - the length of the out_err buffer.
 * @return 0 upon sucess and -1 if failed.
 */
extern int enum_parse(
	const string& input,
	const vector<string>& values,
	int is_case_sensitive,
	int* out_index,	
	string* out_err)
{
	typedef vector<string> vs_t;
				
	int (*cmp)(const char*,const char*);
	int ret;
	string option_list;

	assert(out_index != NULL);

	// reset output and err.
	*out_index = -1;
	if (out_err) out_err->clear();
	
	// select comparison function.
	if (is_case_sensitive) cmp = strcmp;
	else cmp = strcasecmp;

	// compare the input to all values, until found.
	
	int i = 0;
	for (vs_t::const_iterator it = values.begin(); it != values.end(); ++it, i++)
	{
		if (cmp(input.c_str(), it->c_str()) == 0) *out_index = i;
		option_list += *it + " ";
	}

	// if not found, return an error.
	if (*out_index == -1)
	{
		if (out_err)
		{
			*out_err = stl_sprintf("'%s': invalid option. valid options are: %s", 
											input.c_str(), option_list.c_str());
		}

		ret = -1;
		goto cleanup;
	}

	// if found, return 0.
	ret = 0;
	
cleanup:
	return ret;				
}
