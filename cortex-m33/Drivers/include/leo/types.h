#ifndef TYPES_H
#define TYPES_H

//#include "platform.h"
//#include "plat-FIE3100.h"
#include "spec.h"

#define BIT0                            0x00000001
#define BIT1                            0x00000002
#define BIT2                            0x00000004
#define BIT3                            0x00000008
#define BIT4                            0x00000010
#define BIT5                            0x00000020
#define BIT6                            0x00000040
#define BIT7                            0x00000080
#define BIT8                            0x00000100
#define BIT9                            0x00000200
#define BIT10                           0x00000400
#define BIT11                           0x00000800
#define BIT12                           0x00001000
#define BIT13                           0x00002000
#define BIT14                           0x00004000
#define BIT15                           0x00008000
#define BIT16                           0x00010000
#define BIT17                           0x00020000
#define BIT18                           0x00040000
#define BIT19                           0x00080000
#define BIT20                           0x00100000
#define BIT21                           0x00200000
#define BIT22                           0x00400000
#define BIT23                           0x00800000
#define BIT24                           0x01000000
#define BIT25                           0x02000000
#define BIT26                           0x04000000
#define BIT27                           0x08000000
#define BIT28                           0x10000000
#define BIT29                           0x20000000
#define BIT30                           0x40000000
#define BIT31                           0x80000000


// --------------------------------------------------------------------
// find the last bit set in x and return the index of that bit.
// --------------------------------------------------------------------
#if 0
static inline int fls(int x)
{
	int r = 32;

    if (!x)
		return 0;
    if (!(x & 0xffff0000u)) {
        x <<= 16;
        r -= 16;
    }
    if (!(x & 0xff000000u)) {
        x <<= 8;
        r -= 8;
    }
    if (!(x & 0xf0000000u)) {
        x <<= 4;
        r -= 4;
    }
    if (!(x & 0xc0000000u)) {
        x <<= 2;
        r -= 2;
    }
    if (!(x & 0x80000000u)) {
        x <<= 1;
        r -= 1;
    }
    return r;
}


// --------------------------------------------------------------------
// find the first bit set in x and return the index of that bit.
// --------------------------------------------------------------------
static inline int ffs(int x)
{
    int r = 1;

    if (!x)
		return 0;
    if (!(x & 0xffff)) {
        x >>= 16;
        r += 16;
    }
    if (!(x & 0xff)) {
        x >>= 8;
        r += 8;
    }
    if (!(x & 0xf)) {
        x >>= 4;
        r += 4;
    }
    if (!(x & 3)) {
        x >>= 2;
        r += 2;
    }
    if (!(x & 1)) {
        x >>= 1;
        r += 1;
    }
    return r;
}
#endif


#define CR              0x0D
#define LF              0x0A
#define BS              0x08
#define ESC				27

#ifndef FALSE
#define FALSE   0
#endif

#ifndef TRUE
#define TRUE    1
#endif

#ifndef NULL
#define NULL    0
#endif


#ifndef ON
#define ON              1
#endif

#ifndef OFF
#define OFF             0
#endif


#ifndef ENABLE
#define ENABLE  1
#endif

#ifndef DISABLE
#define DISABLE 0
#endif




#ifndef __INLINE
	#if defined ( __CC_ARM   )
		#define __INLINE         __inline                                   /*!< inline keyword for ARM Compiler       */
	#elif defined ( __ICCARM__ )
		#define __INLINE        inline                                      /*!< inline keyword for IAR Compiler. Only avaiable in High optimization mode! */
	#elif defined   (  __GNUC__  )
		#define __INLINE         inline                                     /*!< inline keyword for GNU Compiler       */
	#elif defined   (  __TASKING__  )
		#define __INLINE         inline                                     /*!< inline keyword for TASKING Compiler   */
	#endif
#endif


/* type define */
	typedef unsigned long long 		UINT64;
	typedef long long 				INT64;
	typedef	unsigned int			UINT32;
	typedef	unsigned int			uint32;
	typedef	int						INT32;
	typedef	unsigned short			UINT16;
	typedef	short					INT16;
	typedef unsigned char			UINT8;
	typedef unsigned char			uint8;
	typedef char					INT8;
	typedef unsigned char			BOOL;
	typedef unsigned char 			BOOLEAN;

	typedef unsigned char           u8_t;
	typedef unsigned short          u16_t;
	typedef unsigned long           u32_t;
	typedef unsigned long long		u64_t;

	typedef unsigned char 			uchar;

    typedef unsigned char           u8;
	typedef unsigned short          u16;
    typedef unsigned int            u32;
    typedef unsigned long long      u64;



typedef INT8          INT8S;
typedef UINT8         INT8U;
typedef INT16         INT16S;
typedef UINT16        INT16U;
typedef INT32         INT32S;
typedef UINT32        INT32U;


typedef unsigned char                   byte;
typedef unsigned short                  word;
typedef unsigned long int               dword;

#define vLib_LeWrite8(x,y)   *(volatile INT8U *)((INT8U * )x)=(y)
#define vLib_LeWrite32(x,y)   *(volatile INT32U *)((INT8U * )x)=(y)  //bessel:add  (INT8U * )
#define u32lib_leread32(x)      *((volatile INT32U *)((INT8U * )x))  //bessel:add  (INT8U * )
#define u32Lib_LeRead32(x)      *((volatile INT32U *)((INT8U * )x)) //bessel:add  (INT8U * )

extern void *hstack_alloc(int size);
extern void hstack_free(void *ptr);

#if 0
	extern void *fa_malloc (size_t size);
	extern void fa_free (const void *ptr);
#else
	#define fa_malloc		malloc
	#define fa_free			free
#endif

//#define NO_MALLOC
#if defined(NO_MALLOC)
	#define kmalloc(size, flag)             hstack_alloc(size)
    #define kfree(ptr)                     	hstack_free(ptr)
#else
    #define kmalloc(size, flag)             fa_malloc(size)
    #define kfree(ptr)                      fa_free(ptr)
#endif //NO_MALLOC


#endif //TYPES_H
