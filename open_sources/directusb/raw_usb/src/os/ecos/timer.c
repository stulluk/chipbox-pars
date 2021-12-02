
#include <types.h>
#include <timer.h>

/* macro to read the 32 bit timer */
#define READ_TIMER 		(orion_apb_read32(CFG_TIMERBASE+4))

static ulong timestamp;
static ulong lastdec;

static void reset_timer_masked(void);
static ulong get_timer_masked(void);

/* ORION: only support 8/16bit write to APB bus */
static void orion_apb_write32(ulong addr, ulong data)
{
	*(volatile ushort *) (addr + 0) = (ushort) (data);
	*(volatile ushort *) (addr + 2) = (ushort) (data >> 16);
}

static ulong orion_apb_read32(ulong addr)
{
	ulong data;
	data = *(volatile ushort *) (addr + 0);
	data |= (*(volatile ushort *) (addr + 2) << 16);
	return data;
}

/* 
 * nothing really to do with interrupts, 
 * just starts up a counter. 
 */
int timer_init(void)
{
// 	orion_apb_write32((CFG_TIMERBASE + 0), CFG_TIMER_RELOAD); /* TimerLoad */
// 	orion_apb_write32((CFG_TIMERBASE + 4), CFG_TIMER_RELOAD); /* TimerValue */
// 	orion_apb_write32((CFG_TIMERBASE + 8), 0x1);
// 
// 	/* init the timestamp and lastdec value */
// 	reset_timer_masked();

	return 0;
}

/*
 * timer without interrupts
 */

void reset_timer(void)
{
	reset_timer_masked();
}

ulong get_timer(ulong base)
{
	return get_timer_masked() - base;
}

static void set_timer(ulong t)
{
	timestamp = t;
}

/* 
 * delay x useconds AND perserve advance timstamp value.
 */
void udelay(unsigned long usec)
{
	ulong tmo, tmp;

	tmo = usec * (CFG_HZ / (1000 * 1000));

	tmp = get_timer(0);	/* get current timestamp */
	if ((tmo + tmp + 1) < tmp)	/* if setting this fordward will roll time stamp */
		reset_timer_masked();	/* reset "advancing" timestamp to 0, set lastdec value */
	else
		tmo += tmp;	/* else, set advancing stamp wake up time */

	while (get_timer_masked() < tmo)	/* loop till event */
		 /*NOP*/;
}

static void reset_timer_masked(void)
{
	/* reset time */
	lastdec = READ_TIMER;	/* capure current decrementer value time */
	timestamp = 0;		/* start "advancing" time stamp from 0 */
}

static ulong get_timer_masked(void)
{
	ulong now = READ_TIMER;	/* current tick value */

	if (lastdec >= now) {	/* normal mode (non roll) */
		/* normal mode */
		timestamp += lastdec - now;	/* move stamp fordward with absoulte diff ticks */
	}
	else {	/* we have overflow of the count down timer */
		/* nts = ts + ld + (TLV - now)
		 * ts=old stamp, ld=time that passed before passing through -1
		 * (TLV-now) amount of time after passing though -1
		 * nts = new "advancing time stamp"...it could also roll and cause problems.
		 */
		timestamp += lastdec + TIMER_LOAD_VAL - now;
	}
	lastdec = now;

	return timestamp;
}

/* waits specified delay value and resets timestamp */
static void udelay_masked(unsigned long usec)
{
	ulong tmo;
	ulong endtime;
	signed long diff;

	tmo = usec * (CFG_HZ / (1000 * 1000));

	endtime = get_timer_masked() + tmo;

	do {
		ulong now = get_timer_masked();
		diff = endtime - now;
	} while (diff >= 0);
}

/*
 * This function is derived from PowerPC code (read timebase as long long).
 * On ARM it just returns the timer value.
 */
static unsigned long long get_ticks(void)
{
	return get_timer(0);
}

/*
 * This function is derived from PowerPC code (timebase clock frequency).
 * On ARM it returns the number of timer ticks per second.
 */
static ulong get_tbclk(void)
{
	ulong tbclk;

	tbclk = CFG_HZ;
	return tbclk;
}

