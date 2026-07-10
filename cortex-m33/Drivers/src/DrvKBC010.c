#include "io.h"
#include "leo_cm33.h"
#include "Driver_USART.h"
#include "DrvkBC010.h"
	 
#define MAX_KEY_ROWS 8
#define MAX_KEY_COLS 8
#define MAX_KEYPAD_KEYS (MAX_KEY_ROWS * MAX_KEY_COLS)

#define MATRIX_SCAN_CODE(row, col, row_shift)	(((row) << (row_shift)) + (col))
extern void *malloc  (unsigned int size);

struct ftkbc010 {
   unsigned short keycode[MAX_KEYPAD_KEYS];
   unsigned int base;
   int irq;
   unsigned int rows;
   unsigned int cols;
   unsigned int row_shift;
};

struct ftkbc010 *ftkbc010;


int ffs(unsigned int val)
{
  int i;

  if(val == 0){
     return 0;
  }
  for(i=0;i<31;i++){
    if((val&0x01)==0x01){
       return i+1;
    }
    val = val >> 1;
  }
	return 0;//should not come here
}

/******************************************************************************
 * internal functions
 *****************************************************************************/
static void ftkbc010_enable(void)
{
   int reg;

   reg = FTKBC010_ASP_PERIOD(0x98fff);

   fLib_printf("[ASP] = %08x\n", reg);
   outw(FTKBC010_PA_BASE + FTKBC010_OFFSET_ASP , reg);

   reg = FTKBC010_CR_ENABLE
       | FTKBC010_CR_EN_TXINT
       | FTKBC010_CR_EN_RXINT
       | FTKBC010_CR_CL_TXINT
       | FTKBC010_CR_CL_RXINT
       | FTKBC010_CR_EN_PAD
       | FTKBC010_CR_AUTOSCAN
       | FTKBC010_CR_CL_PADINT;

   fLib_printf("[CR]  = %08x\n", reg);
   outw(FTKBC010_PA_BASE + FTKBC010_OFFSET_CR , reg);
	 NVIC_EnableIRQ(KBC_FTKBCC010_IRQ);
}

void ftkbc010_disable(void)
{
   iowrite32(FTKBC010_PA_BASE + FTKBC010_OFFSET_CR , 0);
	 NVIC_DisableIRQ(KBC_FTKBCC010_IRQ);
}

/******************************************************************************
 * interrupt handler
 *****************************************************************************/
void ftkbc010_interrupt(void)
{
   unsigned int status;
   unsigned int cr;

   status = inw(FTKBC010_PA_BASE + FTKBC010_OFFSET_ISR);

   /*
    * The clear interrupt bits are inside control register,
    * so we need to read it out and then modify it -
    * completely brain-dead design.
    */
   cr = inw(FTKBC010_PA_BASE + FTKBC010_OFFSET_CR);

   if (status & FTKBC010_ISR_RXINT) {
      //dev_dbg(&input->dev, "RXINT\n");
      cr |= FTKBC010_CR_CL_RXINT;
   }

   if (status & FTKBC010_ISR_TXINT) {
      //dev_dbg(&input->dev, "TXINT\n");
      cr |= FTKBC010_CR_CL_TXINT;
   }

   if (status & FTKBC010_ISR_PADINT) {
      unsigned int data;
      unsigned int row, col;
      unsigned int scancode;
      unsigned int keycode;

      data = inw(FTKBC010_PA_BASE + FTKBC010_OFFSET_XC);
      col = ffs(0xff - FTKBC010_XC_L(data)) - 1;

      data = inw(FTKBC010_PA_BASE + FTKBC010_OFFSET_YC);
      row = ffs(0xff - FTKBC010_YC_L(data)) - 1;

      scancode = MATRIX_SCAN_CODE(row, col, ftkbc010->row_shift);
			fLib_printf("row %d col %d scancode %x\n",row,col,scancode);
      keycode = ftkbc010->keycode[scancode];
			
//      input_report_key(input, keycode, 1);
//      input_sync(input);

      /* We cannot tell key release - simulate one */
//      input_report_key(input, keycode, 0);
//      input_sync(input);

      //dev_info(&input->dev, "(%x, %x) scancode %d, keycode %d\n",
      // row, col, scancode, keycode);

      cr |= FTKBC010_CR_CL_PADINT;
   }

   outw(FTKBC010_PA_BASE + FTKBC010_OFFSET_CR , cr);
}

int ftkbc010_init(void)
{
	ftkbc010 = malloc(sizeof(struct ftkbc010));
  if (!ftkbc010) {
      fLib_printf("Failed to allocate memory.\n");
      return -1;
  }	
	ftkbc010->base = FTKBC010_PA_BASE;
	ftkbc010->rows = 8;
	ftkbc010->cols = 8;
	ftkbc010->row_shift = 8;
	
	ftkbc010_enable();
	return 0;
}