#include "prioQ.h"
#include "xen_timer.h"
#include <xmalloc.h>

#define HEAP_SIZE 11    /*Most ever seen was 5*/

#define DEBUG 0
#if DEBUG
#define PRINTF(...) printk(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

/*The heap
 *It can have a HEAP_SIZE -1 elements, ignore heap[0]
 */
static struct event *heap;

static int *position;

void printHeap(int *q)
{	
	int i =0;
	for(i ; i < 6; i++)
	{
		printk("POSITION : %d  VALUE: %lld, TIMER : %c", i, heap[i].due, heap[i].timer);
		printk("\n");
	}
	printk("\n******************\n");
}
/*------------------------------------------------------------------*/
void swap(struct event *a, struct event *b)
{
	struct event temp;

	temp = *a;
	*a=*b;
	*b=temp;

}
/*----------------------------------------------------------------------*/
/* This reestablishes the heap property when adding a new item
 * @param x[] : This is the heap
 * @param n   : This is the current last position in the heap
 */
void siftUp(struct event x[], int n)
{
	int p, i = n;

	while(i > 1)
	{
		p = i/2;
		if(x[p].due <= x[i].due)
			break;
		swap(x+p, x+i);
		i = p;
	}
}

/*----------------------------------------------------------------------*/
/*This reestablishes the heap propterty after removing the root*/
void siftDown(struct event x[], int n)
{
	int c, i;
	
	i =1;
	while(1)
	{
		c = 2 * i;
		if(c > n)			/*Check this*/
			break;
		if((c + 1) <= n)
		{		
			if(x[(c+1)].due < x[c].due)
				c++;
		}	
		if(x[i].due <= x[c].due)
			break;
		swap(x+c, x+i);
		i=c;
	}
}

/*----------------------------------------------------------------------------*/
/* This creates a queue and returns a pointer tot eh edn of the heap*/
int q_create()
{
heap = (struct event *) malloc(sizeof(struct event) * HEAP_SIZE);

	int i =0;
	for(i ; i < HEAP_SIZE ; i++)
	{
		heap[i].due = 0;
		heap[i].timer = ' ';
	}

	heap[0].timer = 'X';
	/*As all heaps start at 1*/
	return 1;
}
/*----------------------------------------------------------------------------*/
/*This change adds an element to the heap and preseves the heap proerty, also udpates the 
 *end pointer
 */
void q_add(int *q, void *te)
{
	int pos = *q;
	struct event *t = (struct event *)te;

	int i = 0;
	for(i ; i < pos; i++)
	{
		if(heap[i].timer == t->timer && heap[i].due <= t->due)
			return;
	}

	memcpy(heap+pos, t, sizeof(struct event));
	siftUp(heap, pos);
	
	
	PRINTF("The desired val %c \n", t->timer);
	PRINTF("The value %d\n", pos);
	PRINTF("1The timer val %s \n" ,(heap+pos)->timer);
	PRINTF("2The timer val %c \n" ,heap[pos].timer);
	PRINTF("2The timer val %lld\n" ,heap[pos].due);
	
	PRINTF("PRIO Q ADDING: timer %c, timer %lld \n" ,heap[pos].timer, heap[pos].due);
	PRINTF("PRIO Q: new position %d\n\n", pos);
	pos++;
	memcpy(q ,&(pos), sizeof(int));
}
/*----------------------------------------------------------------------------*/
/*The removes the firs element of the heap, restores the heap roperty and updates the pointer*/
void q_remove(int *q , void *te)
{

	int pos = *q;
	//printk("PRIOQ: The position : %d\n", pos);
	
	memcpy(te, heap+1, sizeof(struct event));
	//printk("PRIO Q REMOVING: timer %c, timer %lld \n" ,heap[1].timer, heap[1].due);

	heap[1].timer = 'D';
	heap[1].due = 9999999999999999;

	/*Move the pointer back into the right place*/
	pos--;
	memcpy(q ,&(pos), sizeof(int));
	/*Swap the head and the tail*/

	swap(heap+1, heap+pos);

	siftDown(heap, pos);
	
	
}
/*----------------------------------------------------------------------------*/
/*This removes the first instance of a specified timer from the heap
 *very tighlty couple and used for premptive additions to the heap by the timers
 **/
void q_delete_first_timer(int *q, char timer)
{
	int pos = *q;
	int i = 0;
	
 	/*put the even to delet in the first position*/
	for(i ; i < pos ; i++)
	{
		if(heap[i].timer == timer)
			swap(heap+1, heap+i);
	}

	/*Get the new position*/
	pos--;
	memcpy(q ,&(pos), sizeof(int));

	/*Restore the heap property*/
	siftDown(heap, pos);

		
}
