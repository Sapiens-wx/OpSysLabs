#include <cstring>
#include <cstdlib>
#include <iostream>

#include "EStore.h"
#include "RequestGenerator.h"
#include "TaskQueue.h"

class Simulation {
    public:
    TaskQueue supplierTasks;
    TaskQueue customerTasks;
    EStore store;

    int maxTasks;
    int numSuppliers;
    int numCustomers;

    explicit Simulation(bool useFineMode) : store(useFineMode) { }
};

/*
 * ------------------------------------------------------------------
 * supplierGenerator --
 *
 *      The supplier generator thread. The argument is a pointer to
 *      the shared Simulation object.
 *
 *      Enqueue arg->maxTasks requests to the supplier queue, then
 *      stop all supplier threads by enqueuing arg->numSuppliers
 *      stop requests.
 *
 *      Use a SupplierRequestGenerator to generate and enqueue
 *      requests.
 *
 *      This thread should exit when done.
 *
 * Results:
 *      Does not return. Exit instead.
 *
 * ------------------------------------------------------------------
 */
static void*
supplierGenerator(void* arg)
{
    // TODO: Your code here.
	Simulation* sim=(Simulation*)arg;
	//initialize SupplierRequestGenerator
	SupplierRequestGenerator supplierRG(&sim->supplierTasks);
	//
	supplierRG.enqueueTasks(sim->maxTasks, &sim->store);
	supplierRG.enqueueStops(sim->numSuppliers);

	sthread_exit();
    return NULL; // Keep compiler happy.
}

/*
 * ------------------------------------------------------------------
 * customerGenerator --
 *
 *      The customer generator thread. The argument is a pointer to
 *      the shared Simulation object.
 *
 *      Enqueue arg->maxTasks requests to the customer queue, then
 *      stop all customer threads by enqueuing arg->numCustomers
 *      stop requests.
 *
 *      Use a CustomerRequestGenerator to generate and enqueue
 *      requests.  For the fineMode argument to the constructor
 *      of CustomerRequestGenerator, use the output of
 *      store.fineModeEnabled() method, where store is a field
 *      in the Simulation class.
 *
 *      This thread should exit when done.
 *
 * Results:
 *      Does not return. Exit instead.
 *
 * ------------------------------------------------------------------
 */
static void*
customerGenerator(void* arg)
{
    // TODO: Your code here.
	Simulation* sim = (Simulation*)arg;
	CustomerRequestGenerator customerRG(&sim->customerTasks, sim->store.fineModeEnabled());
	customerRG.enqueueTasks(sim->maxTasks, &sim->store);
	customerRG.enqueueStops(sim->numCustomers);

	sthread_exit();
    return NULL; // Keep compiler happy.
}

/*
 * ------------------------------------------------------------------
 * supplier --
 *
 *      The main supplier thread. The argument is a pointer to the
 *      shared Simulation object.
 *
 *      Dequeue Tasks from the supplier queue and execute them.
 *
 * Results:
 *      Does not return.
 *
 * ------------------------------------------------------------------
 */
static void*
supplier(void* arg)
{
    // TODO: Your code here.
	Simulation* sim=(Simulation*)arg;
	while(true){
		Task t = sim->supplierTasks.dequeue();
		t.handler(t.arg);
	}
    return NULL; // Keep compiler happy.
}

/*
 * ------------------------------------------------------------------
 * customer --
 *
 *      The main customer thread. The argument is a pointer to the
 *      shared Simulation object.
 *
 *      Dequeue Tasks from the customer queue and execute them.
 *
 * Results:
 *      Does not return.
 *
 * ------------------------------------------------------------------
 */
static void*
customer(void* arg)
{
    // TODO: Your code here.
	Simulation* sim=(Simulation*)arg;
	while(true){
		Task t = sim->customerTasks.dequeue();
		t.handler(t.arg);
	}
    return NULL; // Keep compiler happy.
}

/*
 * ------------------------------------------------------------------
 * startSimulation --
 *      Create a new Simulation object. This object will serve as
 *      the shared state for the simulation. 
 *
 *      Create the following threads:
 *          - 1 supplier generator thread.
 *          - 1 customer generator thread.
 *          - numSuppliers supplier threads.
 *          - numCustomers customer threads.
 *
 *      After creating the worker threads, the main thread
 *      should wait until all of them exit, at which point it
 *      should return.
 *
 *      Hint: Use sthread_join.
 *
 * Results:
 *      None.
 *
 * ------------------------------------------------------------------
 */
static void
startSimulation(int numSuppliers, int numCustomers, int maxTasks, bool useFineMode)
{
    // TODO: Your code here.
	Simulation sim(useFineMode);
	sim.maxTasks=maxTasks;
	sim.numSuppliers=numSuppliers;
	sim.numCustomers=numCustomers;

	sthread_t supplierGeneratorTH, customerGeneratorTH;
	sthread_create(&supplierGeneratorTH, supplierGenerator, &sim);
	sthread_create(&customerGeneratorTH, customerGenerator, &sim);
	//customer and supplier threads
	sthread_t* suppliers=(sthread_t*)malloc(sizeof(sthread_t)*numSuppliers);
	sthread_t* customers=(sthread_t*)malloc(sizeof(sthread_t)*numCustomers);
	for(int i=0;i<numSuppliers;++i){
		sthread_create(suppliers+i, supplier, &sim);
	}
	for(int i=0;i<numCustomers;++i){
		sthread_create(customers+i, customer, &sim);
	}
	//join threads
	sthread_join(supplierGeneratorTH);
	sthread_join(customerGeneratorTH);
	for(int i=0;i<numSuppliers;++i){
		sthread_join(suppliers[i]);
	}
	for(int i=0;i<numCustomers;++i){
		sthread_join(customers[i]);
	}

	return;
}

int main(int argc, char **argv)
{
    bool useFineMode = false;

    // Seed the random number generator.
    // You can remove this line or set it to some constant to get deterministic
    // results, but make sure you put it back before turning in.
    srand(time(NULL));

    if (argc > 1)
        useFineMode = strcmp(argv[1], "--fine") == 0;
    startSimulation(10, 10, 100, useFineMode);
    return 0;
}

