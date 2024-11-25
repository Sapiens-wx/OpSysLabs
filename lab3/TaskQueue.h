#pragma once


#include <queue>
#include "sthread.h"

typedef void (*handler_t) (void *); 

struct Task {
    handler_t handler;
    void* arg;
};

/*
 * ------------------------------------------------------------------
 * TaskQueue --
 * 
 *      A thread-safe task queue. This queue should be implemented
 *      as a monitor.
 *
 * ------------------------------------------------------------------
 */
class TaskQueue {
    private:
    // TODO: More needed here.
	std::queue<Task> q;
	smutex_t q_mutex;
	scond_t scond_enqueue;

    public:
    TaskQueue();
    ~TaskQueue();
    
    // no default copy constructor and assignment operators. this will prevent some
    // painful bugs by converting them into compiler errors.
    TaskQueue(const TaskQueue&) = delete;
    TaskQueue& operator=(const TaskQueue &) = delete;

    void enqueue(Task task);
    Task dequeue();

    int size();
    bool empty();
};

