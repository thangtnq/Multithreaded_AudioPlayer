//----------------------------------------------------------------------------
// Copyright 2019, Ed Keenan, all rights reserved.
//----------------------------------------------------------------------------

#ifndef THREAD_COUNT_PROXY_H
#define THREAD_COUNT_PROXY_H

#include "ThreadCount.h"

class ThreadCountProxy
{
public:
	ThreadCountProxy();
	ThreadCountProxy(const ThreadCountProxy&) = delete;
	ThreadCountProxy& operator = (const ThreadCountProxy&) = delete;
	~ThreadCountProxy();

};

#define KILL_THREAD_ENABLE ThreadCountProxy tcp;

#endif
