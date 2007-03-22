/* pedrosh EXTERNAL header file - for programs using pedro */

#ifndef __PEDROSH_H__
#define __PEDROSH_H__

typedef enum {
	PEDRO_ARG_STRING,
	PEDRO_ARG_INT,
	PEDRO_ARG_IPV4,
	PEDRO_ARG_IPV6,
	PEDRO_ARG_ENUM,
	PEDRO_ARG_DATETIME
} pedro_arg_type_t;


typedef struct
{
	char *arg_name;              // the name the user enters. different nodes on the
	                             //   same command line may have parameters with the
	                             //   same param_name;
//char *arg_callback_name;     // the name by which the callback function refers to
	                             //   this param. this must be unique across a single
	                             //   command line, or the callback function won't be
	                             //   able to differentiate between the parameters.
	int is_optional;

	pedro_arg_type_t arg_type;   // argument type
	
	// if arg_type==PEDRO_ARG_STRING:
	//     min = (int)"val1\0val2\0val3\0";
	//     max not used
	// if arg_type==PEDRO_ARG_INT:
	//     min = minimum value
	//     max = maximum value
	// else
	//     min, max unused
	int min;                     
	int max;
} pedro_arg_t;


// this is the arguments passed to pedro_callback_func_t functions
typedef struct {
	int argc;
	char **argv;
	FILE *fin;
	FILE *fout;
} pedro_callback_func_args_t;

// command callback function
typedef void (*pedro_callback_func_t)(int argc, char *argv[],
                                      FILE *fin, FILE *fout,
                                      pedro_callback_func_args_t *pedro);


// a generic structure which describes all pedro nodes the user might be interested in
// the meaning of some field may change depending on the nodetype (and some fields
// may simply become irrelevant). see example_task2.cc for a complete example.
typedef struct {
	char             *nodepath; // in this format "interface/ethernet/disable"
	                            //   spaces and dots are not allowed.
	pedro_callback_func_t callback; // - callback function (if executable node)
	                                // - arg name alias (optional, for arg node)
	pedro_arg_t *args;  // if NULL, command is parsed in argc/argv mode
	                    //  (parameters are not validated)
	char *shorthelp;    // SHORT help message. NULL is allowed. displayed when pressing TAB.
	char *longhelp;     // long help message. NULL is allowed.
} pedro_cmd_t;


/* prefix to define globals commands - commands that will be found regardless
   of the current directory */
#define PEDRO_GLOBALS "@globals"


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

int pedro_init(const char *mgmtfs);

int pedro_add_cmds(pedro_cmd_t *nodes,
                   const char *mgmtfs); // path to the management file system


// internal commands

int pedro_add_internal_exit_cmd(const char *help, const char *longhelp, const char *nodepath, const char *mgmtfs);
int pedro_add_internal_telnet_cmd(const char *help, const char *longhelp, const char *nodepath, const char *mgmtfs);
int pedro_add_internal_system_cmd(const char *cmd_to_run, const char *help, const char *longhelp, const char *nodepath, const char *mgmtfs);
int pedro_add_internal_help_cmd(const char *help, const char *longhelp, const char *nodepath, const char *mgmtfs);
int pedro_add_internal_cdup_cmd(const char *help, const char *longhelp, const char *nodepath, const char *mgmtfs);



// utility function to manipulate the mgmtfs.
int pedro_del_node(const char *nodepath, const char *mgmtfs);

int pedro_add_cmd(
									pedro_callback_func_t callback,
									int   validate_args_flag,
									const char *shorthelp,
									const char *longhelp,
									const char *nodepath,
									const char *mgmtfs);
#define pedro_add_enterable_dir(shorthelp, longhelp, nodepath, mgmtfs) \
	pedro_add_cmd(NULL, 0, (shorthelp), (longhelp), (nodepath), (mgmtfs))


//
// add parameters to commands
//

// param_type: "bool", "string", "enum" "ipv4", "ipv6", "int"
int pedro_add_param(const char *callback_param_name, // may be NULL
                    const char *param_type,
                    int optional_flag,
                    const char *nodepath,
                    const char *mgmtfs);
int pedro_add_int_param(int min, int max,  // if min==max, they are both ignored!
                    // rest of the parameters - like pedro_add_param()
                    const char *callback_param_name,
                    const int optional_flag,
                    const char *nodepath,
                    const char *mgmtfs);

int pedro_add_enum_param(const char *strings, // '\0' separated list.
                                       // for example: "up\0down\0left\0right\0"
                    // rest of the parameters - like pedro_add_param()
                    const char *callback_param_name,
                    int optional_flag,
                    const char *nodepath,
                    const char *mgmtfs);

int pedro_add_datetime_param(const char *format,
                    // rest of the parameters - like pedro_add_param()
                    const char *callback_param_name,
                    int optional_flag,
                    const char *nodepath,
                    const char *mgmtfs);

#define pedro_add_string_param(callback_param_name, optional_flag, nodepath, mgmtfs) \
	pedro_add_param((callback_param_name), "string", (optional_flag), (nodepath), (mgmtfs))

#define pedro_add_ipv4_param(callback_param_name, optional_flag, nodepath, mgmtfs) \
	pedro_add_param((callback_param_name), "ipv4", (optional_flag), (nodepath), (mgmtfs))

#define pedro_add_ipv6_param(callback_param_name, optional_flag, nodepath, mgmtfs) \
	pedro_add_param((callback_param_name), "ipv6", (optional_flag), (nodepath), (mgmtfs))

//
// access typed arguments from command handler
//

char *pedro_get_string_arg(pedro_callback_func_args_t *pedro, char *param_cbname, char *defval);

int pedro_get_int_arg(pedro_callback_func_args_t *pedro, char *param_cbname, int defval);

#define pedro_get_enum_arg(pedro, param_cbname, defval) \
	pedro_get_int_arg((pedro), (param_cbname), (defval))

#define pedro_get_ipv4_arg(pedro, param_cbname, defval) \
	pedro_get_int_arg((pedro), (param_cbname), (defval))

int pedro_get_ipv6_arg(pedro_callback_func_args_t *pedro, char *param_cbname,
                       void *defval, void *outval);

int pedro_get_datetime_arg(pedro_callback_func_args_t *pedro, char *param_cbname,
                           struct tm *defval, struct tm* outval);

int pedro_is_arg_defined(pedro_callback_func_args_t *pedro, char *param_cbname);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __PEDROSH_H__ */




