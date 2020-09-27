//----------------------------------------------------------------------------
// Copyright 2019, Ed Keenan, all rights reserved.
//----------------------------------------------------------------------------

#include "ThreadCount.h"
#include "ThreadCountProxy.h"

ThreadCountProxy::ThreadCountProxy()
{
	ThreadCount::Increment();
}

ThreadCountProxy::~ThreadCountProxy()
{
	ThreadCount::Decrement();
}