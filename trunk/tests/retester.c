#include <stdio.h>
#include <regex.h>
#include <string.h>
#include <stdlib.h>

int main(int argc, char* argv[])
{
	regex_t re;
	regmatch_t matches[100];
	char input[100];
	char* next;
	int idx = 2;
	
	if (argc < 2)
	{
		printf("usage: %s <pattern> [ <input> <input> <input> ]\n", argv[0]);
		printf("if <input> not specified, input is read from stdin.\n");
		exit(-1);
	}
	
	if (regcomp(&re, argv[1], REG_EXTENDED) != 0)
	{
		printf("invalid pattern.\n");
		exit(-1);
	}


	while (1)
	{
		if (argc > 2)
		{
			if (idx < argc) next = argv[idx++];
			else break;
		}
		else
		{
			scanf("%s", input);
			next = input;
		}
		
		if (regexec(&re, next, 100, matches, 0) != 0)
		{
			printf("%s: no matches.\n", next);
			continue;
		}

		{
			int i;
			for (i = 0; i < 100; ++i)
			{
				if (matches[i].rm_so != -1)
				{
					printf("%s: match[%d]:%d-%d\n", next, i, matches[i].rm_so, matches[i].rm_eo);
				}
			}
		}
	}
	

	
	return 0;
}

