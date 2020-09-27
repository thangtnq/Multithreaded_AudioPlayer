#include "FileThread.h"
#include "ThreadCountProxy.h"

using namespace std::chrono_literals;

FileThread::FileThread(std::shared_future<void>& t_future) :
	data_Status(BufferStatus::Empty), fthread_Status(ThreadStatus::Active), mCurrFileIndex(0), mCurrFileSize(0),
	mFutureKill(t_future)
{
	Trace::out("File thread started\n");

	this->pBuffer = new unsigned char[FileThread::BuffSize]();
	char tmpWaveName[16];

	for (int i = 0; i < NumWaveFiles; i++)
	{
		memset(tmpWaveName, 0, StringNameLength);
		sprintf_s(tmpWaveName, StringNameLength, "wave_%d.wav", i);
		memcpy(&this->pWaveHolder[i].waveName, tmpWaveName, StringNameLength);
	}
};

FileThread::~FileThread() {
	delete[] this->pBuffer;
	Trace::out("----------------------------------------------------FileThread is dead.---------------------------------------\n");
}

void FileThread::operator()() {
	Trace::out("FileThread started\n");

	KILL_THREAD_ENABLE
	
	while(!this->mFutureKill._Is_ready())
	{
	for (int i = 0; i < 23; i++) {
		this->privateLoad(i);
		this->f_cv.notify_one();
		while (this->data_Status == BufferStatus::Filled) {
			//keep spinning
			Trace::out("Im spinning because I am filled\n");
			std::unique_lock<std::mutex> lock(this->f_Mutex);
			if (this->data_Status == BufferStatus::Empty) {
				lock.unlock();
				break;
			}
			lock.unlock();
			std::this_thread::sleep_for(100ms);
		}
	}

	}

	Trace::out("I am done. Prepare for exit\n");
	this->data_Status = BufferStatus::Done;
	this->fthread_Status = ThreadStatus::Dead;

	this->f_cv.notify_all();
	Trace::out("I notify others\n");
	Trace::out("--------------------------------File Thread going out of scope soon\n");
}

void FileThread::privateLoad(unsigned int index) 
{
	File_Slow::Handle fh;
	char* pWaveName = this->pWaveHolder[index].waveName;

	File_Slow::open(fh, pWaveName, File_Slow::READ);
	File_Slow::seek(fh, File_Slow::Seek::END, 0);

	//start locking
	Trace::out("Reading-----------Lock\n");
	std::lock_guard<std::mutex> lock(this->f_Mutex);
	File_Slow::tell(fh, this->mCurrFileSize);
	File_Slow::seek(fh, File_Slow::Seek::BEGIN, 0);
	
	Trace::out("File: loading wave[%d]: %s size:%d \n", index, pWaveName, this->mCurrFileSize);
	
	File_Slow::read(fh, this->pBuffer, this->mCurrFileSize);
	this->data_Status = BufferStatus::Filled;
	//end locking
	Trace::out("Reading-----------Unlock\n");
	
	Trace::out("Done reading \n");
	File_Slow::close(fh);
	Trace::out("Finish private load method\n");
}

FileThread::ThreadStatus FileThread::transferData(unsigned char* destination, unsigned int& size)
{
	Trace::out("		Create a unique lock\n");
	std::unique_lock<std::mutex> lock_Transfer(this->f_Mutex);
	Trace::out("		Wait for signal from launch\n");
	this->f_cv.wait(lock_Transfer, [this]() {return this->data_Status   == BufferStatus::Filled
												 || this->fthread_Status == ThreadStatus::Dead; });

	Trace::out("		Receive signal, start copying\n");
	if (this->fthread_Status == ThreadStatus::Active) {
		memcpy(destination, this->pBuffer, this->mCurrFileSize);
		size = this->mCurrFileSize;
		this->data_Status = BufferStatus::Empty;
		this->f_cv.notify_one();
		lock_Transfer.unlock();
		Trace::out("		Done transferring, unlock successfully\n");
		return this->fthread_Status;
	}
	lock_Transfer.unlock();
	Trace::out("		Done transferring, unlock successfully\n");
	return this->fthread_Status;
}

FileThread::ThreadStatus FileThread::getStatus() {
	std::lock_guard<std::mutex> lock(this->f_Mutex);
	FileThread::ThreadStatus a = this->fthread_Status;
	return a;
}

void FileThread::setStatus(FileThread::ThreadStatus data) {
	std::lock_guard<std::mutex> lock(this->f_Mutex);
	this->fthread_Status = data;
}