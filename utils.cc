/**
 * utils.cc
 * some utility function. mostly file system access.
 *
 * copyright (c) 2006 by embrix.
 * all rights reserved.
 * developed by embrix for corrigent systems ltd.
 *
 * @author talba@embrix.com
 * @since july 30, 2006
 */

#include <config.h>     // autoconf

#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <stdarg.h>

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <map>
using namespace std;

#include <libtecla.h>
#include "pedrosh-i.h"
#include "params.h"
#include "debug.h"

#include "utils.h"

//
// type definitions
// 

//
// prototypes
//

//
// static variables
//

//
// global variables
//

//
// function definitions
//


bool file_exists(const string& path)
{
	struct  stat stat_buff;
	return !stat(path.c_str(), &stat_buff);
}

bool dir_exists(const string& path)
{
	struct  stat stat_buff;
	return !stat(path.c_str(), &stat_buff) && S_ISDIR(stat_buff.st_mode);
}

/**
 * deletes the last component of a path (similiar effect to adding "/.." to path)
 * @param _path - input and output
 * @param min_path - don't allow the '_path' parameter to go up beyond min_path.
 *             examples:
 *               cdup("/home/talba/xxx", "/home/talba") returns "/home/talba"
 *               cdup("/home/talba",     "/home/talba") returns "/home/talba"
 *
 * commands - if path ends with '/', the functions ignores this last character.
 *            if path begins with '/'............
 *
 * examples: input		output
 *           a/b/c 		a/b
 *	     a/b/c/		a/b
 *          /a/b/c		/a/b
 *	    /a/b/c/		/a/b
 *	     a			""
 *	    /a			"/"
 *	    ""			""
 */
 string cdup(const string& _path, const string& min_path)
{
	string path = _path;
	// for now, assume path does not end with "/"
	// todo: add assert() for this, or just strip it if it's there...
				
	string::size_type pos = path.find_last_of("/");
	if (pos == string::npos)
		path = "";
	else
	{
		if (pos==0)
			path = "/";
		else
			path.erase(path.begin()+pos, path.end());
	}
	
	// make sure we did't go up too much. min_path is as far as we allow to go up.
	if ( path.find(min_path) != 0 )
		path = min_path;
		
	return path;
}

/**
 * can be passed to ls to return only pedro nodes (e.g. nodes that don't start with '@' or '.'.
 */
extern int pednodes(const struct dirent* de)
{
	return (de->d_name[0] != '.' && de->d_name[0] != '@');
}

/**
 * adds all files under 'path' to the 'out_files' vector, filtering with 'filter'.
 * @return the number of elements added to out_files.
 */
extern int ls(const string& path, int (*filter)(const struct dirent*), vector<string>* out_files, bool full_path)
{
	struct dirent** files;
	int n;

	assert(out_files != NULL);
	
	trace("scandir <%s>\n", path.c_str());
	n = scandir(path.c_str(), &files, filter, alphasort);

	// if scandir returned an error, just return the error.
	if (n < 0) goto cleanup;

	// add all entries into the vector.
	for (int i = 0; i < n; ++i)
	{
		string file = files[i]->d_name;
		trace("file: %s\n", file.c_str());
		
		if (!full_path)	
			out_files->push_back(file);
		else
			out_files->push_back(path + "/" + file);
		
		free(files[i]);
	}

	free(files);
	
cleanup:
	return n;
}

/**
 * return ls(path, pednodes, out_files, full_path);
 */
extern int lsped(const string& path, vector<string>* out_files, bool full_path)
{
	return ls(path, pednodes, out_files, full_path);
}
				
/**
 * used to match only 'str's that start with 'prefix'.
 */
static bool _starts_with(const string& str, const string& prefix)
{
	return (str.find(prefix) == 0);
}

/**
 * filters 'v' by removing all elements that don't start with 'token' and returning a result vector.
 */
extern vector<string> filter_matches(const vector<string>& v, const string& token)
{	
	typedef vector<string> vs_t;
	vs_t result;

	// iterate on input vector and return only items that start with 'token'.
	for (vs_t::const_iterator it = v.begin(); it != v.end(); ++it)
	{
		if (_starts_with(*it, token))
			result.push_back(*it);
	}
	
	return result;	
}

/**
 * adds all the elements from vector 'v' to the completions object.
 */
extern void add_completions_vector(WordCompletion* cpl, const char* line, int word_start, int word_end, const vector<string>& v)
{
	typedef vector<string> vs_t;
	const char* cont = "";
	string token(line + word_start, word_end - word_start);

	if (v.size() == 1) cont = " ";
	
	for (vs_t::const_iterator it = v.begin(); it != v.end(); it++)
	{
		// only if the vector starts with the token
		if (_starts_with(*it, token))
		{
			cpl_add_completion(cpl, line, word_start, word_end, it->c_str() + token.length(), "", cont);
		}
	}
}

/**
 * writes a string into a file.
 */
bool write_file(const string& filepath, const string& filedata)
{
	ofstream out(filepath.c_str());

	if (out.is_open())
	{
		out << filedata;
		out.close();
		return false;
	}

	return true; // error
}

/**
 * read file into a string
 * filepath - file to read
 * defval - value to return if an error occured
 * error_flag - if non null, tells the caller if file access went ok.
 *              false - no error
 *              true  - error
 * delim - stop reading when this character arrives (or eof). the delim is read
 *         from the file, but inserted into the return value.
 * return:
 *      the contents of filepath, or defval if some error occured.
 */
string read_file(const string& filepath, const string& defval, bool *error_flag, char delim)
{
	if (error_flag)
		*error_flag = true; // assume an error will happen

	ifstream in(filepath.c_str());
	if (in.is_open())
	{
		string s;
		getline(in, s);
		in.close();
		if (in.bad())
			return defval;
		if (error_flag)
			*error_flag = false; // no error
		return s;
	}
	return defval;
}

/**
 * read file lines into a string vector
 * filepath - file to read
 * error_flag - if non null, tells the caller if file access went ok.
 *              false - no error
 *              true  - error
 * return:
 *      vector with lines.
 */
vector<string> read_file_lines(const string& filepath, bool *error_flag)
{
	vector<string> lines;

	if (error_flag)
		*error_flag = true; // assume an error will happen

	ifstream in(filepath.c_str());
	if (in.is_open())
	{
		string line;
		while (!in.eof() )
		{
			getline (in,line);
			
			if (in.bad())
				return lines;
				
			lines.push_back(line);
		}
		in.close();
	}
	else
		return lines;  // error

	if (error_flag)
		*error_flag = false; // no error

  return lines;
}

/**
 * an stl version of sprintf that returns a string.
 */
extern string stl_sprintf(const char* fmt, ...)
{
	string str;
	va_list args;
	va_start(args, fmt);
	stl_vsprintf(&str, fmt, args);
	va_end(args);
	return str;
}

/**
 * an stl version of sprintf with va_list.
 */
extern void stl_vsprintf(string* out, const char* fmt, va_list ap)
{
	assert(out != NULL);
	char* str = NULL;
	vasprintf(&str, fmt, ap);
	
	*out = str;
	free(str);
}

/**
 * an stl version of the all-popular sprintf.
 */
extern void stl_sprintf(string* out, const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	stl_vsprintf(out, fmt, args);
	va_end(args);
}

// return false on error
bool mkdir_recursive(const string& path)
{
	if (path.length() == 0)
		return true;

	// first make a recursive call for the same path save the last component
	string::size_type pos = path.find_last_of("/");
	if (pos != string::npos)
	{
		if (!mkdir_recursive(path.substr(0, pos)))
			return false;
	}
	
	// now create the last path directory, if it does not already exist
	if (!file_exists(path))
	{
		if (mkdir(path.c_str(), 0777))
			return false;
	}

	return true;
}

bool delfile_recursive(const string& path)
{
	if ( !file_exists(path) )
	  return true;

	if ( !dir_exists(path) )
	   return (unlink(path.c_str()) == 0);  // delete regular file

	// this is a directory.

	vector<string> files_list;
	bool success = ls(path, NULL, &files_list, false) >= 0;

	for (vector<string>::const_iterator it = files_list.begin();
	     it != files_list.end();
		 ++it)
	{
		if (*it == "." || *it == "..")
			continue;

		// delete the file.
		// don't stop on error, try to delete as much as possible.
		success = delfile_recursive(path + '/' + *it) && success;
	}
	
	// now remove the directory
	success = !rmdir(path.c_str()) && success;
	return success;
}
