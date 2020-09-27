#ifndef FILETHREAD_H
#define FILETHREAD_H

#include "ThreadHelper.h"
#include "File_Slow.h"
#include <mutex>
#include <future>

class FileThread
{
public: //data
	static const unsigned int BuffSize = 512 * 1024;
	static const unsigned int NumWaveFiles = 23;
	static const unsigned int StringNameLength = 16;

	enum class BufferStatus
	{
		Empty,
		Filled,
		Done
	};

	enum class ThreadStatus
	{
		Active,
		Dead
	};

	struct waveName_t
	{
		char waveName[16];
	};

private: //data
	
	unsigned int			mCurrFileIndex;
	int						mCurrFileSize;
	
	BufferStatus				data_Status;
	ThreadStatus			fthread_Status;
	unsigned char			*pBuffer;	
	waveName_t				pWaveHolder[NumWaveFiles];

	std::mutex				f_Mutex;
	std::condition_variable f_cv;
	std::shared_future<void>	mFutureKill;
	

private: //method
	void privateLoad(unsigned int index);
	
public:
	FileThread(std::shared_future<void>& t_future);
	FileThread(const FileThread& data) = delete;
	FileThread& operator=(const FileThread& data) = delete;
	~FileThread();

	void operator()();

	ThreadStatus transferData(unsigned char* destination, unsigned int& size);
	ThreadStatus getStatus();
	void setStatus(ThreadStatus data);
};
#define UNUSED_VAR(x)  ((void )(x))

#endif