#ifndef WM8510_H
#define WM8510_H

/************************************************************************
 *  Wolfson WM8731 WM8510 Audio Codec Definition
 ************************************************************************/
typedef enum 
{
  CODEC_AS_MASTER = 0,
  CODEC_AS_SLAVE = 1
}CODEC_MODE_T;


#define KTrue	(1 == 1)
#define KFalse	(1 == 0)

#define KBit0		(1 << 0)
#define KBit1		(1 << 1)
#define KBit2		(1 << 2)
#define KBit3		(1 << 3)
#define KBit4		(1 << 4)
#define KBit5		(1 << 5)
#define KBit6		(1 << 6)
#define KBit7		(1 << 7)
#define KBit8		(1 << 8)


// WM8510 CODEC Registers
#define KR0		0x0
#define KR1		0x1
#define KR2		0x2
#define KR3		0x3
#define KR4		0x4
#define KR5		0x5
#define KR6		0x6
#define KR7		0x7 
#define KR8		0x8
#define KR9		0x9
#define KR10		0xA
#define KR11		0xB
#define KR14		0xE
#define KR15	        0xF
#define KR18	        0x12
#define KR19	        0x13
#define KR20	        0x14
#define KR21	        0x15
#define KR22	        0x16
#define KR24		0x18
#define KR25		0x19
#define KR26		0x1A
#define KR27		0x1B
#define KR28		0x1C
#define KR29		0x1D
#define KR30		0x1E
#define KR32		0x20
#define KR33		0x21
#define KR34		0x22
#define KR35		0x23
#define KR36		0x24
#define KR37		0x25
#define KR38		0x26
#define KR39		0x27
#define KR40		0x28
#define KR44		0x2C
#define KR45		0x2D
#define KR47		0x2F
#define KR49		0x31
#define KR50		0x32
#define KR54		0x36
#define KR56		0x38

#define KAddrShift	9
#define KR0VALUE	((KR0<<KAddrShift) | 0x117)
#define KR1VALUE	((KR1<<KAddrShift) | 0x117)

#define KR2VALUE	((KR2<<KAddrShift) | 0x100)
//#define KR2VALUE	((KR2<<KAddrShift) | 0x014)

#define KR3VALUE	((KR3<<KAddrShift) | 0x100)
//#define KR3VALUE	((KR3<<KAddrShift) | 0x081)

//#define KR4VALUE	((KR4<<KAddrShift) | 0x050)
#define KR4VALUEL	((KR4<<KAddrShift) | 0x011) //Line-in
#define KR4VALUEM	((KR4<<KAddrShift) | 0x014) //MAC-in
#define KR4VALUEO	((KR4<<KAddrShift) | 0x016) //Other (Disable bypass; Enable mute; Microphone input select to ADC)
#define KR5VALUE	((KR5<<KAddrShift) | 0x006)
#define KR6VALUE	((KR6<<KAddrShift) | 0x000)
//#define KR6VALUE	((KR6<<KAddrShift) | 0x040)		// lmc83: test ==> no clkout

	#define KR7VALUE	((KR7<<KAddrShift) | 0x002)
#define KR8VALUE	((KR8<<KAddrShift) | 0x000)
//#define KR8VALUE	((KR8<<KAddrShift) | 0x040)		// lmc83: test ==> mclk/2
//#define KR8VALUE	((KR8<<KAddrShift) | 0x080)		// lmc83: test ==> clkout/2
#define KR9VALUE	((KR9<<KAddrShift) | 0x001)
#define KR10VALUE	((KR10<<KAddrShift) | 0x000)
#define KR11VALUE	((KR11<<KAddrShift) | 0x0FF)

#define KR14VALUE	((KR14<<KAddrShift) | 0x100)
//#define KR14VALUE	((KR14<<KAddrShift) | 0x000)

#define KR15VALUE	((KR15<<KAddrShift) | 0x0)
#define KR18VALUE	((KR18<<KAddrShift) | 0x12C)
#define KR19VALUE	((KR19<<KAddrShift) | 0x02C)
#define KR20VALUE	((KR20<<KAddrShift) | 0x02C)
#define KR21VALUE	((KR21<<KAddrShift) | 0x02C)
#define KR22VALUE	((KR22<<KAddrShift) | 0x02C)
#define KR24VALUE	((KR24<<KAddrShift) | 0x032)
#define KR25VALUE	((KR25<<KAddrShift) | 0x000)

#define KR27VALUE	((KR27<<KAddrShift) | 0x000)
//#define KR27VALUE	((KR27<<KAddrShift) | 0x080)

#define KR28VALUE	((KR28<<KAddrShift) | 0x000)
#define KR29VALUE	((KR29<<KAddrShift) | 0x000)
#define KR30VALUE	((KR30<<KAddrShift) | 0x000)
#define KR32VALUE	((KR32<<KAddrShift) | 0x038)
#define KR33VALUE	((KR33<<KAddrShift) | 0x00B)
#define KR34VALUE	((KR34<<KAddrShift) | 0x032)
#define KR35VALUE	((KR35<<KAddrShift) | 0x000)
#define KR36VALUE	((KR36<<KAddrShift) | 0x008)
#define KR37VALUE	((KR37<<KAddrShift) | 0x00C)
#define KR38VALUE	((KR38<<KAddrShift) | 0x093)
#define KR39VALUE	((KR39<<KAddrShift) | 0x0E9)

//#define KR44VALUE	((KR44<<KAddrShift) | 0x004)
#define KR44VALUE	((KR44<<KAddrShift) | 0x00c)

#define KR45VALUE	((KR45<<KAddrShift) | 0x010)

//#define KR47VALUE	((KR47<<KAddrShift) | 0x157)
#define KR47VALUE	((KR47<<KAddrShift) | 0x105)

#define KR49VALUE	((KR49<<KAddrShift) | 0x002)
#define KR50VALUE	((KR50<<KAddrShift) | 0x000)
#define KR54VALUE	((KR54<<KAddrShift) | 0x079)

#define KR56VALUE	((KR56<<KAddrShift) | 0x001)


extern int wm8510SetSamplingCtrl(unsigned int );
extern void wm8510_init(CODEC_MODE_T, int i2c_slave_addr);
extern void wm8510_init_volume(unsigned int);
extern void wm8510_set_volume(unsigned int);

#endif
