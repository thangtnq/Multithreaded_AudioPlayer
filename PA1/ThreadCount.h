#ifndef THREAD_COUNT_H
#define THREAD_COUNT_H
class ThreadCount
{
public:
	ThreadCount(const ThreadCount&) = delete;
	ThreadCount& operator = (const ThreadCount&) = delete;
	~ThreadCount();

	static void SetCV(std::condition_variable* cv_ThreadCount);
	static void Increment();
	static void Decrement();

private:
	static ThreadCount* privGetInstance();
	ThreadCount();

	int						count;
	std::mutex				mtx_ThreadCount;
	std::condition_variable* pcv_ThreadCount;
};

#endif