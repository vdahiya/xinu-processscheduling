/*   rescheduler.h - setschedclass, getschedclass, setrolloverthreshold */


#define AGINGSCHED 1
#define LINUXSCHED 2

int schedtype; 	/* holds the type of scheduler to use */

int rolloverthreshold;		/* used only for linux like scheduler. If the left over quantum is greater than
the threshold, bonus will be given to that process in the next quantum recalculation */



/*         changes the scheduling type to either of the supplied AGINGSCHED or LINUXSCHED  */
int setschedclass(int sched_class);


/*         returns the scheduling class which should be either AGINGSCHED or LINUXSCHED    */
int getschedclass(void);


/*	   changes the quantum rollover threshold					   */
void setrolloverthreshold(int threshold);


/* epoch list entry */

struct	epochentry	{
	int	pprio;			/* process priority		*/
	int enext;			/* pointer to next entry in epoch list or tail	*/
};

extern	struct	epochentry epochlst[];
