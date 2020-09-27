#ifndef COORDINATORTHREAD_H
#define COORDINATORTHREAD_H

#include "FileThread.h"
#include "WaveOutThread.h"
class CoordinatorThread
{

public:
	CoordinatorThread();
	CoordinatorThread(const CoordinatorThread&) = delete;
	CoordinatorThread& operator=(const CoordinatorThread&) = delete;
	~CoordinatorThread();

	void operator()(FileThread&);
	FileThread::ThreadStatus transfer_to_WaveOut(WaveOutThread& data);
	FileThread::ThreadStatus getCoordStatus();

private:

	unsigned char*				dataBuffer[2];
	unsigned char*				activeBuffer;

	unsigned int				bufferSize[2];
	unsigned int				index;

	FileThread::ThreadStatus	cThread_status;
	FileThread::BufferStatus	buffer_status;

	std::mutex					data_mutex;
	std::mutex					swap_mutex;
	std::condition_variable		c_cv;
	std::shared_future<void>	mFutureKill;

	void swappingBuffer();
};

#endif