#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "io.h"
#include "types.h"
#include "leo_cm33.h"
#include "utility.h"

int substring(char **ptr, char *string, char *pattern)
{   
    int i;

    ptr[0]=(char *)strtok(string, pattern);
    for (i=0;ptr[i]!=NULL;++i)
    {   
    	ptr[i+1]=strtok(NULL,pattern);
    }
    return i;
}

unsigned int atonum(char *val_str)
{
	unsigned int address;

    if (val_str[0] =='0' && val_str[1] == 'x')
    {
		sscanf(val_str, "%x\n", &address);//bessel:convert to hexidecimal value
		//ASSERT(0);
	}
    else
    {
		address = atoi(val_str);
	}
    return address;
}

