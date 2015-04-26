#include <q.h>
#include <kernel.h>
#include <proc.h>
#include <rescheduler.h>

extern int ctxsw(int, int, int, int);

#include<math.h>
#include<stdio.h>
int updprocprio()
{

        int temp, prio;

        for( temp = q[rdyhead].qnext; temp != rdytail; temp = q[temp].qnext)
        {
               	prio = q[temp].qkey;
               	prio =  (int)ceil( (float)prio * 1.2);
                if ( prio >  99) prio = 99;
       	        q[temp].qkey = prio;

        }
        return(OK);
}

/* Aging Scheduler */
int aging_resched()
{
	register struct qent *oqptr; 	/* pointer to the currently running process in queue entry table 		*/
	register struct pentry *opptr;	/* pointer to the currently running process in process entry table 	*/

	register struct qent *nqptr; 	/* pointer to the newly selected process in queue entry table 		*/
	register struct pentry *npptr;	/* pointer to the newly selected process in process entry table 	*/
	
	int opid = currpid;

	/* no switch needed if current process priority higher than next*/
	
	oqptr = &q[opid];
	opptr = &proctab[opid];
	
	/* set the priority of current process to base priority */
	q[currpid].qkey = proctab[currpid].pprio;

	if ( ( opptr->pstate == PRCURR) && (lastkey(rdytail) < oqptr->qkey)  ) {		

        updprocprio();          /* update priorities of all the processes in ready queue */

#ifdef  RTCLOCK
	        	preempt = QUANTUM;              /* reset preemption counter     */
#endif
		return(OK);
	}	

	/* remove highest priority process at end of ready list */

	npptr = &proctab[ (currpid = getlast(rdytail)) ];
	npptr->pstate = PRCURR;		/* mark it currently running	*/

	updprocprio();		/* update priorities of all the processes in ready queue */

        /* put old process in ready queue if it is the current executing process */

        if (opptr->pstate == PRCURR) {
                opptr->pstate = PRREADY;
                insert(opid,rdyhead,opptr->pprio);

        }

#ifdef  RTCLOCK
        	preempt = QUANTUM;              /* reset preemption counter     */
#endif

	ctxsw((int)&opptr->pesp, (int)opptr->pirmask, (int)&npptr->pesp, (int)npptr->pirmask);
	
	/* The OLD process returns here when resumed. */
	return OK;
}
