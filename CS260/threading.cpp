/*
Demo code:  This application will spawn two threads.  These two threads will count from 0 to 100 and then
reset back to 0 and continue.  They will do this forever until you halt the application by pressing the enter key,
at which point the threads will shut down gracefully.

The way this works is by using an event.  The main thread creates an event and then passes it to the threads that it
creates by way of the ThreadInfo struct.  When that event is signaled, the threads should clean up and halt.  The main
thread will then wait until both of its worker threads have returned and then will quit.
*/

#include <windows.h>
#include <assert.h>
#include <iostream>


// we're going to create two threads.  Try changing this value and see what happens. . .
const int thread_count = 2;

// we're counting up to this value
const int max_count = 100;

// This will be how we pass data to our thread for it to be operated upon
struct ThreadInfo
{
	int* counter; // this is the number that we'll be counting with
	CRITICAL_SECTION crit; // this helps avoid a race condition between threads
	HANDLE terminationEvent; // this will signal the thread to exit
};

// This is the function that each thread will execute
// LPVOID is a fancy way of writing void* but windows wants it to look like this.
// This will get whatever data is passed in to CreateThread
DWORD WINAPI CountSomeNumbers(LPVOID param)
{
	ThreadInfo* info = (ThreadInfo*)param;

	while(1)
	{
		// We should exit if the event is set
		DWORD result =  WaitForSingleObject(info->terminationEvent, 0);
		if(result == WAIT_OBJECT_0)
		{
			// we're done

			// TRY:  what happens if we don't use the critical section here?
			// what would cout do?
			EnterCriticalSection(&info->crit);
			std::cout << "shutting down a thread" << std::endl;
			LeaveCriticalSection(&info->crit);
			return 0;
		}
		else if(result == WAIT_TIMEOUT)
		{
			// output some number.  Be sure that we don't cause a race
			// condition with other threads.
			EnterCriticalSection(&info->crit);
			(*(info->counter))++;
			if(*(info->counter) == max_count)
			{
				*(info->counter) = 0;
			}
			std::cout << *(info->counter) << std::endl;
			LeaveCriticalSection(&info->crit);
			// slow things down a bit so that you can actually see the output.
			// try changing this value and see what happens.
			Sleep(100);
		}
		else
		{
			assert(false);
			// epic fail somewhere
		}
	}
}


int main(void)
{

	// arguments:
	// NULL:  no security descriptor, use the default.  Normally, you'll do this
	// true:  this is a manual reset event (false would make this an auto reset event)
	// false: this event's initial state should be nonsignaled
	// NULL:  we are not giving this event a name, although you can reference events by name if you want to
	HANDLE terminationEvent = CreateEvent(NULL, true, false, NULL);
	CRITICAL_SECTION crit;
	InitializeCriticalSection(&crit);

	HANDLE threadHandles[thread_count];

	// this is how the threads are going to keep track of what number they're on
	int* counter = new int;
	*counter = 0;

	ThreadInfo* info = new ThreadInfo;
	info->counter = counter;
	// CRITICAL_SECTION and HANDLE are both pointers, so this code is safe.  Do NOT
	// pass &crit or &terminationEvent here, as those would be pointers to pointers and could potentially go
	// out of scope (they won't in this app, but it's good practice to avoid such things).
	info->crit = crit;
	info->terminationEvent = terminationEvent;

	for(int i = 0; i < thread_count; ++i)
	{
		// Let's create some threads!
		// arguments:
		// NULL:              the security descriptor, same as the event.
		//                    We don't care, so we'll use the default for this process

		// 0:                the stack size. 0 says to use the default stack size, but
		//                    we could make it different if we cared to (we don't)

		// CountSomeNumbers:  This is the function that our thread will imediately call

		// (void*)info:      This is passed as the argument to the function we're
		//                    calling in the thread (in this case, CountSomeNumbers)

		// 0:                This 0 tells the thread to start immediately.  We could tell
		//                    this thread to sleep when it starts up and wake it up later

		// NULL:              This would give us the ID of the thread, which can be very
		//                    useful, but we're not using it here so let's make it NULL
		threadHandles[i] = CreateThread(NULL, 0, CountSomeNumbers, (void*)info, 0, NULL);
	}

	// get some user input.  This blocks, but that's ok since our threads
	// will continue to count to 100 over and over again while this is waiting
	std::cin.get();
	std::cout << "end threads" << std::endl;

	// user pressed the "any" key, let's quit the app
	SetEvent(terminationEvent);

	std::cout << "waiting for threads" << std::endl;
	// now wait for the threads to end.  We want to wait for all of them and we want to wait forever
	WaitForMultipleObjects(thread_count, threadHandles, true, INFINITE);

	std::cout << "all threads exited" << std::endl;

	// don't forget to clean up all of this stuff
	CloseHandle(terminationEvent);
	for(int i = 0; i < thread_count; ++i)
	{
		CloseHandle(threadHandles[i]);
	}
	DeleteCriticalSection(&crit);
	delete info;
	delete counter;
	std::cout << "press enter to quit" << std::endl;
	std::cin.get();
}
