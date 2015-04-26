/*
 * linuxsched.c - linux-like scheduler
 */

#include <q.h>
#include <kernel.h>
#include <proc.h>
#include <rescheduler.h>
#include <math.h>
#include <stdio.h>

extern int ctxsw(int, int, int, int);

/*
 * module to assign fresh quantum to all processes
 * should be done when a new epoch starts
 */
void reassignquantum()
{
	struct pentry *tempptr;
	int temp, quantum, counter;

	for(temp = 0; temp < NPROC; temp++)
	{
		tempptr = &proctab[temp];

		if(tempptr->pstate != PRFREE)
		{
			counter = tempptr->counter;
			if (counter >=rolloverthreshold)
				quantum = (int)floor(counter/2) + tempptr->pprio;
			else
				quantum = tempptr->pprio;

				tempptr->counter = quantum;
		}
	}
}

/*
 * module to recreate epoch list
 */
void recreateepochlst()
{
	int temp, epochptr = NPROC;
	register struct pentry *ptr;

	for(temp = 0; temp < NPROC; temp++)
	{
		ptr = &proctab[temp];
		epochlst[temp].pprio = ptr->pprio;

		if(ptr->pstate == PRREADY || ptr->pstate == PRCURR)
		{
			epochlst[epochptr].enext = temp;					
			epochptr = temp;
		}
	}
	epochlst[epochptr].enext = NPROC + 1;

}

/*
 * calculates the goodness of a process
 */
int goodness(int procid)
{
	register struct	pentry	*ptr;	/* pointer to the process entry */
	ptr = &proctab[procid];

	if (ptr->counter == 0)
	       return 0;
	return ptr->counter + epochlst[procid].pprio;

}

/* LINUX Rescheduler */
int linux_resched()
{
	register struct pentry *opptr;	/* pointer to the currently running process in process entry table 	*/
	register struct pentry *npptr;	/* pointer to the newly selected process in process entry table 	*/

	opptr = &proctab[currpid];

	opptr->counter = opptr->counter + preempt;

	/* module to scan epoch list and find the process with highest goodness
	* This scan has been written assuming there will be at least 'prnull' process in the
	* epochlst all the time
	*/
	int prev, next, p, c, weight;	/* prev will have the pid which just relinquished cpu,
									next, c will have the pid, goodness of the process
									which is going for execution */
	prev = currpid;
	next = epochlst[NPROC].enext;	// points to the very first process in epoch list
	c = goodness(next);

	p = epochlst[next].enext;
	while (p != NPROC+1) 
	{
		if(proctab[p].pstate == PRREADY || proctab[p].pstate == PRCURR)
		{
			weight = goodness(p);
			if (weight > c) 
			{
				c = weight;
				next = p;
			}
			else if( weight == c && weight > 0)
			{
				c = weight;
				next = p;
			}		
		}
		p = epochlst[p].enext;
	}

	/* If c is 0, it means that all the processes have exhausted their time quantum.
	 * A new epoch has to be started freshly.
	 */
	if(c==0)
	{

		int i =0 ;
		reassignquantum();		
		recreateepochlst();
	}

    if(prev != next)
    {

    	/* make the highest goodness process as currently running */
    	 npptr = &proctab[ (currpid = dequeue(next)) ];
    	 npptr->pstate = PRCURR;

#ifdef  RTCLOCK
    	 preempt = npptr->counter;              		/* reset preemption counter     */
	 npptr->counter = 0;
	if(next == 0) preempt = QUANTUM; 
#endif

    	 /* put old process in ready queue if it is the current executing process */

         if (opptr->pstate == PRCURR) {
        	 opptr->pstate = PRREADY;
    	     insert(prev,rdyhead,opptr->pprio);
         }

    	 ctxsw((int)&opptr->pesp, (int)opptr->pirmask, (int)&npptr->pesp, (int)npptr->pirmask);
    }
    else
    {

#ifdef  RTCLOCK
 	preempt = opptr->counter;              /* reset preemption counter     */
	if(prev == 0) preempt = QUANTUM;
	opptr->counter = 0;
#endif
    }

     /* The OLD process returns here when resumed. */
     return OK;
}
