#ifndef UTILITY_H
#define UTILITY_H


#ifdef __cplusplus
extern "C" {
#endif

#include "types.h"

#define divRoundDown(n,s)   ((n) / (s))
#define divRoundUp(n,s)     ((n+s-1)/(s))
#define RoundUp(val, units) \
		((((unsigned long)(val) + ((units) - 1)) / (units)) * (units))
#define RoundDown(val, units) \
		(((unsigned long)(val)/(units))*(units))			
			
#define ARRAY_SIZE(x) 		(sizeof(x) / sizeof((x)[0]))		
		
int substring(char **ptr, char *string, char *pattern);
unsigned int atonum(char *val_str);
		
		
#ifdef __cplusplus
}
#endif



#endif
