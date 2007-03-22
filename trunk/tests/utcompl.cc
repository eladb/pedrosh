/**
 * utcompl.c
 */

#include <stdio.h>
#include <string.h>
#include <libtecla.h>

#include "../pedrosh-i.h"

CPL_MATCH_FN(cli_compl);

string g_mgmt_path;
string g_curr_dir;
GetLine *g_gl = NULL;


int main(int argc, char* argv[])
{
  char line[MAX_PATH+1];
  WordCompletion* wc = new_WordCompletion();
  CplMatches* matches;
  int i;
  
  if (argc < 2)
  {
    fprintf(stderr, "usage: utcompl <pedfs> line\n");
    return 1;
  }
  
  g_mgmt_path = argv[1];
  
  strcpy(line, "");
  for (i = 2; i < argc; ++i)
  {
    strcat(line, argv[i]);
    
    if (i + 1 != argc) strcat(line, " ");
  }
  
  // invoke complete_word and receive a list of matches.
  matches = cpl_complete_word(wc, line, strlen(line), NULL, cli_compl);
  cpl_list_completions(matches, stdout, 50);
  
  del_WordCompletion(wc);	
  
  return 0;
}
