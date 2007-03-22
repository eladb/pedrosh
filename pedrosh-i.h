// pedrosh INTERNAL header file

#ifndef __PEDROSH_I_H__
#define __PEDROSH_I_H__

#include <string>
using namespace std;

#include <libtecla.h> // for GetLine typedef

//
// defines
//

#define MAX_PATH 2048
#define MAX_ERR_LEN	512

#define MAX(a,b) (a > b ? a : b)

// message definition. used with libipc.
#define PEDROSH_MODULE_NAME   "pedrosh"
#define PEDROSH_MSG_NAME      "pedrosh_oncmd"
typedef struct {
	int argc;
	char **argv;
} PEDROSH_MSG;
#define PEDROSH_MSG_FORM      "{int, <string: 1>}"

#define PEDRO_META_CHAR		 '@'
#define PEDRO_META_GLOBALS "/@globals"
#define PEDRO_META_EXEC		 "@exec"
#define PEDRO_META_EXEC_ARGV "@exec.argv"
#define PEDRO_META_PLUGIN  "@plugin"
#define PEDRO_META_HELP		 "@help"
#define PEDRO_META_DIR		 "@dir"
#define PEDRO_META_VOID    "@void"
#define PEDRO_META_ARG     "@arg"
#define PEDRO_META_ARG_OPT  "@arg.opt"
#define PEDRO_META_ARG_CBNAME  "@arg.callback_name"
																												

/* these two meta files are used by plug-ins and not part of the pedro infrastructure */
// #define PEDRO_META_DOC     "@doc"     // defined in plugin-globals.h
// #define PEDRO_META_SYSTEM  "@system"  // defined in plugin-globals.h

#define PEDRO_EXEC_SUCCESS			(0)
#define PEDRO_EXEC_UNKNOWN			(-1)
#define PEDRO_EXEC_NONCOMMAND  	(-2)
#define PEDRO_EXEC_TOOMANYARGS 	(-3)

#define PEDRO_EXEC_ERROR				(-128)
#define PEDRO_EXEC_WARNING			(128)


//
// globals declarations
//

extern string g_mgmt_path;
extern string g_curr_dir;
extern GetLine *g_gl;

#endif /* __PEDROSH_I_H__ */
