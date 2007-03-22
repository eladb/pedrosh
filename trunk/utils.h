/**
 * utils.h
 *
 * copyright (c) 2006 by embrix.
 * all rights reserved.
 * developed by embrix for corrigent systems ltd.
 *
 * @author talba@embrix.com
 * @since july 30, 2006
 */

#ifndef __UTILS_H__
#define __UTILS_H__

#include <libtecla.h>
#include <string>
#include <vector>

using namespace std;

//
// typedefs
//

//
// function declarataions
// 

extern string cdup(const string& path, const string& min_path);
extern bool file_exists(const string& path);
extern bool dir_exists(const string& path);
extern int pednodes(const struct dirent* de);
extern int ls(const string& path, int (*filter)(const struct dirent*), vector<string>* out_files, bool full_path);
extern int lsped(const string& path, vector<string>* out_files, bool full_path);

extern vector<string> filter_matches(const vector<string>& v, const string& token);

extern string read_file(const string& filepath,
                        const string& defval="",
                        bool *error_flag=NULL,
												char delim = '\n');
vector<string> read_file_lines(const string& filepath, bool *error_flag=NULL);
extern bool write_file(const string& filepath, const string& filedata);


extern void add_completions_vector(
	WordCompletion* cpl, const char* line, int word_start, int word_end, const vector<string>& v);


extern void stl_vsprintf(string* out, const char* fmt, va_list ap);
extern void stl_sprintf(string* out, const char* fmt, ...);
extern string stl_sprintf(const char* fmt, ...);

bool mkdir_recursive(const string& path);
bool delfile_recursive(const string& path);

#endif /* __UTILS_H__ */
