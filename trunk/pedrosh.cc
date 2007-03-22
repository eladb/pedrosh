/*
 * copyright (c) 2006 by embrix.
 * 
 * all rights reserved.
 * 
 * developed by embrix for corrigent systems ltd.
 */


/* this should be the first include file */
#include <config.h>

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>
#include <locale.h>
#include <dirent.h>
#include <unistd.h>
#include <fcntl.h>
#include <assert.h>

#include <vector>
#include <string>
#include <map>
using namespace std;

#include <libtecla.h>

#include "pedrosh-i.h"
#include "pedrosh-getopt.h"
#include "pedrosh-compl.h"
#include "params.h"
#include "utils.h"

// plugins
#include "plugins.h"
#include "plugin-ipc.h"
#include "plugin-stub.h"
#include "plugin-globals.h"


//
// defines
//

#ifndef DEF_MGMTFS_PATH
#define DEF_MGMTFS_PATH "/var/pedrofs"
#endif

//
// private data
//

GetLine *g_gl;
string g_mgmt_path;          // without no trailing "/"
string g_curr_dir;    // without trailing "/", without leading "/"

//
// function declarations
//

int execute_command_line2(char* line, char* out_err, int err_len, int allow_relative_path);

//
// function definitions
//


// read all generic parameter description of a specific node.
// generic means everything needed for the execution of a command by
// the pedrosh "infracstructure" function. specific parts needed for
// specific param type will not be read here. (they should be read by
// the specific parameter type handle function.
//
// return: false if ok, true on error
bool load_node_params_desc(const string& nodepath, map<string/*subdir_name*/, param_t>& params_desc)
{
	struct dirent** ents;   // for scandir()
	int no_of_ents;      // for scandir()
	ents = NULL;
	no_of_ents = scandir(nodepath.c_str(), &ents, NULL, alphasort);
	string tmp;

	params_desc.clear(); // maybe this should be left to the caller?
	
	for (int i = 0; i < no_of_ents; ++i)
	{
		// reject names starting with "." or (@) PEDRO_META_CHAR
		// especially - we MUST reject ".." and "."
		if (ents[i]->d_name[0] == '.' || ents[i]->d_name[0] == PEDRO_META_CHAR)
		{
			free(ents[i]);
			continue;
		}
	
		tmp = nodepath + "/" + ents[i]->d_name;
		const param_type_t *param_type = get_param_type(tmp);
		if (!param_type)
		{
			free(ents[i]);
			continue;
		}
		
		param_t& param_desc = params_desc[ents[i]->d_name];
		param_desc.param_name = ents[i]->d_name;
		param_desc.param_callback_name = read_file( 
		            nodepath + "/" + ents[i]->d_name + "/"PEDRO_META_ARG_CBNAME,
		            ents[i]->d_name  // deafult value
		            );
		param_desc.param_name = ents[i]->d_name;
		param_desc.is_optional = file_exists(tmp + "/"PEDRO_META_ARG_OPT);
		param_desc.param_type = param_type;
	}

	free(ents);

	return false;
}

// this function is part of the execution sequence of a command line.
// it is called by execute_command_line() to collect parameters for every
// node we walk through.
//
// parameters - see below.
//
// ret: PEDRO_EXEC_SUCCESS
//      PEDRO_EXEC_ERROR - on error, the function prints an error message
//                         before returning
int exec_handle_node_params(
		const string &nodepath, // full path (incl g_mgmt_path) to the node whose
		                        // parameters we're going to handle
		map<string,string>& named_params, // list of pairs - <param_callback_name, param_val>
		                              //   if this function finds parameters,
		                              //   they are added to this list.
		vector<string>& tokens, // the list of tokens on the command line
		vector<string>::iterator &curr_token // input:  the place in tokens to start
		                                     //         parsing from.
		                                     // output: the first token that is not a 
		                                     //         parameter of this node.
		)
{
	map<string/*subdir_name*/, param_t> params_desc;
	typedef typeof(params_desc) typeof__params_desc; // used to define iterators

	if (load_node_params_desc(nodepath, params_desc))
	{
		printf("unknown error.\n");
		return PEDRO_EXEC_ERROR;
	}

  bool non_arg_found = false;
	while (curr_token != tokens.end())
	{
		typeof__params_desc::iterator param_desc;
		param_desc = params_desc.find(*curr_token);
		if (param_desc == params_desc.end())
		{
			// reached a non parameter token. stop here.
			non_arg_found = true;
			break;
		}
		
		//
		// now *curr_token is parameter name. *iter is the corresponding param_type_t.
		//

		// make sure this is the first time this parameter is entered
		if (named_params.find((*param_desc).second.param_callback_name)
			!= named_params.end())
		{
			printf("error: parameter '%s' is given more than once.\n",
				(*param_desc).second.param_name.c_str());
			return PEDRO_EXEC_ERROR;
		}
		
		// make sure the user entered another token, which we'll take as the
		// parameter value
		if (++curr_token == tokens.end())
		{
			printf("error: missing parameter value for parameter '%s'.\n",
				(*param_desc).second.param_name.c_str());
			return PEDRO_EXEC_ERROR;
		}
	
		// add the new token to the map
		named_params[ (*param_desc).second.param_callback_name ] = *curr_token;
		string &token_in_map = named_params[ (*param_desc).second.param_callback_name ];
		
		string arg_name = param_desc->second.param_name;
		int res = (*param_desc).second.param_type->exec_func(nodepath + "/" + arg_name, arg_name, token_in_map);
		if (res != PEDRO_EXEC_SUCCESS)
			return PEDRO_EXEC_ERROR;  // error message already printed
		
		curr_token++;
	}

	// ok, we're done with the parameters the user entered.
	// but has he left out a mandatory parameter?
	
	typedef typeof(params_desc) typeof__params_desc;
	typeof__params_desc::iterator param_desc;
	param_desc = params_desc.begin();
	while (param_desc != params_desc.end())
	{
		if ((*param_desc).second.is_optional == false &&
		    named_params.find( (*param_desc).second.param_callback_name )
			   == named_params.end()
			 )
		{
			printf("error: mandatory parameter is missing - '%s'.\n",
				(*param_desc).second.param_name.c_str());
			return PEDRO_EXEC_ERROR;
		}
		param_desc++;
	}

	return PEDRO_EXEC_SUCCESS;
}

int is_empty_line(const char* line)
{
	// todo: support other whitespace
	return line[0] == '\n' || line[0] == 0 || line[0] == '#';
}

int execute_command_line(char* line)
{
	char err[MAX_ERR_LEN+1];
	char new_line[MAX_PATH+1];
	int ret;

	if (is_empty_line(line)) return PEDRO_EXEC_SUCCESS;
	
	strncpy(new_line, PEDRO_META_GLOBALS " ", MAX_PATH);
	strncat(new_line, line, MAX_PATH);
	ret = execute_command_line2(new_line, err, MAX_ERR_LEN, 0); // no relative path

	// if the attempt to execute the global command failed with an unknown command,
	// we will go and execute the command as is.
	if (ret == PEDRO_EXEC_UNKNOWN)
	{
		ret = execute_command_line2(line, err, MAX_ERR_LEN, 1);
	}

	if (ret != PEDRO_EXEC_SUCCESS && line[0] != '\n')
	{
		fprintf(stderr, "%s", err);
	}

	return ret;
}


// This function handles the line that gl_get_line() returned.
// The line is parsed to see what command should be executed. The parameters
// are gathered and the command handler is called.
// It uses the globals variables: mgmt_path, gl
// return: 0=no error
int execute_command_line2(char *_line, char* out_err, int err_len, int allow_relative_path)
{
	//
	// separate the tokens of _line
	//
  static const char TOKEN_SEPARATORS[] = " \t\n"; // also appears in execute_command_line()
	vector<string> tokens;
	int token_start, token_end;
	string line_wrapper = _line; // wrap line with string object
	string exec_node_path; // path to the first @exec we meet while we parse
		
	token_start = token_end = 0;
	while (1)
	{
//TODO: should use line_wrapper.find_first_of() line_wrapper.find_first_not_of()
  // get next token limits
		token_start = token_end   + strspn(_line+token_end   ,TOKEN_SEPARATORS);
		token_end   = token_start + strcspn(_line+token_start,TOKEN_SEPARATORS);
		if (token_start == token_end)
			break;

		tokens.push_back(string(line_wrapper, token_start, token_end-token_start));
	} // while (1)

	// now vector<string> tokens contains the list of all the tokens
	
	string currpath;
	string tmp;

	////vector<string> argv;	// TODO: maybe this should be defined later, when it is used
	map<string,string> named_params;
	
	currpath = g_mgmt_path;
	typedef typeof(tokens) typeof__tokens;
	typeof__tokens::iterator currtok = tokens.begin();

	if (tokens.size() == 0) // if there are no tokens, ignore the line
	                        // i think everything will go smoothly even without
	                        // this chekc, but i don't want to check...
		return PEDRO_EXEC_SUCCESS;

	// special handling for the first token:
	//   it may contain "/" to indicate absolute path
	//   examples:
	//      1)          /      [change to root directory]
	//      2)          /interface eth enable
	//      3)          / interface eth enable
	if (tokens.size() > 0 && tokens[0][0] == '/')
	{
		if (tokens[0].size() > 1)
			tokens[0].erase(0,1);
		else
			currtok++; // skip the first token if it is exactly "/"
	}
	else
	{
		if ( g_curr_dir[0] != 0 )
		{
			currpath += '/';
			currpath += g_curr_dir;
		}
	}

	gl_normal_io(g_gl);

  // go over all the tokens, building a path and collecting parameters
	// as we go
	while (currtok != tokens.end())
	{
		if (*currtok == "..")
		{
			if (allow_relative_path)
			{
				currpath = cdup(currpath, g_mgmt_path);
				currtok++;
				continue;
			}
			else
				return PEDRO_EXEC_UNKNOWN;
		}
		
		// don't allow tokens with "." or "/" in them
		if ( (*currtok).find_first_of("./") != string::npos )
		{
			if (out_err != NULL)
				snprintf(out_err, err_len, "Error: unknown command - %s\n", (*currtok).c_str());
			return PEDRO_EXEC_UNKNOWN;
		}
		
   	// check if $currpath/$token exists and is a directory
		tmp = currpath + '/' + (*currtok);
		if (!dir_exists(tmp))
		{
			if (out_err != NULL)
				snprintf(out_err, err_len, "Error: unknown command - %s\n", (*currtok).c_str());
			return PEDRO_EXEC_UNKNOWN;
		}
		currpath = tmp; // the new token is a valid subdir. update currpath.

		// is this an argc/argv style executable node?
		if (file_exists(currpath+"/"PEDRO_META_EXEC_ARGV))
		{
			//
			// collect the parameters into an argc/argv style
			//
			currtok++; // now currtok is the token *after* the @exec command.
			int _argc = 1;
			char *_argv[tokens.size()]; // that's a little more then we need. nevermind.
			
			_argv[0] = (char*)currpath.c_str()+g_mgmt_path.size()+1;
			while (currtok != tokens.end())
				_argv[_argc++] = (char*)(*currtok++).c_str();
			
			//
			// call the correct plugin and exit this function
			//
			int cmd_ret = plugins_execute(_argc, _argv);
			if (cmd_ret)
			{
				if (out_err != NULL) out_err[0] =0;
				return PEDRO_EXEC_ERROR;
			}

			return PEDRO_EXEC_SUCCESS;
		}
		
		// is this an executable node?
		// test for the existence of the file $currpath/@%exec
		if (file_exists(currpath+"/"PEDRO_META_EXEC))
		{
	    exec_node_path = currpath;
		}

		currtok++;  // advance to next token

		// handle the parameters of the previous token, if any, before looping
		int ret = exec_handle_node_params(currpath, named_params, tokens, currtok);
		if (ret)
		{
			if (out_err != NULL) out_err[0] = 0;
			return ret;
		}
		if (currtok == tokens.end()) // are we out of tokens
			break;
	} // for each token in tokens


	// no @exec node found?
	if (exec_node_path.size() == 0)
	{
		// does currpath has the @dir attribute?
		// (can the user make this node his "current directory"?)
		if (!file_exists(currpath+"/"PEDRO_META_DIR))
		{
			// no - not a command and not a "directory". can't press ENTER on this node!
			if (out_err != NULL) snprintf(out_err, err_len, "Error: non executable command. More input is needed.\n");
			return PEDRO_EXEC_NONCOMMAND;
		}

		// this is a "directory", make it the "current directory"
		//  currpath is usually something like "$g_curr_dir/....."
		//  but it may also be exactly "$g_curr_dir", without a trailing "/" !!
		const char *p = currpath.c_str() + g_mgmt_path.size();
		if (*p == '/') p++;
		g_curr_dir =  p;
		return PEDRO_EXEC_SUCCESS;
	}

	
	//
	// here - we have a valid @exec command + named parameter.
	// first build argc/argv
	//
	typedef typeof(named_params) typeof__named_params;
	typeof__named_params::iterator nparams_iter;
	char *_argv[1 + 2*named_params.size()];
	int _argc = 1;
	_argv[0] = (char*)exec_node_path.c_str()+g_mgmt_path.size()+1;
	
	nparams_iter = named_params.begin();
	while (nparams_iter != named_params.end())
	{
		_argv[_argc++] = (char*)(*nparams_iter  ).first.c_str();
		_argv[_argc++] = (char*)(*nparams_iter++).second.c_str();
	}

	int cmd_ret = plugins_execute(_argc, _argv);
	if (cmd_ret)
	{
		if (out_err != NULL) out_err[0] =0;
		return PEDRO_EXEC_ERROR;
	}

	return PEDRO_EXEC_SUCCESS;
}


/**
 * the main entry point of pedrosh
 */
int main(int argc, char *argv[])
{
  struct gengetopt_args_info args_info;

  // default mgmt root.
  g_mgmt_path = DEF_MGMTFS_PATH;

  /* call the cmdline parser generated by gengetopt to parse argc,argv */
  if (cmdline_parser (argc, argv, &args_info) != 0)
    exit(EXIT_FAILURE);

  if (args_info.help_given)
  {
    cmdline_parser_print_help ();
    printf("\nReport bugs to <"PACKAGE_BUGREPORT">.\n");
    exit (EXIT_SUCCESS);
  }
		
	// don't accept non-option parameters on the command line
  if (args_info.inputs_num)
	{
    fprintf(stderr, "error: bad command line parameter.\n");
    exit(EXIT_FAILURE);
	}
	
  // in case --mgmtfs was specified.
  if (args_info.mgmtfs_given)
  {
    g_mgmt_path = args_info.mgmtfs_arg;
  }
  
  // in case --testing-stub was specified.
  if (args_info.testing_stub_given)
  {
		plugins_set_default_handler(stub_cmd_handler);
  }
	else
	{
		plugins_set_default_handler(ipc_cmd_handler);
	}
  
	// register global commands into the plugin module.
	globals_init();
	params_init();

					
  // todo: only available in a newer version of gengetopt??
  // cmdline_parser_free (&args_info);
  
  // verify that mgmt_path is a valid directory
  if (!dir_exists(g_mgmt_path))
  {
    fprintf(stderr,"error: '%s' is not a valid directory path.\n", g_mgmt_path.c_str());
    return EXIT_FAILURE;
  }

  // initialize the pedrosh-ipc module.
  // todo: in the future, dynamically initialize all plugins.
  if (ipc_init() != 0)
  {
    fprintf(stderr, "error initializing pedro ipc support.");
    return EXIT_FAILURE;
  }

  // create the line editor, specifying a max line length of 500 bytes,
  // and 10000 bytes to allocate to storage of historical input lines.
  g_gl = new_GetLine(500, 5000);
  if(!g_gl) return 1;

  // if the user has the LC_CTYPE or LC_ALL environment variables set,
  // enable display of characters corresponding to the specified locale.
  (void) setlocale(LC_CTYPE, "");
  
  //gl_completion_action(gl, NULL, cli_compl, 0, "ebi", " ");
  gl_completion_action(g_gl, NULL, cli_compl, 0, "ebi", "?");
  gl_customize_completion(g_gl, NULL, cli_compl);

  // read lines of input from the user and print them to stdout.
  while (1)
  {
    char prompt[MAX_PATH];
    char *line;
    
    sprintf(prompt, "[%s/] ", g_curr_dir.c_str());
		    
    // get a new line from the user.
    line = gl_get_line(g_gl, prompt, NULL, 0);

    // if there's a new line from the user, execute it.
    if (line != NULL)
    {
      execute_command_line(line);

      // todo: handle return value
    }
    else
    {
      // if there was an error, break the loop.
      break;
    }
  }

  // cleanup.
  g_gl = del_GetLine(g_gl);

  // shutdown the pedrosh-ipc module.
  // todo: in the future, dynamically shutdown all plugins.
  if (ipc_cleanup() != 0)
  {
    fprintf(stderr, "error while shutting down pedro ipc support.");
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
