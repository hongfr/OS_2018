// scheduler.cc
//  Routines to choose the next thread to run, and to dispatch to
//  that thread.
//
//  These routines assume that interrupts are already disabled.
//  If interrupts are disabled, we can assume mutual exclusion
//  (since we are on a uniprocessor).
//
//  NOTE: We can't use Locks to provide mutual exclusion here, since
//  if we needed to wait for a lock, and the lock was busy, we would
//  end up calling FindNextToRun(), and that would put us in an
//  infinite loop.
//
//  Very simple implementation -- no priorities, straight FIFO.
//  Might need to be improved in later assignments.
//
// Copyright (c) 1992-1996 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "debug.h"
#include "scheduler.h"
#include "main.h"

//----------------------------------------------------------------------
// Scheduler::Scheduler
//  Initialize the list of ready but not running threads.
//  Initially, no ready threads.
//----------------------------------------------------------------------

// Modified !!!!!!!!!!!!!!
int compare_time(Thread *T1, Thread *T2)
{
    int T1_time = T1->approximate_burst;
    int T2_time = T2->approximate_burst;
    if (T1_time < T2_time)
        return -1;
    else if (T1_time == T2_time)
        return 0;
    else if (T1_time > T2_time)
        return 1;
}
int compare_priority(Thread *T1, Thread *T2)
{
    int T1_priority = T1->priority;
    int T2_priority = T2->priority;
    // Sorted list always pop the smallest

    // But we want the largest priority -->Reverse the comparison
    if (T1_priority < T2_priority)
        return 1;
    else if (T1_priority == T2_priority)
        return 0;
    else if (T1_priority > T2_priority)
        return -1;
}
// Modified !!!!!!!!!!!!!!

Scheduler::Scheduler()
{

    // Modified !!!!!!!!!!!!!!
    L1_list = new SortedList<Thread *>(compare_time);     //shortest job first
    L2_list = new SortedList<Thread *>(compare_priority); // highest priority first
    L3_list = new List<Thread *>;

    // Modified !!!!!!!!!!!!!!
    // readyList = new List<Thread *>;
    toBeDestroyed = NULL;
}

//----------------------------------------------------------------------
// Scheduler::~Scheduler
//  De-allocate the list of ready threads.
//----------------------------------------------------------------------

Scheduler::~Scheduler()
{
    delete L1_list;
    delete L2_list;

    // for (int i=0;i<50;i++)
    // {
    //     delete L2_list[i];
    // }
    delete L3_list;
    // delete readyList;
}

//----------------------------------------------------------------------
// Scheduler::ReadyToRun
//  Mark a thread as ready, but not running.
//  Put it on the ready list, for later scheduling onto the CPU.
//
//  "thread" is the thread to be put on the ready list.
//----------------------------------------------------------------------

void Scheduler::ReadyToRun(Thread *thread)
{
    ASSERT(kernel->interrupt->getLevel() == IntOff);
    // DEBUG(dbgThread, "Putting thread on ready list: " << thread->getName());
    //cout << "Putting thread on ready list: " << thread->getName() << endl ;
    thread->setStatus(READY);
    // readyList->Append(thread);

    // Modified !!!!!!!!!!!!!!
    if (thread->priority < 50)
    {
        DEBUG(dbgSch, "\n## Tick ["<< kernel->stats->totalTicks << "]:Thread[" << thread->getID() <<"] is inserted into queue L3 " << "// "<< thread->getName());
        L3_list->Append(thread);
    }
    else if (thread->priority < 100)
    {
        DEBUG(dbgSch, "\n## Tick ["<< kernel->stats->totalTicks << "]:Thread[" << thread->getID() <<"] is inserted into queue L2 " << "// "<< thread->getName());
        L2_list->Insert(thread);
    }
    else if (thread->priority < 150)
    {
        DEBUG(dbgSch, "\n## Tick ["<< kernel->stats->totalTicks << "]:Thread[" << thread->getID() <<"] is inserted into queue L1 " << "// "<< thread->getName());
        L1_list->Insert(thread);
    }
    else
    {
        DEBUG(dbgSch, "Invalid Priority: " << thread->priority);
        ASSERTNOTREACHED();
    }
    // Modified !!!!!!!!!!!!!!
}

//----------------------------------------------------------------------
// Scheduler::FindNextToRun
//  Return the next thread to be scheduled onto the CPU.
//  If there are no ready threads, return NULL.
// Side effect:
//  Thread is removed from the ready list.
//----------------------------------------------------------------------

Thread *
Scheduler::FindNextToRun()
{
    Thread * t;
    ASSERT(kernel->interrupt->getLevel() == IntOff);
    if (! (L1_list->IsEmpty()))
    {

        bool removeFront = FALSE;
        if(kernel->currentThread->getStatus() != RUNNING)
        {
            removeFront = TRUE;
        }

        int cur_remain = kernel->currentThread->approximate_burst - kernel->currentThread->cur_cpu_burst;
        t = L1_list->Front();
        if(kernel->currentThread->priority / 50 <=1)
            removeFront = TRUE;

        if(cur_remain > t->approximate_burst)
        {
            removeFront = TRUE;
        }

        if(removeFront)
        {
            t = L1_list->RemoveFront(); 
            t->aging = 0;
            DEBUG(dbgSch, "## Tick ["<< kernel->stats->totalTicks << "]:Thread[" << t->getID() 
                <<"] is removed from queue L1 " << "// "<< t->getName());
            DEBUG(dbgSch, "##Tick ["<< kernel->stats->totalTicks << "]:Thread[" << t->getID() 
                <<"] is now selected for execution" << "// "<< t->getName());
    
            return t;        
        }
        else
        {
            // cout << "Return current Thread" << endl;
            return kernel->currentThread;
        }
    }
    // L2 is non preemptive, select new one only when previous thread is sleeping(or finisg)
    else if (! (L2_list->IsEmpty()))
    {
        // Sleeping Thread
        if(kernel->currentThread->getStatus() != RUNNING)
        {
            t = L2_list->RemoveFront();
            t->aging = 0;
            DEBUG(dbgSch, "## Tick ["<< kernel->stats->totalTicks << "]:Thread[" << t->getID() 
                <<"] is removed from queue L2 " << "// "<< t->getName());
            
            DEBUG(dbgSch, "## Tick ["<< kernel->stats->totalTicks << "]:Thread[" << t->getID() 
                <<"] is now selected for execution" << "// "<< t->getName());
            return t;
        }
        else
        {
            // cout << "Return current Thread" << endl;
            return kernel->currentThread;
        }
        
    }
    else if (! (L3_list->IsEmpty()))
    {
        // Timer interrupt and running thread is in L3                                   or             sleeping thread
        if(kernel->interrupt->timer_interrupt && (kernel->currentThread->priority < 50) || kernel->currentThread->getStatus() != RUNNING)
        {
            t = L3_list->RemoveFront();
            t->aging = 0;
            DEBUG(dbgSch, "## Tick ["<< kernel->stats->totalTicks << "]:Thread[" << t->getID() 
                <<"] is removed from queue L3 " << "// "<< t->getName());
            DEBUG(dbgSch, "## Tick ["<< kernel->stats->totalTicks << "]:Thread[" << t->getID() 
                <<"] is now selected for execution" << "// "<< t->getName());
            return t;
        }
        else
        {
            // cout << "Return current Thread" << endl;
            return kernel->currentThread;
        }
    }
    else
    {
        return NULL;
    }
    // if (readyList->IsEmpty())
    // {
    //     return NULL;
    // }
    // else
    // {
    //     return readyList->RemoveFront();
    // }
}

//----------------------------------------------------------------------
// Scheduler::Run
//  Dispatch the CPU to nextThread.  Save the state of the old thread,
//  and load the state of the new thread, by calling the machine
//  dependent context switch routine, SWITCH.
//
//      Note: we assume the state of the previously running thread has
//  already been changed from running to blocked or ready (depending).
// Side effect:
//  The global variable kernel->currentThread becomes nextThread.
//
//  "nextThread" is the thread to be put into the CPU.
//  "finishing" is set if the current thread is to be deleted
//      once we're no longer running on its stack
//      (when the next thread starts running)
//----------------------------------------------------------------------

void Scheduler::Run(Thread *nextThread, bool finishing)
{
    Thread *oldThread = kernel->currentThread;

    ASSERT(kernel->interrupt->getLevel() == IntOff);

    if (finishing)
    { // mark that we need to delete current thread
        ASSERT(toBeDestroyed == NULL);
        toBeDestroyed = oldThread;
    }

    if (oldThread->space != NULL)
    {                               // if this thread is a user program,
        oldThread->SaveUserState(); // save the user's CPU registers
        oldThread->space->SaveState();
    }

    oldThread->CheckOverflow(); // check if the old thread
                                // had an undetected stack overflow
    if(oldThread->getStatus() == RUNNING)
    {
        DEBUG(dbgSch, "## Tick ["<< kernel->stats->totalTicks << "]:Thread[" << oldThread->getID() <<"] is replaced, and it has executed[" << oldThread->cur_cpu_burst << "] ticks"<< "// "<< oldThread->getName());
    }


    kernel->currentThread = nextThread; // switch to the next thread
    nextThread->setStatus(RUNNING);     // nextThread is now running

    DEBUG(dbgThread, "Switching from: " << oldThread->getName() << " to: " << nextThread->getName());
               
    // This is a machine-dependent assembly language routine defined
    // in switch.s.  You may have to think
    // a bit to figure out what happens after this, both from the point
    // of view of the thread and from the perspective of the "outside world".

    // Modified !!!!!!!!!!!!!!
    if(nextThread != oldThread){
    // if(TRUE){
        SWITCH(oldThread, nextThread);
        
        // we're back, running oldThread

        // interrupts are off when we return from switch!
        ASSERT(kernel->interrupt->getLevel() == IntOff);

        DEBUG(dbgThread, "Now in thread: " << oldThread->getName());

        CheckToBeDestroyed(); // check if thread we were running
                              // before this one has finished
                              // and needs to be cleaned up
        if (oldThread->space != NULL)
        {                                  // if there is an address space
            oldThread->RestoreUserState(); // to restore, do it.
            oldThread->space->RestoreState();
        }
    }
    // Modified !!!!!!!!!!!!!!

    // cout << "!!!Finish Scheduler::Run() "  << kernel->currentThread->getName()<< endl;
}

//----------------------------------------------------------------------
// Scheduler::CheckToBeDestroyed
//  If the old thread gave up the processor because it was finishing,
//  we need to delete its carcass.  Note we cannot delete the thread
//  before now (for example, in Thread::Finish()), because up to this
//  point, we were still running on the old thread's stack!
//----------------------------------------------------------------------

void Scheduler::CheckToBeDestroyed()
{
    if (toBeDestroyed != NULL)
    {
        delete toBeDestroyed;
        toBeDestroyed = NULL;
    }
}

//----------------------------------------------------------------------
// Scheduler::Print
//  Print the scheduler state -- in other words, the contents of
//  the ready list.  For debugging.
//----------------------------------------------------------------------
void Scheduler::Print()
{
    cout << "Ready list contents:\n";
    cout << "L1: ";
    L1_list->Apply(ThreadPrint);
    cout << "\nL2: ";
    L2_list->Apply(ThreadPrint);
    cout << "\nL3: ";
    L3_list->Apply(ThreadPrint);
}

void Scheduler::Aging()
{
    // cout << "!!!!!!!!Aging!!!!!!!!!" << endl;f
    // SortedList<Thread *> temp_L1 = new SortedList<Thread *>(compare_time);     //shortest job first
    SortedList<Thread *> *temp_L2 = new SortedList<Thread *>(compare_priority); // highest priority first
    List<Thread *> *temp_L3 = new List<Thread *>;
    // we do not age L1 since priority does not matters in L1
    // if (! (L1_list->IsEmpty()))
    // {
    //     t = L1_list->RemoveFront();
        
    // }
    // else 
    Thread* t;
    // Iterate through L2
    while (! (L2_list->IsEmpty()))
    {
        t = L2_list->RemoveFront();
        t->aging += 100; // currently, only timer will call Aging every 100 ticks
        
        if(t->aging >= 1500)
        {
            int new_priority = t->priority + 10 * (t->aging/1500);
            DEBUG(dbgSch, "## Tick ["<< kernel->stats->totalTicks << "]:Thread[" << t->getID() 
                <<"] Changes its priority from [" << t->priority <<"] to [" << new_priority << "]" <<"// "<< t->getName());
            t->priority = new_priority;
            t->aging -= 1500 * (t->aging/1500);
        }
        if(t->priority >= 100)
        {
            DEBUG(dbgSch, "## Tick ["<< kernel->stats->totalTicks << "]:Thread[" << t->getID() <<"] is removed from queue L2 " << "// "<< t->getName());
            L1_list->Insert(t);
            DEBUG(dbgSch, "## Tick ["<< kernel->stats->totalTicks << "]:Thread[" << t->getID() <<"] is inserted into queue L1 " << "// "<< t->getName());
        
        }
        else
        {
            temp_L2->Insert(t);
        }
    }
    delete L2_list;
    L2_list = temp_L2;

    // Iterate through L3
    while (! (L3_list->IsEmpty()))
    {
        t = L3_list->RemoveFront();
        t->aging += 100; // currently, only timer will call Aging every 100 ticks
        if(t->aging >= 1500)
        {
            int new_priority = t->priority + 10 * (t->aging/1500);
            DEBUG(dbgSch, "## Tick ["<< kernel->stats->totalTicks << "]:Thread[" << t->getID() 
                <<"] Changes its priority from [" << t->priority <<"] to [" << new_priority << "]" <<"// "<< t->getName());
            t->priority = new_priority;
            t->aging -= 1500 * (t->aging/1500);
        }
        if(t->priority >= 100)
        {
            DEBUG(dbgSch, "## Tick ["<< kernel->stats->totalTicks << "]:Thread[" << t->getID() <<"] is removed from queue L3 " << "// "<< t->getName());
            L2_list->Insert(t);
            DEBUG(dbgSch, "## Tick ["<< kernel->stats->totalTicks << "]:Thread[" << t->getID() <<"] is inserted into queue L2 " << "// "<< t->getName());
        
        }
        else
        {
            temp_L3->Append(t);
        }
    }
    delete L3_list;
    L3_list = temp_L3;
    return;
}