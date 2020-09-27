#include "KillThread.h"
#include "ThreadHelper.h"

using namespace std::chrono_literals;

KillThread::KillThread(std::promise<void>& t_promKill, std::promise<void>& t_promEnd, std::condition_variable* cv_KeyPress, std::condition_variable* cv_ThreadCount)
	: numTabs(1),
	mThread(),
	mPromiseKill(std::move(t_promKill)),
	mPromiseEnd(std::move(t_promEnd))
{
	pcv_KeyPress = cv_KeyPress;
	pcv_ThreadCount = cv_ThreadCount;
}

KillThread::~KillThread()
{
	// Every thread needs to be joined or detach before destruction
	if (this->mThread.joinable())
	{
		this->mThread.join();
	}
}

void KillThread::Launch()
{
	// make sure the thread wasn't started
	if (this->mThread.joinable() == false)
	{
		this->mThread = std::thread(std::ref(*this));
	}
	else
	{
		// Lock up
		assert(false);
	}
}

void KillThread::operator() ()
{
	ThreadHelper::SetCurrentThreadName(this->pName);
	ThreadHelper::DebugBegin(this->numTabs);

	// waiting for a key press...
	std::unique_lock<std::mutex> lock_key(this->mtx_KeyPress);
	this->pcv_KeyPress->wait(lock_key);

	ThreadHelper::TabMe(this->numTabs);
	Trace::out("(%s): key pressed <----  \n", ThreadHelper::GetThreadName());

	// kill them now
	this->mPromiseKill.set_value();

	// wait until all are done
	std::unique_lock<std::mutex> lock_ct(this->mtx_ThreadCount);
	this->pcv_ThreadCount->wait(lock_ct);

	this->mPromiseEnd.set_value();

	ThreadHelper::DebugEnd(this->numTabs);
}