#ifndef FLIB_TIME_H
#define FLIB_TIME_H


#if 1
	#define HZ						100						/// 每秒 Hz 個 tick
	#define JIFF_TO_SEC(x)			((x)/HZ)
	#define JIFF_TO_MSEC(x)			((x)*10)				/// (x*1000/HZ)
	#define JIFF_TO_MICROSEC(x)		((x)*10000)

	#define SEC_TO_JIFF(x)			((x)*HZ)
	#define MSEC_TO_JIFF(x)			(((x)+9)/10)			/// (x*HZ/1000)
	#define MICROSEC_TO_JIFF(x)		((x)/10000)
#else
	#define HZ						10						/// 每秒 Hz 個 tick
	#define JIFF_TO_SEC(x)			((x)/HZ)
	#define JIFF_TO_MSEC(x)			((x)*100)				/// (x*1000/HZ)

	#define SEC_TO_JIFF(x)			((x)*HZ)
	#define MSEC_TO_JIFF(x)			(((x)+99)/100)			/// (x*HZ/1000)
#endif

#define msecs_to_jiffies(x)		MSEC_TO_JIFF(x)

#define time_after(a,b)         \
         ((long)(b) - (long)(a) < 0)

#define time_before(a,b)        time_after(b,a)

/* Parameters used to convert the timespec values: */
#define MSEC_PER_SEC    1000L
#define USEC_PER_MSEC   1000L
#define NSEC_PER_USEC   1000L
#define NSEC_PER_MSEC   1000000L
#define USEC_PER_SEC    1000000L
#define NSEC_PER_SEC    1000000000L
#define FSEC_PER_SEC    1000000000000000L


#if 0
#if !defined(__ASSEMBLY__)

#include <sys/time.h>

#ifdef not_complete_yet   
struct timeval {
	time_t          tv_sec;         /* seconds */
 	long int            tv_usec;        /* microseconds */
};

struct timezone
{
	int tz_minuteswest;         /* Minutes west of GMT.  */
	int tz_dsttime;             /* Nonzero if DST is ever in effect.  */
};
#endif /* end_of_not */


int flib_gettimeofday(struct timeval *tv, struct timezone *tz);
time_t flib_time(time_t *t);
void time_init(void);

extern unsigned long volatile jiffies;
#endif //__ASSEMBLY__

#endif

#endif // FLIB_TIME_H
