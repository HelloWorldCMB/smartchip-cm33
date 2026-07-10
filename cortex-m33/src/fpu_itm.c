#include "cmsis_os2.h"                  // ARM::CMSIS:RTOS2:Keil RTX5
#include "Common_Include.h"
#include <stdio.h>
#include <math.h>
#include "arm_math.h"

extern void delay_ms(unsigned int count);

void itm_test(void)
{
	int i=0;
	while(i<400)
		printf("THIS IS ITM %d\n",i++);
	  delay_ms(1);
}

const float32_t testInput_f32[32] =
{
  -1.244916875853235400,  -4.793533929171324800,   0.360705030233248850,   0.827929644170887320,  -3.299532218312426900,   3.427441903227623800,   3.422401784294607700,  -0.108308165334010680,
   0.941943896490312180,   0.502609575000365850,  -0.537345278736373500,   2.088817392965764500,  -1.693168684143455700,   6.283185307179590700,  -0.392545884746175080,   0.327893095115825040,
   3.070147440456292300,   0.170611405884662230,  -0.275275082396073010,  -2.395492805446796300,   0.847311163536506600,  -3.845517018083148800,   2.055818378415868300,   4.672594161978930800,
  -1.990923030266425800,   2.469305197656249500,   3.609002606064021000,  -4.586736582331667500,  -4.147080139136136300,   1.643756718868359500,  -1.150866392366494800,   1.985805026477433800
};

void FPU_test_main(void)
{
	UINT32 data;
	
	data = inw(0xE000ED88); // read CPACR
	outw(0xE000ED88, data | 0xF00000);
	//FPU test
	fLib_printf("FPU test: Enable FPU\n");
	int i = 0; 
	float32_t  cosOutput;
	for(i = 0; i < 32; i++){
		cosOutput = arm_cos_f32(testInput_f32[i]);
		fLib_printf("cosine[%d] example = %f\n", i, cosOutput);
	}
}
