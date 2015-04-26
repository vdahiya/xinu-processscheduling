/* chprio.c - chprio
 *
 * Added code to update the dynamic priority in readylist, if we are using aging scheduler
 * */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <stdio.h>
#include <rescheduler.h>

/*------------------------------------------------------------------------
 * chprio  --  change the scheduling priority of a process
 *------------------------------------------------------------------------
 */
SYSCALL chprio(int pid, int newprio)
{
	STATWORD ps;    
	struct	pentry	*pptr;

	disable(ps);
	if (isbadpid(pid) || newprio<=0 ||
	    (pptr = &proctab[pid])->pstate == PRFREE) {
		restore(ps);
		return(SYSERR);
	}
	pptr->pprio = newprio;
	if(getschedclass() == AGINGSCHED)	// (1) added this code in if condition
	{
		if(pptr->pstate == PRREADY)
		{
			dequeue(pid);
			q[pid].qkey = newprio;
			insert(pid,rdyhead,pptr->pprio);
		}
	}
	restore(ps);
	return(newprio);
}
