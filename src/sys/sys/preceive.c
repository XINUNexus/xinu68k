/* preceive.c - preceive */

#include <conf.h>
#include <kernel.h>
#include <mark.h>
#include <ports.h>

/*------------------------------------------------------------------------
 *  preceive  --  receive a message from a port, blocking if port empty
 *------------------------------------------------------------------------
 */
SYSCALL	preceive(portid)
int portid;
{
	struct	pt	*ptptr;
	int	seq;
	WORD	msg;
	struct	ptnode	*nxtnode;

#ifdef	DEBUG
	dotrace("preceive", &portid, 1);
#endif
	disable();
	if ( isbadport(portid) ||
#ifdef	MEMMARK
	     unmarked(ptmark) ||
#endif
	     (ptptr= &ports[portid])->ptstate != PTALLOC ) {
		restore();
		return(SYSERR);
	}

	/* wait for message and verify that the port is still allocated */

	seq = ptptr->ptseq;
	if (wait(ptptr->ptrsem) == SYSERR || ptptr->ptstate != PTALLOC
	    || ptptr->ptseq != seq) {
		restore();
		return(SYSERR);
	}

	/* dequeue first message that is waiting in the port */

	nxtnode = ptptr->pthead;
	msg = nxtnode->ptmsg;
	if (ptptr->pthead == ptptr->pttail)	/* delete last item	*/
		ptptr->pthead = ptptr->pttail = (struct ptnode *)NULL;
	else
		ptptr->pthead = nxtnode->ptnext;
	nxtnode->ptnext = ptfree;		/* return to free list	*/
	ptfree = nxtnode;
	signal(ptptr->ptssem);
	restore();
	return(msg);
}
