#include "DrvUART010.h"	
#include "Common_Include.h"
#include "types.h"
#include "io.h"

#define TIMING_1_REG				0x0C
#define TIMING_2_REG 				0x04
#define CTRL_TIMING_3_REG 	0x08
#define PAGE_DAT_STATUS			0x0C
#define PROTECT_REG 				0x10
#define UNLOCK_REG 					0x14

//#define fPCLK  100       // PLL1  100Mhz
#define fPCLK  25       // XTAL 25 system clock of efuse controller
#define tF (1000/fPCLK)

//for read
#define tRAS     7       // No the timing
#define tESR     7       // tESP
#define tRPW     35      // tRP
#define tRC      35      // tRP+tAC+1T
//for write
#define tEPS     7       // tESP+tFS
#define tPP      4000    // tPP
#define tPI      1000    // tPI
#define tPD      500     // tPI
// for 40LP efuse
#define  tPS_CS  57      //
#define  tPD_PS  500     //

//for read
#define  CtRAS    ((tF<tRAS)? 0x01 : 0x0)          // CtRAS <10ns to shift 1 cycles by PCLK, when set to 1
#define  CtESR    ((tF<tESR)? 0x0 : (tESR/tF))                     // 55ns;
#define  CtRPW    ((tRPW/tF)+(CtRAS+1))            // 35ns + (CtRAS+1tF), for CtRAS
#define  CtRC     ((tRC-tRPW)/tF)                  // (70-35)ns
//for write
#define  CtEPS    (CtESR>(tEPS/tF)? CtESR: (tEPS/tF))  // 55ns;
#define  CtPP     ((tPP/tF)+1)                  // 9000ns +1tF (included 1T for tAS)
#define  CtPI     (tPI/tF)                    // 800ns
#define  CtPD     (tPD/tF)                  // 500ns
// for 40LP efuse
#define  CtPS_CS  (tPS_CS/tF)
#define  CtPD_PS  (tPD_PS/tF)

#define  tBIT     16

#define xPCLK    12      // 25MHz, efuse clock
#define xF      (1000/xPCLK)

//eFuse timing parameter setting and control
//for read
#define xRAS     ((xF<tRAS)?0x01: 0x0)             // eFuse read signal is shifted cycles by PCLK
#define xESR     ((xF<tESR)?0x0: (tESR/xF))        //9'h001;  // 55ns;
#define xRPW     ((tRPW/xF)+(xRAS+1))              //9'h002;  // (35/35)+1(included 1T for tERS
#define xRC      ((tRC-tRPW)/xF)                    //9'h000;  // (70-35)/35
//for write
#define xEPS     (xESR>(tEPS/xF)? xESR: (tEPS/xF))  //9'h001;  // 55ns;
#define xPP      ((tPP/xF)+1)                        //9'h102;  // (9000/35)+1(included 1T for tAS)
#define xPI      (tPI/xF)                         //9'h016;  // 800/35
#define xPD      ((tPD/xF)+1)                        // 500ns
// for 40LP efuse
#define xPS_CS  (tPS_CS/xF)
#define xPD_PS  (tPD_PS/xF)

/*Kneron programming: use this very carefully (eFuse_BASE + 0x4)*/
#define eFuse_IPL_XIP_DISABLE				0x8
#define eFuse_SPL_UART_DISABLE			0x4
#define eFuse_CPU_JTAG_DISABLE			0x2
#define eFuse_HIF_DISABLE						0x1

void eFuse_main(void)
{
	char key;
	int data;
	fLib_printf("eFuse Test Main\n");
	
	//1. ======== check register initial value ========
	data = ((CtEPS & 0xff) <<24) | ((CtPI&0xff) <<16) | (CtPP&0xffff);
	fLib_printf("data = 0x%x, rd=0x%x\n", data, inw(EFUSE_PA_BASE+0x1000));

	data = ((CtRC & 0xffff) <<16) | ((CtRAS & 0xff) <<8) | (CtRPW & 0xff);
	fLib_printf("data = 0x%x, rd=0x%x\n", data, inw(EFUSE_PA_BASE+0x1004));
	
	data = ((CtPD & 0xffff) <<16) | ((CtPS_CS & 0xf) <<12) | ((0x0 & 0xf) <<8) | ((CtPD_PS & 0xf) <<4) | (0x0 & 0xf);
	fLib_printf("data = 0x%x, rd=0x%x\n", data, inw(EFUSE_PA_BASE+0x1008));

	outw(EFUSE_PA_BASE+0x1000, 0xFFFFFFFF);
	fLib_printf("rd=0x%x\n", data, inw(EFUSE_PA_BASE+0x1000));

	outw(EFUSE_PA_BASE+0x1004, 0xFFFFFFFF);
	fLib_printf("rd=0x%x\n", data, inw(EFUSE_PA_BASE+0x1004));
	
  outw(EFUSE_PA_BASE+0x1008, 0xfffff0f0);
	fLib_printf("rd=0x%x\n", data, inw(EFUSE_PA_BASE+0x1008));

	fLib_printf("write to 0x1000\n");
	data = ((xEPS & 0xff) <<24) | ((xPI & 0xff) <<16) | (xPP & 0xffff);
	outw(EFUSE_PA_BASE+0x1000, data);
	fLib_printf("data = 0x%x, rd=0x%x\n", data, inw(EFUSE_PA_BASE+0x1000));
	
	fLib_printf("write to 0x1004\n");
	data = ((xRC & 0xf) << 16) | (xRAS << 8) | (xRPW & 0xf) ;
	outw(EFUSE_PA_BASE+0x1004, data);
	fLib_printf("data = 0x%x, rd=0x%x\n", data, inw(EFUSE_PA_BASE+0x1004));

	fLib_printf("write to 0x1008\n");
	data = ((xPD & 0xffff) <<16) | ((xPS_CS & 0xf) <<12) | ((xPD_PS & 0xf) <<4 );
	outw(EFUSE_PA_BASE+0x1008, data);
	fLib_printf("data = 0x%x, rd=0x%x\n", data, inw(EFUSE_PA_BASE+0x1008));
	
	//while(1);
	
	fLib_printf("Protect:\n");
	fLib_printf("Write data 0x55 to PROTECT_KEY register\n");
	outw(EFUSE_PA_BASE + 0x1000 + PROTECT_REG, 0x55);
	fLib_printf("Write data 0xaa to PROTECT_KEY register\n");
	outw(EFUSE_PA_BASE + 0x1000 + PROTECT_REG, 0xaa);
	if(inw(EFUSE_PA_BASE + 0x1000 + PROTECT_REG)==0x1)
	{
		fLib_printf("(In Unprotected State)\n");
	}
	else
	{
		fLib_printf("## Protected! ##\n");
		while(1);
	}
	fLib_printf("(In Lock State) - Enter Q to unlock eFuse R/W\n");
		
	while(1){
		key = fLib_getchar(DEBUG_CONSOLE);
		if (key==0x1b)
			break;
		if (key=='q'){
			fLib_printf("Unlocked!\n");
			fLib_printf("Disable Host SPI I/F\n");
		#if 0	//use this carefully, open when you need it
			outw(EFUSE_PA_BASE + 0x1000 + UNLOCK_REG, 0x40);
			outw(EFUSE_PA_BASE + 0x4, eFuse_HIF_DISABLE);
		#endif
			outw(EFUSE_PA_BASE + 0x1000 + UNLOCK_REG, 0x40);
			fLib_printf("Read data=0x%x\n", inw(EFUSE_PA_BASE + 0x4));
			break;
		}
	}

	fLib_printf("Test END...\n");
}