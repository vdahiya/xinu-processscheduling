/* resched.c  -  resched */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>

#include <rescheduler.h>

unsigned long currSP;	/* REAL sp of current process */
extern int ctxsw(int, int, int, int);
extern int schedtype;

int default_resched();

/*-----------------------------------------------------------------------
 * resched  --  Calls the appropriate scheduling mechanism based on the
 * 				return value of getschedclass() method
 *------------------------------------------------------------------------
 */
int resched()
{
	switch( getschedclass() )
	{
		case AGINGSCHED :
		return aging_resched();
		break;

		case LINUXSCHED :
		return linux_resched();
		break;

		default :
		return default_resched();
		break;
	}

	return OK;
}

/*         changes the scheduling type to either of the supplied AGINGSCHED or LINUXSCHED  */
int setschedclass(int sched_class)
{
	switch( sched_class )
	{
		case AGINGSCHED:
			schedtype = AGINGSCHED;
			break;
		case LINUXSCHED:
			schedtype = LINUXSCHED;
			break;
		default:
			return(SYSERR);
	}

	return(sched_class);
}


/*         returns the scheduling class which should be either AGINGSCHED or LINUXSCHED    */
int getschedclass(void)
{
	return(schedtype);
}


/*	   changes the quantum rollover threshold					   */
void setrolloverthreshold(int threshold)
{
	rolloverthreshold = threshold;
}

/*-----------------------------------------------------------------------
 * default_resched  --  reschedule processor to highest priority ready process
 *
 * Notes:	Upon entry, currpid gives current process id.
 *		Proctab[currpid].pstate gives correct NEXT state for
 *			current process if other than PRREADY.
 *------------------------------------------------------------------------
 */
int default_resched()
{
	register struct	pentry	*optr;	/* pointer to old process entry */
	register struct	pentry	*nptr;	/* pointer to new process entry */

	/* no switch needed if current process priority higher than next*/

	if ( ( (optr= &proctab[currpid])->pstate == PRCURR) &&
	   (lastkey(rdytail)<optr->pprio)) {
	

#ifdef  RTCLOCK
        preempt = QUANTUM;              /* reset preemption counter     */
#endif
		return(OK);
	}
	
	/* force context switch */

	if (optr->pstate == PRCURR) {
		optr->pstate = PRREADY;
		insert(currpid,rdyhead,optr->pprio);
	}

	/* remove highest priority process at end of ready list */

	nptr = &proctab[ (currpid = getlast(rdytail)) ];
	nptr->pstate = PRCURR;		/* mark it currently running	*/
#ifdef	RTCLOCK
	preempt = QUANTUM;		/* reset preemption counter	*/
#endif
	
	ctxsw((int)&optr->pesp, (int)optr->pirmask, (int)&nptr->pesp, (int)nptr->pirmask);
	
	/* The OLD process returns here when resumed. */
	return OK;
}
