#include "DrvPWMTMR010.h"
#include "DrvUART010.h"
#include "leo_cm33.h"
#include "io.h"
#include "types.h"
#include <stdio.h>
#include "DrvKBC010.h"

extern int ftkbc010_init(void);
extern void ftkbc010_disable(void);

void kbc_test_main(void)
{
	int i;
	char chr;
	
	fLib_printf("%s\n",__func__);
  ftkbc010_init();
	fLib_printf("Press ESC to end kbc test\n");
	while(1){
			chr = fLib_getch(DEBUG_CONSOLE);	//Press "ESC" key to terminate GPIO test	
        if (chr==0x1b)
		{
			break;
		}
	}
	
	ftkbc010_disable();
}