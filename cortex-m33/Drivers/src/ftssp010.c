/**
 * -------------------------------------------------------------------------
 * 	Copyright  Faraday Technology Corp.  All rights reserved.
 * -------------------------------------------------------------------------
 * FILENAME:  ftssp010.c
 * DEPARTMENT :CTD/SD
 * VERSION: Revision:0.1
 * -------------------------------------------------------------------------
 *  MAJOR REVISION HISTORY
 * DATE        	AUTHOR       DESCRIPTION
 * 2015/06      BingYao      commands to do verification
 * -------------------------------------------------------------------------
 */

#include "ftssp010.h"
#include "io.h"
#include "types.h"
#include "Common_Include.h"
#include "DrvDMAC020.h"
#include "DrvSSP010.h"

extern void delay_ms(unsigned int count);
extern void switch_mode_register(int mode);

unsigned int FTSSP010_REG_BASE_M = SPI_FTSSP010_0_PA_BASE;//SPI_FTSSP010_2_PA_BASE;
int spi_choice;
int sclkdiv = 15;//25600;//15; /*7:650kHZ*/
int debug = 0;
int sdl_in_bytes = 12;
int pcl = /*0*/8;

/*SPI2AHB*/
#define HSPI_RD_CMD (0 << 31)
#define HSPI_WR_CMD (1 << 31)

const char *ffmt_str[] = { "SSP", "SPI", "MicroWire", "I2S", "ACL", "SPDIF" };

char * ftssp010_get_fformat_string(int base)
{
	int cr0;

	cr0 = inl(base + FTSSP010_REG_CR0);
	cr0 = (cr0 >> 12) & 0x7;

	return (char*)ffmt_str[cr0];
}


void ftssp010_set_sclkdiv(int base)
{
	int cr1;

	cr1 = inl(base + FTSSP010_REG_CR1);
	cr1 &= ~FTSSP010_CR1_SCLKDIV_MASK;
	cr1 |= FTSSP010_CR1_SCLKDIV(sclkdiv);

	outl(cr1, base + FTSSP010_REG_CR1);
}

void ftssp010_ssp_reset(int base_addr)
{
	int cr2;

	cr2 = inl(base_addr + FTSSP010_REG_CR2);

	cr2 |= FTSSP010_CR2_SSPRST;

	outl(cr2, base_addr + FTSSP010_REG_CR2);
}

void ftssp010_clear_txfifo(int base_addr)
{
	int cr2;

	cr2 = inl(base_addr + FTSSP010_REG_CR2);

	cr2 |= FTSSP010_CR2_TXFCLR;

	outl(cr2, base_addr + FTSSP010_REG_CR2);
}

void ftssp010_clear_rxfifo(int base_addr)
{
	int cr2;

	cr2 = inl(base_addr + FTSSP010_REG_CR2);

	cr2 |= FTSSP010_CR2_RXFCLR;

	outl(cr2, base_addr + FTSSP010_REG_CR2);
}
/**
 * CR1.SDL has 7 bits means maximum: 2^7 = 127 + 1 bits.
 * 128 bits = 16 bytes
 */
int ftssp010_set_data_length(int base_addr, int sdl)
{
	int cr1;
fLib_printf("1sdl=%x\n", sdl);
	//convert to bits;
	sdl <<= 3;
	sdl -= 1;
	if (sdl & ~FTSSP010_SDL_MAX_BYTES_MASK) {
		fLib_printf(" sdl val range from 1 to 16 bytes\n");
		return 1;
	}
//sdl=0x40;
	cr1 = inl(base_addr + FTSSP010_REG_CR1);
	cr1 &= ~FTSSP010_CR1_SDL_MASK;

	cr1 |= FTSSP010_CR1_SDL(sdl);
fLib_printf("sdl=%x\n", sdl);
	outl(cr1, base_addr + FTSSP010_REG_CR1);

	fLib_printf(" Addr 0x%x CR1 0x%x, sdl %d\n", base_addr,
		inl(base_addr + FTSSP010_REG_CR1), ((sdl+1)>>3));

	return 0;
}
void ftssp010_enable(int base_addr, int tx, int rx)
{
	int cr2 = 0;

	if (tx || rx)
		cr2 = (FTSSP010_CR2_SSPEN | FTSSP010_CR2_TXDOE);

	if (tx)
		cr2 |= FTSSP010_CR2_TXEN;

	if (rx)
		cr2 |= FTSSP010_CR2_RXEN;

	outl(cr2, base_addr + FTSSP010_REG_CR2);

	if (debug)
		fLib_printf(" Addr 0x%x CR2 0x%x\n", base_addr,
			inl(base_addr + FTSSP010_REG_CR2));
}

void ftssp010_enable_FS(int base_addr, int tx, int rx, int fs)
{
	int cr2 = 0;

	if (tx || rx)
		cr2 = (FTSSP010_CR2_SSPEN | FTSSP010_CR2_TXDOE);

	if (tx)
		cr2 |= FTSSP010_CR2_TXEN;

	if (rx)
		cr2 |= FTSSP010_CR2_RXEN;

	if (fs)
		cr2 |= FTSSP010_CR2_FS;

	outl(cr2, base_addr + FTSSP010_REG_CR2);

	if (debug)
		fLib_printf(" Addr 0x%x CR2 0x%x\n", base_addr,
			inl(base_addr + FTSSP010_REG_CR2));
}

/**
 * Return the number of entries TX FIFO can be written to
 */
int ftssp010_txfifo_depth(int base_addr)
{
	int depth;

	depth = FTSSP010_FEA_TXFIFO_DEPTH(
		inl(base_addr + FTSSP010_REG_FEATURE));
	depth += 1;
	return depth;
}

int ftssp010_txfifo_not_full(int base_addr)
{
	int sts;

	sts = inl(base_addr + FTSSP010_REG_STS);

	return (sts & FTSSP010_STS_TFNF);
}

int ftssp010_txfifo_valid_entries(int base_addr)
{
	return FTSSP010_STS_TFVE(inl(base_addr + FTSSP010_REG_STS));

}

/**
 * Return the number of entries RX FIFO can be written to
 */
int ftssp010_rxfifo_depth(int base_addr)
{
	int depth;

	depth = FTSSP010_FEA_RXFIFO_DEPTH(
		inl(base_addr + FTSSP010_REG_FEATURE));
	depth += 1;
	return depth;
}

int ftssp010_rxfifo_full(int base_addr)
{
	int sts;

	sts = inl(base_addr + FTSSP010_REG_STS);

	return (sts & FTSSP010_STS_RFF);
}

int ftssp010_rxfifo_valid_entries(int base_addr)
{
	int ent;

	ent = FTSSP010_STS_RFVE(inl(base_addr + FTSSP010_REG_STS));

	return ent;
}

/*SPI2AHB*/
void ftssp010_SPI2AHB_master_init(int cr0)
{
	ftssp010_ssp_reset(FTSSP010_REG_BASE_M);

	cr0 = cr0 | (FTSSP010_CR0_FFMT_SPI | FTSSP010_CR0_MSTR_SPI | FTSSP010_CR0_SPI_FLASH | FTSSP010_CR0_FSPO) /*| FTSSP010_CR0_LSB*/;
	//cr0 = 0x8102f;

	outl(cr0, FTSSP010_REG_BASE_M + FTSSP010_REG_CR0);

	ftssp010_set_sclkdiv(FTSSP010_REG_BASE_M);
	//outl(0x1f07cf, FTSSP010_REG_BASE_M + FTSSP010_REG_CR1);

	ftssp010_enable_FS(FTSSP010_REG_BASE_M, 0, 0, 1);

	ftssp010_set_data_length(FTSSP010_REG_BASE_M, sdl_in_bytes);

	ftssp010_set_pcl(FTSSP010_REG_BASE_M, pcl);
	//outl(0x0, FTSSP010_REG_BASE_M + FTSSP010_REG_CR3);

	fLib_printf(" Master CR0 0x%x\n", inl(FTSSP010_REG_BASE_M +
					FTSSP010_REG_CR0));
	fLib_printf(" Master CR1 0x%x\n", inl(FTSSP010_REG_BASE_M +
					FTSSP010_REG_CR1));
	fLib_printf(" Master CR2 0x%x\n", inl(FTSSP010_REG_BASE_M +
					FTSSP010_REG_CR2));
	fLib_printf(" Master CR3 0x%x\n", inl(FTSSP010_REG_BASE_M +
					FTSSP010_REG_CR3));
}

int ftssp010_SSP_busy(int base_addr)
{
	return (((inw(base_addr + FTSSP010_REG_STS) ) >> 2) & 0x1);
}

void SPI2AHB_Write(int base, UINT32 wr_buf, UINT32 byte_addr, UINT32 num)
{
	UINT32 cur_addr, write_cmd1, write_cmd2, write_cmd3;

	fLib_printf("SPI2AHB: single word write, byte_addr = 0x%x, buf = %x\n", byte_addr, wr_buf);

	ftssp010_clear_txfifo(base);
	ftssp010_clear_rxfifo(base);

	while(!ftssp010_txfifo_not_full(base))
			;

	write_cmd1 = HSPI_WR_CMD | (byte_addr >> 1);
	write_cmd2 = wr_buf >> 1;
	if (wr_buf & 0x1)
		write_cmd3 = 0x80000000; // wr_buf bit 0
	else
		write_cmd3 = 0;


	//fLib_printf("write_cmd1 = 0x%x, write_cmd2 = 0x%x, write_cmd3 = 0x%x\n", write_cmd1, write_cmd2, write_cmd3);

	outw(base + FTSSP010_REG_DATA_PORT, write_cmd3); // wr_buf + Dummy
	outw(base + FTSSP010_REG_DATA_PORT, write_cmd2); // addr + wr_buf
	outw(base + FTSSP010_REG_DATA_PORT, write_cmd1); // write CMD + addr

	ftssp010_enable_FS(base, 1, 1, 0);
	//ftssp010_enable_FS(base, 1, 0, 0);

	outw(base + FTSSP010_REG_DATA_PORT, 0); // addr + wr_buf
	while (ftssp010_txfifo_valid_entries(base))
			;

	ftssp010_enable_FS(base, 0, 0, 1);
}

#define BUF_SIZE 96
void spi2ahb_dma_read(int base, UINT32 *buf, struct ssp_dma_req_mode *req_mode, int len , int tx)
{
	int i;
	char chr;
	int src_width = 2;//source width 8/16/32 bits -> 0/1/2
	int dst_width = 2;//dst width 8/16/32 bits -> 0/1/2
	int burst_size = 0;
	int src_ctrl = 2 ;// Inc/dec/fixed --> 0/1/2
	int dst_ctrl = 0 ;// Inc/dec/fixed --> 0/1/2
	int priority = 0;
	int hw = 1;

	if(tx == 1){
		src_ctrl = 0 ;// Inc/dec/fixed --> 0/1/2
		dst_ctrl = 2 ;// Inc/dec/fixed --> 0/1/2
	}
	//prepare data bufer for dmac020

	//switch system mode register
	switch_mode_register(req_mode->mode);
	//send some data by using dma020 API;

  fLib_InitDMA(FALSE, FALSE, 0x0);
	fLib_DMA_ClearAllInterrupt();
	fLib_DMA_EnableDMAInt(); //Enable DMA Interrupt
	if(tx == 1)
		fLib_DMA_NormalMode(0, (UINT32)buf, base+0x18, len, src_width , dst_width, burst_size, src_ctrl,dst_ctrl ,priority, hw,req_mode->dma_txreq);
	else
		fLib_DMA_NormalMode(0,base+0x18, (UINT32)buf, len, src_width , dst_width, burst_size, src_ctrl,dst_ctrl ,priority, hw,req_mode->dma_txreq);

	fLib_EnableDMAChannel(0);
	fLib_DMA_WaitDMAInt(0);
	fLib_DisableDMAChannel(0);
 // md((UINT32)buf,BUFSIZ);
}

void i2s_dma(int base, UINT32 *buf, int dma_req, int len , int tx)
{
	int i;
	char chr;
	int src_width = 2;//source width 8/16/32 bits -> 0/1/2
	int dst_width = 2;//dst width 8/16/32 bits -> 0/1/2
	int burst_size = 0;
	int src_ctrl = 2 ;// Inc/dec/fixed --> 0/1/2
	int dst_ctrl = 0 ;// Inc/dec/fixed --> 0/1/2
	int priority = 0;
	int hw = 1;
	int mode = 2; //this is scu hardware handshake mode switch(0x50001000)

	//prepare data bufer for dmac020
	if(tx == 1){ //playing
		fLib_printf("@@tx %x\n",tx);
		src_width = 2;//source width 8/16/32 bits -> 0/1/2
		dst_width = 2;//dst width 8/16/32 bits -> 0/1/2
		burst_size = 0;
		src_ctrl = 0 ;// Inc/dec/fixed --> 0/1/2
		dst_ctrl = 2 ;// Inc/dec/fixed --> 0/1/2
	}
	//switch system mode register
	switch_mode_register(mode);
	//send some data by using dma020 API;

  fLib_InitDMA(FALSE, FALSE, 0x0);
	fLib_DMA_ClearAllInterrupt();
	fLib_DMA_EnableDMAInt(); //Enable DMA Interrupt

	if(tx == 1)
		fLib_DMA_NormalMode(0,(UINT32)buf, base + SSP_DATA, len, src_width , dst_width, burst_size, src_ctrl,dst_ctrl ,priority, hw,dma_req);
	else //recording
		fLib_DMA_NormalMode(0,base + SSP_DATA, (UINT32)buf, len, src_width , dst_width, burst_size, src_ctrl,dst_ctrl ,priority, hw,dma_req);

	fLib_EnableDMAChannel(0);
	fLib_DMA_WaitDMAInt(0);
	fLib_DisableDMAChannel(0);
 // md((UINT32)buf,BUFSIZ);
}

void SPI2AHB_Find_DMA_REQ(UINT32 base, struct ssp_dma_req_mode *ptr)
{
	int choice;
	char buf[64];

	switch(base)
	{
		case SPI_FTSSP010_0_PA_BASE:
			ptr->dma_txreq = SSP_FTSSP010_0_DMA_TXREQ;
		  ptr->dma_rxreq = SSP_FTSSP010_0_DMA_RXREQ;
		  ptr->mode = SSP_FTSSP010_0_DMA_MODE_MUX;
			break;

		case SPI_FTSSP010_1_PA_BASE:
			ptr->dma_txreq = SSP_FTSSP010_1_DMA_TXREQ;
		  ptr->dma_rxreq = SSP_FTSSP010_1_DMA_RXREQ;
		  ptr->mode = SSP_FTSSP010_1_DMA_MODE_MUX;
			break;

		case SPI_FTSSP010_2_PA_BASE:
			ptr->dma_txreq = SSP_FTSSP010_2_DMA_TXREQ;
		  ptr->dma_rxreq = SSP_FTSSP010_2_DMA_RXREQ;
		  ptr->mode = SSP_FTSSP010_2_DMA_MODE_MUX;

			break;
		case SPI_FTSSP010_3_PA_BASE:
		  switch(spi_choice){
				case 1:
					ptr->dma_txreq = SSP_FTSSP010_3_DMA_TXREQ;
					ptr->dma_rxreq = SSP_FTSSP010_3_DMA_RXREQ;
					ptr->mode = SSP_FTSSP010_3_DMA_MODE_MUX;
				break;
				default:
					ptr->dma_txreq = SSP_FTSSP010_3_DMA_TXREQ_MODE3;
					ptr->dma_rxreq = SSP_FTSSP010_3_DMA_RXREQ_MODE3;
					ptr->mode = SSP_FTSSP010_3_DMA_MODE_MUX_MODE3;
			}

			break;

		case SPI_FTSSP010_4_PA_BASE:
		  switch(spi_choice){
				case 1:
					ptr->dma_txreq = SSP_FTSSP010_4_DMA_TXREQ;
					ptr->dma_rxreq = SSP_FTSSP010_4_DMA_RXREQ;
					ptr->mode = SSP_FTSSP010_4_DMA_MODE_MUX;
				break;
				default:
					ptr->dma_txreq = SSP_FTSSP010_4_DMA_TXREQ_MODE3;
					ptr->dma_rxreq = SSP_FTSSP010_4_DMA_RXREQ_MODE3;
					ptr->mode = SSP_FTSSP010_4_DMA_MODE_MUX_MODE3;
			}
			break;

		case SPI_FTSSP010_5_PA_BASE:
			ptr->dma_txreq = SSP_FTSSP010_5_DMA_TXREQ;
		  ptr->dma_rxreq = SSP_FTSSP010_5_DMA_RXREQ;
		  ptr->mode = SSP_FTSSP010_5_DMA_MODE_MUX;
			break;

		case SPI_FTSSP010_6_PA_BASE:
			ptr->dma_txreq = SSP_FTSSP010_6_DMA_TXREQ;
		  ptr->dma_rxreq = SSP_FTSSP010_6_DMA_RXREQ;
		  ptr->mode = SSP_FTSSP010_6_DMA_MODE_MUX;
			break;
	}
	//fLib_printf("%s txreq %x rxreq %x mode %x\n",__func__,ptr->dma_txreq,ptr->dma_rxreq,ptr->mode);
}

void SPI2AHB_Read_Dma(UINT32 base, UINT32 *buf, UINT32 byte_addr)
{
	UINT32 i, tmp1, tmp2;
	UINT32 tx_buf[3];
	struct ssp_dma_req_mode req_mode;
	int tx;

	ftssp010_clear_txfifo(base);
	ftssp010_clear_rxfifo(base);
	fLib_SetSSP_DMA(base, 1,0);

	i = 0;
	while(!ftssp010_txfifo_not_full(base))
		;

	tx_buf[0] = 0x11223344;
	tx_buf[1] = 0x0;
	tx_buf[2] = HSPI_RD_CMD | ((byte_addr + i*4) >> 1);
#if 0
	outw(base + FTSSP010_REG_DATA_PORT, tx_buf[0]); // Dummy
	outw(base + FTSSP010_REG_DATA_PORT, tx_buf[1]); // read CMD
	outw(base + FTSSP010_REG_DATA_PORT, tx_buf[2]); // read CMD
#else
//Enable SSP1
	fLib_SetSSP_Enable(base, 1);
	SPI2AHB_Find_DMA_REQ(base, &req_mode);

	tx = 1;
  spi2ahb_dma_read(base, tx_buf,&req_mode, 12, tx);
//Enable SSP1
	fLib_SetSSP_Enable(base, 0);

#endif
	i = 0;

	ftssp010_enable_FS(base, 1, 1, 0);
	outw(base + FTSSP010_REG_DATA_PORT, 0); // addr + wr_buf
	while (!ftssp010_rxfifo_valid_entries(base))
		;

#if 0
	tmp1 = (UINT32)(inw(base + FTSSP010_REG_DATA_PORT)); // Dummy read
	buf[i] = (UINT32)(inw(base + FTSSP010_REG_DATA_PORT)); // Real data
	ftssp010_enable_FS(base, 0, 0, 1);

	buf[i] = (buf[i]<<1) | ((tmp1 & 0x80000000)? 0x1:0x0);

#else
	tx = 0; //read from ssp
	spi2ahb_dma_read(base, buf, &req_mode, 8, tx);
	buf[0] = (buf[1]<<1) | ((buf[0] & 0x80000000)? 0x1:0x0);
	ftssp010_enable_FS(base, 0, 0, 1);
#endif
}

void SPI2AHB_Read(UINT32 base, UINT32 *buf, UINT32 byte_addr)
{
	UINT32 i, tmp1, tmp2;
	UINT32 read_cmd1, read_cmd2;

	ftssp010_clear_txfifo(base);
	ftssp010_clear_rxfifo(base);

	i = 0;
	while(!ftssp010_txfifo_not_full(base))
		;

	read_cmd1 = HSPI_RD_CMD | ((byte_addr + i*4) >> 1);
	read_cmd2 = 0x0;
#if 1
	outw(base + FTSSP010_REG_DATA_PORT, 0x112233ff); // Dummy
	outw(base + FTSSP010_REG_DATA_PORT, read_cmd2); // read CMD
	outw(base + FTSSP010_REG_DATA_PORT, read_cmd1); // read CMD
#else
	outw(base + FTSSP010_REG_DATA_PORT, read_cmd1); // read CMD
	outw(base + FTSSP010_REG_DATA_PORT, read_cmd2); // read CMD
	outw(base + FTSSP010_REG_DATA_PORT, 0x112233ff); // Dummy



	#endif

	ftssp010_enable_FS(base, 1, 1, 0);
	outw(base + FTSSP010_REG_DATA_PORT, 0); // addr + wr_buf
	while (!ftssp010_rxfifo_valid_entries(base))
		;

#if 1
	tmp1 = (UINT32)(inw(base + FTSSP010_REG_DATA_PORT)); // Dummy read
	buf[i] = (UINT32)(inw(base + FTSSP010_REG_DATA_PORT)); // Real data
#else
	buf[i] = (UINT32)(inw(base + FTSSP010_REG_DATA_PORT)); // Real data
	tmp1 = (UINT32)(inw(base + FTSSP010_REG_DATA_PORT)); // Dummy read

#endif
	ftssp010_enable_FS(base, 0, 0, 1);

	buf[i] = (buf[i]<<1) | ((tmp1 & 0x80000000)? 0x1:0x0);
  fLib_printf("buf %x\n",buf[0]);
}
#if 0
void SPI2AHB_Multi_Read(UINT32 base, UINT32 *buf, UINT32 byte_addr, UINT32 num)
{
	UINT32 i, tmp1, tmp2;
	UINT32 cur_addr, read_cmd1, read_cmd2;

  for(i=0;i<num;i++)
	{
		ftssp010_clear_txfifo(base);
		ftssp010_clear_rxfifo(base);
		cur_addr=byte_addr+i;

		while(!ftssp010_txfifo_not_full(base))
			;

		read_cmd1 = HSPI_RD_CMD | ((byte_addr + i*4) >> 1);
		read_cmd2 = 0x0;

		outw(base + FTSSP010_REG_DATA_PORT, 0x112233ff); // Dummy
		outw(base + FTSSP010_REG_DATA_PORT, read_cmd2); // read CMD
		outw(base + FTSSP010_REG_DATA_PORT, read_cmd1); // read CMD

		ftssp010_enable_FS(base, 1, 1, 0);
		outw(base + FTSSP010_REG_DATA_PORT, 0); // addr + wr_buf
		while (!ftssp010_rxfifo_valid_entries(base))
			;

		tmp1 = (UINT32)(inw(base + FTSSP010_REG_DATA_PORT)); // Dummy read
		buf[i] = (UINT32)(inw(base + FTSSP010_REG_DATA_PORT)); // Real data

		ftssp010_enable_FS(base, 0, 0, 1);

		buf[i] = (buf[i]<<1) | ((tmp1 & 0x80000000)? 0x1:0x0);
		delay_ms(5);
	}
}
#endif
/*SPI2AHB*/

static void ftssp010_write_word(int base, const void *data, int wsize)
{
	unsigned int    tmp = 0;

	if (data) {
		switch (wsize) {
		case 1:
			tmp = *(const UINT8 *)data;
		  //ftssp010_fill_in_fifo_dma(base, data,0, 1);
			break;

		case 2:
			tmp = *(const UINT16 *)data;
		  //ftssp010_fill_in_fifo_dma(base, data,0, 2);
			break;

		default:
			//ftssp010_fill_in_fifo_dma(base, data,0, 4);
			tmp = *(const UINT32 *)data;
			break;
		}
	}

	outl(tmp, base + FTSSP010_REG_DATA_PORT);
}

void ftssp010_read_word(int base, void *buf, int wsize)
{
	unsigned int    data = inl(base + FTSSP010_REG_DATA_PORT);

	if (buf) {
		switch (wsize) {
		case 1:
			*(UINT8 *) buf = data;
			break;

		case 2:
			*(UINT16 *) buf = data;
			break;

		default:
			*(UINT32 *) buf = data;
			break;
		}
	}
}

/**
 * len unit is bytes
 *
 * Return number of fifo written.
 */
int ftssp010_fill_in_fifo(int tx_addr, const void *buf,
			  int rx_addr, int *len)
{
	int count = 0;
	int rxfifo, fifo, wsize, i;

	rxfifo = ftssp010_rxfifo_depth(rx_addr);
fLib_printf("rxfifo=%x\n",rxfifo);
	//clear before start filling in
	ftssp010_clear_txfifo(tx_addr);

	i = 0;
	while (ftssp010_txfifo_not_full(tx_addr)) {

		if (!*len || !buf)
			break;

		if (i == 0) {
			i = sdl_in_bytes;
			fifo = (sdl_in_bytes + 3) / 4;
		}

		//rx fifo doesn't have enough entries to receive
		if ((count + fifo) > rxfifo)
			break;

		if (i > 3)
			wsize = 4;
		else
			wsize = i;

		fLib_printf("i=%d, len=0x%x, wsize=%d, fifo=%d, count=%d, buf=%x\n",i,*len,wsize,fifo,count,buf);
		ftssp010_write_word(tx_addr, buf, wsize);

		i -= wsize;
		*len -= wsize;
		fifo--;
		count++;
		// Always add 4 bytes for buffer pointer
		buf += 4;
	}

	return count;
}

void ftssp010_enable_dma(int tx_addr)
{
	int val;

	val = inw(tx_addr+ SSP_INT_CONTROL);
	val |= 0x01<<5 ; //enable tx/rx
	outw( tx_addr+SSP_INT_CONTROL, val);
}

int ftssp010_fill_in_fifo_dma(int tx_addr, const void *buf,
			  int rx_addr, int *len)
{
	int fifo, wsize, i;
	int src_width = 2;//source width 8/16/32 bits -> 0/1/2
	int dst_width = 2;//dst width 8/16/32 bits -> 0/1/2
	int burst_size = 0;
	int src_ctrl = 0 ;// Inc/dec/fixed --> 0/1/2
	int dst_ctrl = 2 ;// Inc/dec/fixed --> 0/1/2
	int priority = 0;
	int rxfifo = ftssp010_rxfifo_depth(rx_addr);
	int dma_req = 10;
	int hw = 0;
	int transfer_len;
	int tx;


	if(*len > rxfifo)
		transfer_len = rxfifo;
	else
		transfer_len = *len;

	rxfifo = ftssp010_rxfifo_depth(rx_addr);
	fLib_printf("rxfifo=%x tx_addr %x len %x transfer len %x\n",rxfifo,tx_addr,*len,transfer_len);
	//clear before start filling in
	ftssp010_clear_txfifo(tx_addr);

  //enable sspdma
	ftssp010_enable_dma(tx_addr);
	//enable dmac020
  fLib_InitDMA(FALSE, FALSE, 0x0);
	fLib_DMA_ClearAllInterrupt();
	fLib_DMA_EnableDMAInt(); //Enable DMA Interrupt
	fLib_DMA_NormalMode(0,(UINT32)buf, tx_addr + FTSSP010_REG_DATA_PORT ,transfer_len , src_width , dst_width, burst_size, src_ctrl,dst_ctrl ,priority, hw,dma_req);
	// Always enable rxside before txside
//	tx = (tx_addr == FTSSP010_REG_BASE_S)? 1 : 0;
	//ftssp010_enable(FTSSP010_REG_BASE_S, tx, !tx);
//	ftssp010_enable(FTSSP010_REG_BASE_M, !tx, tx);
	fLib_EnableDMAChannel(0);

	fLib_DMA_WaitDMAInt(0);
	fLib_DisableDMAChannel(0);
  fLib_printf("over\n");
	*len -= transfer_len;
	return transfer_len;
}

/**
 * count is the number of FIFO entries wanted to be read.
 *
 * Return the remaining number of fifo not read yet.
 */
int ftssp010_take_out_fifo(int rx_addr, void *buf, int count)
{
	int i, wsize;

	i = 0;
	while (count) {

		if (!buf)
			break;

		while (!ftssp010_rxfifo_valid_entries(rx_addr)) {
			;
		}

		if (i == 0)
			i = sdl_in_bytes;

		if (i > 3)
			wsize = 4;
		else
			wsize = i;

		ftssp010_read_word(rx_addr, buf, wsize);

		i -= wsize;
		count--;
		buf += 4;

	}

	return count;
}

/**
 * count is the number of FIFO entries wanted to be read.
 *
 * Return the remaining number of fifo not read yet.
 */
int ftssp010_take_out_fifo_bidirect(int rx1_addr, void *buf1,
				    int rx2_addr, void *buf2,
				    int count)
{
	int i, wsize;

	i = 0;
	while (count) {

		if (!buf1 || !buf2)
			break;

		while (!ftssp010_rxfifo_valid_entries(rx1_addr)) {
			;
		}

		if (i == 0)
			i = sdl_in_bytes;

		if (i > 3)
			wsize = 4;
		else
			wsize = i;

		ftssp010_read_word(rx1_addr, buf1, wsize);
		ftssp010_read_word(rx2_addr, buf2, wsize);

		i -= wsize;
		count--;
		buf1 += 4;
		buf2 += 4;

	}

	return count;
}


/**
 * len unit is bytes
 *
 * Output:
 *   len1, len2: remaining number of bytes not transfered.
 */
int ftssp010_transfer_data_bidirect(int addr1, const void *tx1_buf,
				    void *rx1_buf, int *len1,
			   	    int addr2, const void *tx2_buf,
				    void *rx2_buf, int *len2)
{
	int count1, count2, fifo;

	// Always disable Master before Slave
	ftssp010_enable(FTSSP010_REG_BASE_M, 0, 0);
	ftssp010_enable(FTSSP010_REG_BASE_S, 0, 0);

	ftssp010_ssp_reset(FTSSP010_REG_BASE_S);
	ftssp010_ssp_reset(FTSSP010_REG_BASE_M);

	count1 = ftssp010_fill_in_fifo(addr1, tx1_buf, addr2, len1);
	count2 = ftssp010_fill_in_fifo(addr2, tx2_buf, addr1, len2);

	if (count1 != count2) {
		fLib_printf("Fill Tx fifo count not same cnt1 = %d, cnt2 = %d\n",
		       count1, count2);
		return 1;
	}

	ftssp010_clear_rxfifo(addr1);
	ftssp010_clear_rxfifo(addr2);

	// Must enable Slave first
	ftssp010_enable(FTSSP010_REG_BASE_S, 1, 1);
	ftssp010_enable(FTSSP010_REG_BASE_M, 1, 1);

	// Receive the data
	fifo = ftssp010_take_out_fifo_bidirect(addr1, rx1_buf, addr2,
					       rx2_buf, count1);
	if (fifo != 0) {
		fLib_printf("0x%0x: Tx fifo use %d entries,  rx fifo use %d entries\n",
			addr1, count1, (count1 - fifo));
		return 1;
	}

	return 0;
}

/**
 * len unit is bytes
 *
 * Return number of bytes already transfered.
 */
int ftssp010_transfer_data(int tx_addr, const void *tx_buf,
			   int rx_addr, void *rx_buf,
			   int len)
{
	int total_bytes, count = 0;
	int rxfifo, fifo, tx;

	total_bytes = len;

	// Always disable Master before Slave
	ftssp010_enable(FTSSP010_REG_BASE_M, 0, 0);
	ftssp010_enable(FTSSP010_REG_BASE_S, 0, 0);

	ftssp010_ssp_reset(FTSSP010_REG_BASE_S);
	ftssp010_ssp_reset(FTSSP010_REG_BASE_M);
	count = ftssp010_fill_in_fifo(tx_addr, tx_buf, rx_addr, &len);
  fLib_printf("after fill in fifo, len %x , count %x\n",len,count);
	ftssp010_clear_rxfifo(rx_addr);

	// Always enable rxside before txside
	tx = (tx_addr == FTSSP010_REG_BASE_S)? 1 : 0;
	ftssp010_enable(FTSSP010_REG_BASE_S, tx, !tx);
	ftssp010_enable(FTSSP010_REG_BASE_M, !tx, tx);

	// Receive the data
	fifo = ftssp010_take_out_fifo(rx_addr, rx_buf, count);

	//Count should be zero: tx fifo entries used should the same
	// with rx fifo entries
	if (fifo != 0) {
		fLib_printf("Tx fifo use %d entries,  rx fifo use %d entries\n",
			count, (count - fifo));
	}

	return total_bytes - len;
}

int ftssp010_set_pcl(int base, int val)
{
	int cr3;

	if (val & ~FTSSP010_CR3_PCL_MASK) {
		fLib_printf("pcl val range from 0 to 1023 cycles\n");
		return 1;
	}

	cr3 = inl(base + FTSSP010_REG_CR3);
	cr3 &= ~FTSSP010_CR3_PCL_MASK;
	cr3 |= FTSSP010_CR3_PCL(val);

	outl(cr3, base + FTSSP010_REG_CR3);

	return 0;
}

void ftssp010_spi_slave_init(int cr0)
{
	int cr1;
	int intr_cr;

	ftssp010_ssp_reset(FTSSP010_REG_BASE_S);

	cr0 = cr0 | (FTSSP010_CR0_FFMT_SPI | FTSSP010_CR0_SLV_SPI);
	outl(cr0, FTSSP010_REG_BASE_S + FTSSP010_REG_CR0);

	ftssp010_set_sclkdiv(FTSSP010_REG_BASE_S);


	ftssp010_clear_rxfifo(FTSSP010_REG_BASE_S);

	ftssp010_set_data_length(FTSSP010_REG_BASE_S, 1);

	ftssp010_set_pcl(FTSSP010_REG_BASE_S, pcl);



	fLib_printf(" Slave CR0 0x%x\n", inl(FTSSP010_REG_BASE_S +
				       FTSSP010_REG_CR0));
	fLib_printf(" Slave CR1 0x%x\n", inl(FTSSP010_REG_BASE_S +
				       FTSSP010_REG_CR1));
	fLib_printf(" Slave CR2 0x%x\n", inl(FTSSP010_REG_BASE_S +
				       FTSSP010_REG_CR2));
	fLib_printf(" Slave CR3 0x%x\n", inl(FTSSP010_REG_BASE_S +
				       FTSSP010_REG_CR3));
#if 1
	intr_cr = inl(FTSSP010_REG_BASE_S + FTSSP010_REG_INTR_CR);
	intr_cr &= ~(FTSSP010_INTCR_TFDMAEN);
	intr_cr |= (FTSSP010_INTCR_RFDMAEN);
	intr_cr &= ~(FTSSP010_INTCR_RFTHOD_MASK);
	intr_cr |= FTSSP010_INTCR_RFTHOD(1);
	outl(intr_cr, FTSSP010_REG_BASE_S + FTSSP010_REG_INTR_CR);

	fLib_printf(" Slave INTR_CR 0x%x\n", inl(FTSSP010_REG_BASE_S +
				       FTSSP010_REG_INTR_CR));

	ftssp010_enable(FTSSP010_REG_BASE_S, 0, 1);
#endif

}

void ftssp010_spi_master_init(int cr0)
{
	ftssp010_ssp_reset(FTSSP010_REG_BASE_M);

	cr0 = cr0 | (FTSSP010_CR0_FFMT_SPI | FTSSP010_CR0_MSTR_SPI);
	outl(cr0, FTSSP010_REG_BASE_M + FTSSP010_REG_CR0);

	ftssp010_set_sclkdiv(FTSSP010_REG_BASE_M);

	ftssp010_enable(FTSSP010_REG_BASE_M, 0, 0);

	ftssp010_set_data_length(FTSSP010_REG_BASE_M, sdl_in_bytes);

	ftssp010_set_pcl(FTSSP010_REG_BASE_M, pcl);

	fLib_printf(" Master CR0 0x%x\n", inl(FTSSP010_REG_BASE_M +
					FTSSP010_REG_CR0));
	fLib_printf(" Master CR1 0x%x\n", inl(FTSSP010_REG_BASE_M +
					FTSSP010_REG_CR1));
	fLib_printf(" Master CR2 0x%x\n", inl(FTSSP010_REG_BASE_M +
					FTSSP010_REG_CR2));
	fLib_printf(" Master CR3 0x%x\n", inl(FTSSP010_REG_BASE_M +
					FTSSP010_REG_CR3));
}

/**
 * Enable Slave before Master
 *
 * cs_low equals to 0 means  frame/sync(chip select) active low.
 * lsb equals to 0 means MSB tx first.
 */
char *mode_string[SPI_MODE_MAX] = { "CLKPO = 0, CLKPHA = 0",
				    "CLKPO = 0, CLKPHA = 1",
				    "CLKPO = 1, CLKPHA = 0",
				    "CLKPO = 1, CLKPHA = 1",
				 };
void ftssp010_spi_init(SPI_CHIP_SELECT cs_low, int lsb, SPI_MODE_TYPE mode)
{
	int cr0, md;

	switch (mode) {
		case SPI_MODE_0:
			md = 0;
			break;
		case SPI_MODE_1:
			md = 1;
			break;
		case SPI_MODE_2:
			md = 2;
			break;
		case SPI_MODE_3:
			md = 3;
			break;
		default:
			fLib_printf("SPI undefined mode 0x%x\n", mode);
			return;
	}
	fLib_printf("SPI init mode %s\n", mode_string[md]);

	cr0 = mode;

	cr0 |= cs_low;

	if (lsb)
		cr0 |= FTSSP010_CR0_LSB;

	ftssp010_spi_slave_init(cr0);

	//ftssp010_spi_master_init(cr0 | FTSSP010_CR0_LSB);
}


void ftssp010_spi_slave_dma_read(int base, uint8_t *buf, int len)
{
	int i;
	char chr;
	int src_width = 0;//source width 8/16/32 bits -> 0/1/2
	int dst_width = 0;//dst width 8/16/32 bits -> 0/1/2
	int burst_size = 0;
	int src_ctrl = 2 ;// Inc/dec/fixed --> 0/1/2
	int dst_ctrl = 0 ;// Inc/dec/fixed --> 0/1/2
	int priority = 0;
	int hw = 1;

	//send some data by using dma020 API;

  fLib_InitDMA(FALSE, FALSE, 0x0);
	fLib_DMA_ClearAllInterrupt();
	fLib_DMA_EnableDMAInt(); //Enable DMA Interrupt

	fLib_DMA_NormalMode(0,base+0x18, (UINT32)buf, len, src_width , dst_width, burst_size, src_ctrl,dst_ctrl ,priority, hw, 11);



	fLib_EnableDMAChannel(0);

	//fLib_DMA_WaitDMAInt(0);
	//fLib_DisableDMAChannel(0);

}


fLib_DMA_LLD_t dma_llp[2];

void link_dma(int base, uint8_t *buf, int len)
{
	int i;
	char chr;
	int src_width = 0;//source width 8/16/32 bits -> 0/1/2
	int dst_width = 0;//dst width 8/16/32 bits -> 0/1/2
	int burst_size = 1;
	int src_ctrl = 2 ;// Inc/dec/fixed --> 0/1/2
	int dst_ctrl = 0 ;// Inc/dec/fixed --> 0/1/2
	int priority = 0;
	int hw = 1;

	//send some data by using dma020 API;

	fLib_printf("%p\n",dma_llp);

  fLib_InitDMA(FALSE, FALSE, 0x0);
	fLib_DMA_ClearAllInterrupt();
	fLib_DMA_EnableDMAInt(); //Enable DMA Interrupt

	//fLib_DMA_NormalMode(0,base+0x18, (UINT32)buf, len, src_width , dst_width, burst_size, src_ctrl,dst_ctrl ,priority, hw, 11);

	fLib_DMA_LinkMode(0,(UINT32)dma_llp,2,0x50120000, 0x50126000, 1023, src_width, dst_width, burst_size, src_ctrl, dst_ctrl, priority, hw, 11);

	fLib_EnableDMAChannel(0);

	//fLib_DMA_WaitDMAInt(0);
	//fLib_DisableDMAChannel(0);

}