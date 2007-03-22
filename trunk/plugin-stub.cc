/**
 * pedro-stub.c
 * stub for pedro commands implementations.
 *
 * copyright (c) 2006 by embrix.
 * all rights reserved.
 * developed by embrix for corrigent systems ltd.
 *
 * @author talba@embrix.com
 * @since july, 2006
 */

#include <stdio.h>
#include <string.h>
#include <libtecla.h>

#include "pedrosh-i.h"

// this execution stub should be written by the user. (??)
int stub_cmd_handler(int argc, char *argv[])
{
  int i;
  char buff[2000];
  char cmddir[MAX_PATH]; // path to the command directory: ${mgmtfs_path}/${argv[0]}
  sprintf(cmddir,"%s/%s", g_mgmt_path.c_str(), argv[0]);
  
  printf("user_cmd_handler(argc=%d):\n", argc);
  
  for (i=0; i<=argc; i++)
  {
    printf("\t argv[%d] = '%s'\n", i, argv[i] ? argv[i] : "NULL");
  }
  
  // call user shell script to handle the command
  // by running the shell command:
  //    if [ -x argv[0]/%.exec ] ; then argv[0]/%.exec 'argv[1]' ... 'argv[argc]' ; fi
  sprintf(buff, "if [ -x %s/%s ] ; then %s/%s ",
	  cmddir, 
	  "%.exec", // putting this as a litteral confuses sprintf. workaround is not portable
	  cmddir,
	  "%.exec");
  for (i=1; i<argc; i++)
  {
    strcat(buff, "'");
    strcat(buff, argv[i]);
    strcat(buff, "' ");
  }
 
  strcat(buff, "; fi");
  
  system(buff);
  return 0;
}
