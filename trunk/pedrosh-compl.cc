/**
 * pedrosh-compl.c
 * the completion function of pedro shell.
 *
 * copyright (c) 2006 by embrix.
 * all rights reserved.
 * developed by embrix for corrigent systems ltd.
 *
 * @author talba@embrix.com
 * @since july, 2006
 */

#include <config.h>   // autoconf

#include <string.h>
#include <stdio.h>
#include <dirent.h>
#include <fcntl.h>
#include <string.h>

#include <vector>
#include <string>
using namespace std;

#include <libtecla.h>
#include "pedrosh-i.h"
#include "params.h"
#include "utils.h"


CPL_MATCH_FN(cli_compl2);
int cmpl_arg_val(const string& currpath, WordCompletion *cpl, const char *line,
                 int word_start, int word_end);

  // this is horrible: we are using the type_suffix parameter of
  // cpl_add_completion(), to display a short help message for each
  // possible completion. the problem is that this string parameter is meant
  // to be a literal string - it is not copied. so we must make sure that
  // the help strings this functions passes, remain valid 
  // "for  at  least  as  long  as  the  results  of cpl_com-
  // plete_word() are needed".
  // this arrays holds all such help strings.
  static vector<string> malloced_strings;

CPL_MATCH_FN(cli_compl)
{
	int ret;
	char new_line[MAX_PATH+1];
	int new_word_end;

	gl_normal_io(g_gl);

	malloced_strings.clear();	// free the previously malloced help strings
 
	new_word_end = word_end;
	new_line[0] = '\0';	
	strncat(new_line, PEDRO_META_GLOBALS " ", MAX_PATH);
	new_word_end += strlen(new_line);

	strncat(new_line, line, MAX_PATH);

	ret = cli_compl2(cpl, data, new_line, new_word_end);
	if (ret != 0) return ret;

	return cli_compl2(cpl, data, line, word_end);
}

/**
 * int cli_compl(WordCompletion *cpl, void *data, const char *line, int word_end)
 */
CPL_MATCH_FN(cli_compl2)
{
	//
	// separate the tokens of line
	//
  static const char TOKEN_SEPARATORS[] = " \t\n"; // also appears in execute_command_line()
	vector<string> tokens;
	int token_start, token_end;
	int word_start;
	string line_wrapper = line; // wrap line with string object
		
	token_start = token_end = 0;
	while (1)
	{
//TODO: should use line_wrapper.find_first_of() line_wrapper.find_first_not_of()
  // get next token limits
		token_start = token_end   + strspn(line+token_end   ,TOKEN_SEPARATORS);
		token_end   = token_start + strcspn(line+token_start,TOKEN_SEPARATORS);
		
		// split to three cases:
		//  1. word_end <= token_start
		//       when the user pressed TAB, the cursor was on a separator char
		//       between the previous token and the current token.
		//  2. token_start < word_end <= token_end
		//       user wants to complete this token, or part of it.
		//  3. token_end < word_end
		//       the user does not want to complete this token.
		if (word_end <= token_start) // case 1
		{
			word_start = word_end;
			tokens.push_back(string());			// push an empty token to the end.
			break;
		}
		else if (token_end < word_end)   // case 3
		{
			tokens.push_back(string(line_wrapper, token_start, token_end-token_start));
		}
		else // case 2
		{
			word_start = token_start;
			token_end  = word_end;
			tokens.push_back(string(line_wrapper, token_start, token_end-token_start));
			break; // don't care if there are any other tokens on line
		}
	} // while (1)


	// now vector<string> tokens contains the list of all the tokens,
	// the last token is the one to be completed.
	//  word_start,word_end,line    are set up correctly for later, when we want
	//                              to call cpl_add_completion()
	

	string currpath;
	string tmp;
	
	currpath = g_mgmt_path;
	
	// special handling for the first token:
	//   it may contain "/" to indicate absolute path
	//   examples:
	//      1)          /      [change to root directory]
	//      2)          /interface eth enable
	//      3)          / interface eth enable
	if (tokens.size() > 0 && tokens[0][0] == '/')
	{
		//if (tokens[0].size() > 1)
			tokens[0].erase(0,1);
		//else
		//	tokens.erase(0);
	}
	else
	{
		// currpath = g_mgmt_path + '/' + g_curr_dir
		if ( g_curr_dir[0] != 0 )
		{
			currpath += '/';
			currpath += g_curr_dir;
		}
	}

	vector<string> visited_nodes;

  // go over all the tokens preceding the token to be completed, and build
	//  a path to the relevant node.
	for (unsigned int i=0; i<tokens.size()-1; i++)
	{
		// may need to skip the initial token if it was only a single "/"
		if ( /* i==0 && */ tokens[i].size() == 0)
			continue;
			
		if (tokens[i] == "..")
		{
			currpath = cdup(currpath, g_mgmt_path);
			continue;
		}

		// don't allow tokens with "." or "/" in them
		if ( tokens[i].find_first_of("./") != string::npos )
      return 0;
		
		// exit if this is not a valid directory
		tmp = string(currpath+'/'+tokens[i]);
		if (!dir_exists(tmp))
			return 0;
		
		// the new token is a valid directory. update currpath.
		currpath = tmp;
		visited_nodes.push_back(currpath);
		
		// if this directory is an executable command
		// test for the existence of the file $currpath/@exec
		if (file_exists(currpath+"/"PEDRO_META_EXEC) ||
		    file_exists(currpath+"/"PEDRO_META_EXEC_ARGV))
		{
			// executable node - print the help msg, tell the user 
			// he may press ENTER, and exit this function
			string help_msg = read_file(currpath+"/"PEDRO_META_HELP);
			if (help_msg != "")
				puts(help_msg.c_str());
				
			printf("press ENTER to execute the command\n");
		}
	
		// special behaviour to support the internal "help" command!
		// eladb: it's not special to support "help", it's to support empty names! :-)
		tmp = currpath+"/"PEDRO_META_VOID;
		if (dir_exists(tmp))
		{
			currpath = tmp;
			visited_nodes.push_back(currpath);
		}

		// did we just enter an argument node?
		if (file_exists(currpath+"/"PEDRO_META_ARG))
		{
			if (i < tokens.size()-2) // is this last token before the token-to-be-completed?
				// no! just skip one token (the argument value on the command line)
			{
				// skip the argument value token if it's not intended for completion.
				i++;
				currpath = cdup(currpath, g_mgmt_path);
			}
			else
			{
				int res = cmpl_arg_val(currpath.c_str(), cpl, line, word_start, word_end);
				return res;
			}
		}
	}

	/////////////////////////////////////////////////////////////////////////////
	// ok, currpath is set correctly.
	// now we want to complete tokens[tokens.size()-1].c_str()
  //   word_start,word_end are also correctly set.
	/////////////////////////////////////////////////////////////////////////////
	
	// special handling for ".."
	// normally we don't complete files starting with "."
	// but this keeps the normal behavior when the user enters "..<TAB>" -
	//  a space is added to the end of the word, and another TAB completes
	//  from the parent subdirectory.
	if (tokens[tokens.size()-1] == "..")
	{
		cpl_add_completion(cpl, line, word_start, word_end,
						"", // suffix
						"", // type_suffix (must be malloced)
						" "); // cont_suffix
		return 0;
	}
	
	struct dirent** ents;   // for scandir()
	int no_of_ents;      // for scandir()
	ents = NULL;
	no_of_ents = scandir(currpath.c_str(), &ents, NULL, alphasort);
	for (int i = 0; i < no_of_ents; ++i)
	{
		// make sure this file name starts with our token-to-be-completed
		if (strncmp(ents[i]->d_name, 
								tokens[tokens.size()-1].c_str(),
		            tokens[tokens.size()-1].length()))
		{
			free(ents[i]);
			continue;
		}
    
		// reject names starting with "." or (@) PEDRO_META_CHAR
		if (ents[i]->d_name[0] == '.' || ents[i]->d_name[0] == PEDRO_META_CHAR)
		{
			free(ents[i]);
			continue;
		}
		
		// accept only directories 
		tmp = currpath + "/" + ents[i]->d_name;
		if (!dir_exists(tmp))
		{
			free(ents[i]);
			continue;
		}

		// have we already visited the path we considering to enter?
		// we will reject it, to eliminate loops, and to support single entering of each argument.
		if (find(visited_nodes.begin(), visited_nodes.end(), currpath + '/' + ents[i]->d_name) != visited_nodes.end())
		{	
			// yep. reject it.
			free(ents[i]);
			continue;
		}

		// check if this node is a "directory" (is the user allowed to make it the current directory)
		tmp = currpath + "/" + ents[i]->d_name;
		bool is_directory = file_exists(tmp+"/"PEDRO_META_DIR);
    
		// get the help msg for this file
		string help_msg = read_file(tmp+"/"PEDRO_META_HELP);
		if (help_msg != "")
			help_msg = " (" + help_msg + ") ";
		if (is_directory) 
			help_msg = '/' + help_msg;
		if (help_msg != "")
			malloced_strings.push_back(help_msg);
		
		// add this directory to the completion list.
		cpl_add_completion(cpl, line, word_start, word_end,
						ents[i]->d_name + tokens[tokens.size()-1].size(), // suffix
						help_msg.size() ? // type_suffix (must be malloced)
						  malloced_strings[malloced_strings.size()-1].c_str() : "",  
						" "); // cont_suffix
	
		free(ents[i]);
	}
	
	free(ents);
	return 0;
}


/**
 * a completion function for arguments. called to allow user to enter an argument.
 * @param currpath - the path of the argument.
 * @param cpl,line,word_start,word_end - completion arguments.
 */
int cmpl_arg_val(const string& currpath, WordCompletion *cpl, const char *line,
                 int word_start, int word_end)
{
	// read the param type from @arg file.
	const param_type_t *pt = get_param_type(currpath);
	if (!pt)
	{
		// todo: elad default to string type i think, instead of an error.
		printf("cmpl_arg_val(): get_param_type() returned NULL!\n");
		return -1;
	}

	// call the completion function for this type.
	return pt->cmpl_func(currpath, cpl, line, word_start, word_end);
}

