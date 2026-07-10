#include "DrvSSP010.h"	
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
//#include <malloc/malloc.h>
#include "Common_Include.h"
#include "ftssp010.h"

extern void dump_buf(uint8_t *buf, int len);
extern int ftssp010_transfer_data_bidirect(int addr1, const void *tx1_buf,
				    void *rx1_buf, int *len1,
			   	    int addr2, const void *tx2_buf,
				    void *rx2_buf, int *len2);

static INT32 do_ftssp010_set_txside(INT8 * const argv[]);
static INT32 do_ftssp010_tx(INT8 * const argv[]);
static INT32 do_ftssp010_set_sdl(INT8 * const argv[]);
static INT32 do_ftssp010_set_pcl(INT8 * const argv[]);
static UINT32 do_ftssp010_burnin(void);

typedef enum {
	FFMT_SPI,
	FFMT_I2S,
} FFMT_TYPE;

struct ctrl {
	int addr;
	int tx;
	int rx;
	char *tx_buf;
	char *rx_buf;
};

struct ctrl master, slave;
FFMT_TYPE ffmt;
int buf_len = 512;//1024

typedef struct cmd {
    INT8 *name;
    INT8 *usage;
    INT32(*func) (INT32 argc, INT8 ** argv);
} cmd_t;

/*
static cmd_t ftssp010_cmd_tbl[] = {
	{"txside", "<master|slave|both>", do_ftssp010_set_txside},
	{"sdl", "<len>", do_ftssp010_set_sdl},
	{"pcl", "<len>", do_ftssp010_set_pcl},
	{"tx", "<len> <i2s|spi>", do_ftssp010_tx},
	{"burnin", "<i2s|spi>", do_ftssp010_burnin},
	{"quit", "", 0},
	{0}
};
*/

extern int pcl;
extern int sdl_in_bytes;

// txside <master|slave|both>
static INT32 do_ftssp010_set_txside(INT8 * const argv[])
{
	int data;
	if (strcmp(argv[1], "master") == 0) {
		master.tx = 1;
		master.rx = 0;
		slave.tx = 0;
		slave.rx = 1;
	} else if (strcmp(argv[1], "slave") == 0) {
		master.tx = 0;
		master.rx = 1;
		slave.tx = 1;
		slave.rx = 0;
	} else if (strcmp(argv[1], "both") == 0) { 
		master.tx = 1;
		master.rx = 1;
		slave.tx = 1;
		slave.rx = 1;
	} else {
		fLib_printf(" Argument value not correct\n");
		return 1;
	}

	fLib_printf("Master addr 0x%x tx %d rx %d\n" \
	       "Slave addr 0x%x tx %d rx %d\n",
			master.addr, master.tx, master.rx,
			slave.addr, slave.tx, slave.rx);
	return 0;
}

// sdl <len>
static INT32 do_ftssp010_set_sdl(INT8 * const argv[])
{
	int sdl;


	sdl = strtol(argv[1], 0 , 0);
	if (ftssp010_set_data_length(FTSSP010_REG_BASE_S, sdl))
		return 1;

	if (ftssp010_set_data_length(FTSSP010_REG_BASE_M, sdl))
		return 1;
	

	sdl_in_bytes = sdl;

	return 0;
}

// pcl <len>
static INT32 do_ftssp010_set_pcl(INT8 * const argv[])
{

	pcl = strtol(argv[1], 0 , 0);

	if (ftssp010_set_pcl(FTSSP010_REG_BASE_S, pcl))
		return 1;

	if (ftssp010_set_pcl(FTSSP010_REG_BASE_M, pcl))
		return 1;

	return 0;
}

static int do_ftssp010_transfer_bi(int len)
{
	int i, j, bytes, len1, len2, ret;
	int frsync, fifo, index;
	char *tbuf1, *rbuf1, *tbuf2, *rbuf2;

	fLib_printf(" %s-%s: Bi-direct transfer %d bytes(sdl %d,",
		ftssp010_get_fformat_string(master.addr),
		ftssp010_get_fformat_string(slave.addr),
		len, sdl_in_bytes);

	if (ffmt == FFMT_SPI) {
		fLib_printf("pcl %d)\n", pcl);
	} 
/*
	else if (ffmt == FFMT_I2S) {
		fLib_printf("%s, pdl_r %d pdl_l %d at %s)\n",
			stereo ? "stereo" : "mono", pdl_r,
			pdl_l, pad_data_back ? "back" : "front");
	}
*/
	tbuf1 = master.tx_buf;
	rbuf1 = master.rx_buf;
	tbuf2 = slave.tx_buf;
	rbuf2 = slave.rx_buf;

	// Number of FIFO bytes to tansfer sdl in one frame/sync
	fifo = (sdl_in_bytes + 3) & ~0x3;

	frsync = len / sdl_in_bytes;
	index = fifo * frsync;
	index >>= 2;
	for (i = 0; i < index; i++) {
		*((int *)rbuf1 + i) = 0xAAAAAAAA;
		*((int *)rbuf2 + i) = 0xAAAAAAAA;
	}

	while (len) {

		len1 = len2 = len;
		ret = ftssp010_transfer_data_bidirect(
				master.addr, tbuf1, rbuf1, &len1,
			   	slave.addr, tbuf2, rbuf2, &len2);
		if (ret)
			goto error;

		if (len1 != len2) {
			fLib_printf(" -- Master remain %d bytes, slave %d bytes\n",
				len1, len2);
			goto error;
		}

		bytes = len - len1;
		//Number of frame/sync(s) to transfer "bytes".
		frsync = bytes / sdl_in_bytes;
		index = fifo * frsync;
		fLib_printf(" -- Compare master tbuf 0x%x slave rbuf 0x%x len %d bytes ",
			(int)tbuf1, (int)rbuf2, bytes);
		for (i = 0; i < index;) {
			for (j = i; j < (i + sdl_in_bytes); j++) { 
				if(tbuf1[j] != rbuf2[j]) {
					fLib_printf(" failed\n");
					fLib_printf(" --- i = %d, tbuf(0x%x) = 0x%x, "\
					       "rbuf(0x%x) = 0x%x\n", j, &tbuf1[j],
						tbuf1[j],&rbuf2[j],  rbuf2[j]);
					goto error;
				}
			}

			i = (i + sdl_in_bytes + 3) & ~0x3;
		}
		tbuf1 += index;
		rbuf2 += index;
		fLib_printf("success\n");

		bytes = len - len2;
		//Number of frame/sync(s) to transfer "bytes".
		frsync = bytes / sdl_in_bytes;
		index = fifo * frsync;
		fLib_printf(" -- Compare slave tbuf 0x%x master rbuf 0x%x len %d bytes ",
			(int)tbuf2, (int)rbuf1, bytes);
		for (i = 0; i < index;) {
			for (j = i; j < (i + sdl_in_bytes); j++) { 
				if(tbuf2[j] != rbuf1[j]) {
					fLib_printf(" failed\n");
					fLib_printf(" --- i = %d, tbuf(0x%x) = 0x%x, "\
					       "rbuf(0x%x) = 0x%x\n", j, &tbuf2[j],
						tbuf2[j],&rbuf1[j],  rbuf1[j]);
					goto error;
				}
			}

			i = (i + sdl_in_bytes + 3) & ~0x3;
		}
		tbuf2 += index;
		rbuf1 += index;
		fLib_printf("success\n");

		len -= bytes;
	}
	return 0;

error:
	return 1;
}

static int do_ftssp010_transfer(int tx_addr, char *tbuf,
				int rx_addr, char *rbuf, int len)
{
	int i, j, bytes;
	int frsync, fifo, index;
	int *tmp;

	fLib_printf(" %s(0x%x)-%s(0x%x): Tx %d bytes(sdl %d, ",
		ftssp010_get_fformat_string(tx_addr), tx_addr,
		ftssp010_get_fformat_string(rx_addr), rx_addr,
		len, sdl_in_bytes);

	if (ffmt == FFMT_SPI) {
		fLib_printf("pcl %d)\n", pcl);
	} 
	/*
	else if (ffmt == FFMT_I2S) {
		fLib_printf("%s, pdl_r %d pdl_l %d at %s)\n",
		       stereo ? "stereo" : "mono", pdl_r,
		       pdl_l, pad_data_back ? "back" : "front");
	}
*/
	// Number of FIFO bytes to tansfer sdl in one frame/sync
	fifo = (sdl_in_bytes + 3) & ~0x3;
fLib_printf("fifo = 0x%x\n", fifo);
	frsync = len / sdl_in_bytes;
fLib_printf("frsync = 0x%x, len=0x%x, sdl_in_bytes=%d\n", frsync, len,sdl_in_bytes);
	index = fifo * frsync;
	index >>= 2;
	tmp = (int *) rbuf;
	for (i = 0; i < index; i++)
		*(tmp + i) = 0xAAAAAAAA;

	while (len) {

		bytes = ftssp010_transfer_data(tx_addr, (const void*)tbuf,
					       rx_addr, (void *)rbuf, len);

		//unlikely happens, it is a bug
		if (bytes > len) {
			fLib_printf(" -- Want to transfer %d bytes, actual done %d bytes\n",
				len, bytes);
			goto error;
		}

		//Number of frame/sync(s) to transfer "bytes".
		frsync = bytes / sdl_in_bytes;

		index = fifo * frsync;

		fLib_printf(" -- Compare tbuf 0x%x rbuf 0x%x len %d bytes ",
			(int)tbuf, (int)rbuf, bytes);
		for (i = 0; i < index;) {
			for (j = i; j < (i + sdl_in_bytes); j++) { 
				if(tbuf[j] != rbuf[j]) {
					fLib_printf(" failed\n");
					fLib_printf(" --- i = %d, tbuf(0x%x) = 0x%x, "\
					       "rbuf(0x%x) = 0x%x\n", j, &tbuf[j],
						tbuf[j],&rbuf[j],  rbuf[j]);
					goto error;
				}
				//fLib_printf("\n rbuf[0x%x]=0x%x",&rbuf[j], rbuf[j]);
			}

			i = (i + sdl_in_bytes + 3) & ~0x3;
		}
		fLib_printf("success\n");

		len -= bytes;
		tbuf += index;
		rbuf += index;
	}
	return 0;

error:
	return 1;
}

static INT32 do_ftssp010_get_addrs(int *tx_addr, char **tx_buf,
				   int *rx_addr, char **rx_buf)
{
	if (master.tx && !master.rx) {
		fLib_printf("Master transmit, Slave receive\n");
		*tx_addr = master.addr;
		*tx_buf = master.tx_buf;
		*rx_addr = slave.addr;
		*rx_buf = slave.rx_buf;
	} else if (slave.tx && !slave.rx) {
		fLib_printf("Slave transmit, Master receive\n");
		*tx_addr = slave.addr;
		*tx_buf = slave.tx_buf;
		*rx_addr = master.addr;
		*rx_buf = master.rx_buf;
	} else if (master.tx && slave.tx) {
		fLib_printf("Master transmit/receive, Slave transmit/receive\n");
		return 2;
	} else {
		fLib_printf(" Not valid tx/rx combination\n");
		return 0;
	}

	return 1;
}

// tx <len> <i2s|spi> <mode>
static INT32 do_ftssp010_tx(INT8 * const argv[])
{
	char *tx_buf;
	char *rx_buf;
	int tx_addr, rx_addr;
	int i, len, mode;


	len = strtol(argv[1], 0 , 0);
	/* multiple of sdl_in_bytes bytes */
	len = (len + sdl_in_bytes) - (len % sdl_in_bytes);

	mode = strtol(argv[4], 0 , 0);
	if (mode != 0)
		mode = strtol(argv[3], 0 , 0) & 0x3;
	else
		mode = SPI_MODE_0;

	if (strcmp(argv[2], "i2s") == 0) {
		ffmt = FFMT_I2S;
		//ftssp010_i2s_init(0);
	} else if (strcmp(argv[2], "spi") == 0) {
		ffmt = FFMT_SPI;
		ftssp010_spi_init(SPI_CS_LOW, 0, mode);
	}

	i = do_ftssp010_get_addrs(&tx_addr, &tx_buf,
				  &rx_addr, &rx_buf);

	if (i == 2)
		do_ftssp010_transfer_bi(len);
	else if (i == 1)
		do_ftssp010_transfer(tx_addr, tx_buf,
				     rx_addr, rx_buf,
				     len);
	
	return 0;
}

// burnin <spi|i2s>
static UINT32 do_ftssp010_burnin(void)
{
	char *tx_buf;
	char *rx_buf;
	int tx_addr, rx_addr;
	int i, len, mode, bidirect;
	UINT8 chr;
	//bidirect = 0;
	i = 0;
	mode = 0;
reinit:

	ffmt = FFMT_SPI;
	pcl = FTSSP010_CR3_PCL_MASK;
	ftssp010_spi_init(SPI_CS_LOW, 0, mode);

	bidirect = do_ftssp010_get_addrs(&tx_addr, &tx_buf,
					 &rx_addr, &rx_buf);
	if (!bidirect)
		return 0;

	while (1) {	
		len = (rand() % 200) + 1;
		/* multiple of sdl_in_bytes bytes */
		len = (len + sdl_in_bytes) - (len % sdl_in_bytes);
		if (bidirect == 2) {
			if (do_ftssp010_transfer_bi(len))
				break;
		} else {
			if (do_ftssp010_transfer(tx_addr, tx_buf,
				     rx_addr, rx_buf,
				     len))
				break;
		}

		chr = fLib_getch(DEBUG_CONSOLE);
		if (chr==0x1b)
				  break;
		
		i++;
		if (ffmt == FFMT_SPI) {
			if (i % 10 == 0) {
				pcl += (rand() & 0x7);
				if (pcl & ~FTSSP010_CR3_PCL_MASK)
					pcl = 0;
				if (ftssp010_set_pcl(FTSSP010_REG_BASE_S, pcl))
					return 1;

				if (ftssp010_set_pcl(FTSSP010_REG_BASE_M, pcl))
					return 1;
			}
		} 
		/*else if (ffmt == FFMT_I2S) { //I2S
			if (i % 10 == 0) {
#if 1
				pdl_r += 1;
				pdl_r &= 0xff;
				pdl_l = pdl_r + i;
				pdl_l &= 0xff;
				ftssp010_set_pdl(FTSSP010_REG_BASE_S);
				ftssp010_set_pdl(FTSSP010_REG_BASE_M);
#endif
				pad_data_back = pad_data_back ? 0 : 1;
				ftssp010_set_fsjtfy(FTSSP010_REG_BASE_S);
				ftssp010_set_fsjtfy(FTSSP010_REG_BASE_M);
			}
		}*/
#if 1
		if (i % 400 == 0) {
			sdl_in_bytes++;
			if (sdl_in_bytes > 16)
				sdl_in_bytes = 1;

			if (ftssp010_set_data_length(FTSSP010_REG_BASE_S,
						     sdl_in_bytes))
				return 1;

			if (ftssp010_set_data_length(FTSSP010_REG_BASE_M,
						     sdl_in_bytes))
				return 1;
		}

		if (i % 100 == 0) {
			if (ffmt == FFMT_SPI) {
				mode++;
				mode &= SPI_MODE_3;
			} 
			/*else if (ffmt == FFMT_I2S) {
				stereo = stereo ? 0 : 1;
			}*/
			goto reinit;
		}
#endif
	}

	return 0;
}

//int ftssp010_main(int argc, char * const argv[])
void ftssp010_main(void)
{
	int i, data;
	//char cmdstr[26];
	int argc;
	char *argv[40];
	unsigned char buf[512];
	
	// Default to master tx, slave rx, half duplex
	master.addr = FTSSP010_REG_BASE_M;
	master.tx = 1;
	master.rx = 0;
	//master.tx_buf = (char *) memalign(16, buf_len);
	master.tx_buf = (char * )malloc(buf_len);
	if (!master.tx_buf) {
		fLib_printf("Allocate master.tx_buf %d bytes failed\n", buf_len);
		//return 1;
		while(1);
	}
	//master.rx_buf = (char *) memalign(16, (buf_len << 1));
	master.rx_buf = (char *) malloc((buf_len));//kay 1106
	//master.rx_buf = (char *) malloc((buf_len << 1));
	if (!master.rx_buf) {
		fLib_printf("Allocate master.rx_buf %d bytes failed\n", buf_len);
		//return 1;
		while(1);
	}
	fLib_printf(" master addr: 0x%x, tx_buf 0x%x, rx_buf 0x%x, len %d\n",
		master.addr, (int)master.tx_buf, (int)master.rx_buf, buf_len);

	slave.addr = FTSSP010_REG_BASE_S;
	slave.tx = 0;
	slave.rx = 1;
	//slave.tx_buf = (char *) memalign(16, (buf_len << 1));
	slave.tx_buf = (char * )malloc(buf_len);//kay 1106
	//slave.tx_buf = (char * )malloc(buf_len<<1);
	if (!slave.tx_buf) {
		fLib_printf("Allocate slave.tx_buf %d bytes failed\n", (buf_len));
		//return 1;
		while(1);
	}
	//slave.rx_buf = (char *) memalign(16, buf_len);
	slave.rx_buf = (char *) malloc(buf_len);
	if (!slave.rx_buf) {
		fLib_printf("Allocate slave.rx_buf %d bytes failed\n", buf_len);
		//return 1;
		while(1);
	}
	fLib_printf(" slave addr: 0x%x, : tx_buf 0x%x, rx_buf 0x%x, len %d\n",
		slave.addr, (int)slave.tx_buf, (int) slave.rx_buf, buf_len);

	// Prepare the pattern for writing
	for (i = 0; i < buf_len; i++) {
		*(master.tx_buf + i) = i;
		*(slave.tx_buf + i) = i;
	}

	
	for(;;)
	{
	  fLib_printf("+----------------------------------------------------------+\n");
		fLib_printf("usage(hexadecimal value must have prefix 0x)\n");
		fLib_printf("    txside <master|slave|both>\n"); 
		fLib_printf("    sdl <len>\n");
		fLib_printf("    pcl <len>\n");		
		fLib_printf("    tx <len> <i2s|spi> <spi-mode>\n"); // transfer data
		fLib_printf("    burnin\n");		
		fLib_printf("    exit\n");
		fLib_printf("please input command: ");
		fLib_gets(DEBUG_CONSOLE, (char *)buf);
		fLib_printf("\n");
		argc = substring(argv, (char *)buf, " \r\n\t");
		
		if (strcmp(argv[0], "txside")==0)
		{
			if(argc != 2) 
			{
				fLib_printf("invalid parameters!\n");
				continue;
			}
			do_ftssp010_set_txside(argv);
		}
		
		else if (strcmp(argv[0], "sdl")==0)
		{
			if(argc != 2) 
			{
				fLib_printf("invalid parameters!\n");
				continue;
			}
			do_ftssp010_set_sdl(argv);
		}

		else if (strcmp(argv[0], "pcl")==0)
		{
			if(argc != 2) 
			{
				fLib_printf("invalid parameters!\n");
				continue;
			}
			do_ftssp010_set_pcl(argv);
		}
		
		else if (strcmp(argv[0], "tx")==0) 
		{
			if(argc != 4) 
			{
				fLib_printf("invalid parameters!\n");
				continue;
			}
			do_ftssp010_tx(argv);
		}

		else if (strcmp(argv[0], "burnin")==0)
		{
			if(argc != 1) 
			{
				fLib_printf("invalid parameters!\n");
				continue;
			}
			do_ftssp010_burnin(); //spi only
		}
		else if (strcmp(argv[0], "exit")==0)
		{
			break;
		}
	}	
	
	free((char *)master.tx_buf);
	free((char *)master.rx_buf);
	free((char *)slave.tx_buf);
	free((char *)slave.rx_buf);
	
}

