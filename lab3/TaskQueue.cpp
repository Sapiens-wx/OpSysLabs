
#include "TaskQueue.h"
#include <stdio.h>

TaskQueue::
TaskQueue()
{
    // TODO: Your code here.
	smutex_init(&q_mutex);
	scond_init(&scond_enqueue);
}

TaskQueue::
~TaskQueue()
{
    // TODO: Your code here.
	smutex_destroy(&q_mutex);
	scond_destroy(&scond_enqueue);
}

/*
 * ------------------------------------------------------------------
 * size --
 *
 *      Return the current size of the queue.
 *
 * Results:
 *      The size of the queue.
 *
 * ------------------------------------------------------------------
 */
int TaskQueue::
size()
{
    // TODO: Your code here.
	return q.size();
}

/*
 * ------------------------------------------------------------------
 * empty --
 *
 *      Return whether or not the queue is empty.
 *
 * Results:
 *      true if the queue is empty and false otherwise.
 *
 * ------------------------------------------------------------------
 */
bool TaskQueue::
empty()
{
    // TODO: Your code here.
	return q.empty();
}

/*
 * ------------------------------------------------------------------
 * enqueue --
 *
 *      Insert the task at the back of the queue.
 *
 * Results:
 *      None.
 *
 * ------------------------------------------------------------------
 */
void TaskQueue::
enqueue(Task task)
{
	smutex_lock(&q_mutex);
	q.push(task);
	scond_signal(&scond_enqueue, &q_mutex);
	smutex_unlock(&q_mutex);
}

/*
 * ------------------------------------------------------------------
 * dequeue --
 *
 *      Remove the Task at the front of the queue and return it.
 *      If the queue is empty, block until a Task is inserted.
 *
 * Results:
 *      The Task at the front of the queue.
 *
 * ------------------------------------------------------------------
 */
Task TaskQueue::
dequeue()
{
    // TODO: Your code here.
	smutex_lock(&q_mutex);
	while(q.empty()){
		scond_wait(&scond_enqueue, &q_mutex);
	}
	Task t=q.front();
	q.pop();
	smutex_unlock(&q_mutex);
	return t;
}

