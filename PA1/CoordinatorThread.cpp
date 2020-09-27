#include "CoordinatorThread.h"

CoordinatorThread::CoordinatorThread() :
	index(0), activeBuffer(nullptr), cThread_status(FileThread::ThreadStatus::Active),
	buffer_status(FileThread::BufferStatus::Empty)
{
	for (int i = 0; i < 2; i++) {
		this->dataBuffer[i] = new unsigned char[FileThread::BuffSize];
		memset(this->dataBuffer[i], 0, FileThread::BuffSize);
		this->bufferSize[i] = 0;
	}
};

CoordinatorThread::~CoordinatorThread()
{
	delete[] this->dataBuffer[0];
	delete[] this->dataBuffer[1];
	Trace::out("----------------------------------------------------Coordinator is dead.---------------------------------------\n");
}

void CoordinatorThread::operator()(FileThread& data) {
	Trace::out("				Coordinator started\n");
	FileThread::ThreadStatus temp;
	while (data.getStatus() == FileThread::ThreadStatus::Active)
	{
		unsigned int i = 0x1 ^ this->index;
		temp = data.transferData(this->dataBuffer[i], this->bufferSize[i]);
		data.setStatus(temp);
		std::unique_lock<std::mutex> lock(this->data_mutex);
		this->c_cv.wait(lock, [=]() {return this->buffer_status == FileThread::BufferStatus::Empty; });
		this->swappingBuffer();
	}
	Trace::out("				End coordinator looping\n");
	std::unique_lock<std::mutex> lock(this->data_mutex);
	Trace::out("				Create coordinator ending unique lock\n");
	this->c_cv.wait(lock, [=]() {return this->buffer_status == FileThread::BufferStatus::Empty; });
	Trace::out("				Done waiting on coordinator thread\n");
	this->buffer_status = FileThread::BufferStatus::Filled;
	this->cThread_status = FileThread::ThreadStatus::Dead;

	this->c_cv.notify_one();
	Trace::out("--------------------------------Coordinator Thread going out of scope soon\n");
}

void CoordinatorThread::swappingBuffer()
{
	unsigned int rotate_Index = 0x1 ^ this->index;
	std::unique_lock<std::mutex> lock(this->swap_mutex);

	this->activeBuffer = this->dataBuffer[rotate_Index];
	this->bufferSize[this->index] = this->bufferSize[rotate_Index];
	this->index = rotate_Index;
	this->buffer_status = FileThread::BufferStatus::Filled;
	lock.unlock();
	this->c_cv.notify_one();
}

FileThread::ThreadStatus CoordinatorThread::transfer_to_WaveOut(WaveOutThread& data) {
	if (this->bufferSize[this->index] <= 0) {
		this->buffer_status = FileThread::BufferStatus::Empty;
		this->c_cv.notify_one();
		
		std::unique_lock<std::mutex> lock(this->swap_mutex);
		this->c_cv.wait(lock, [=]() {return this->buffer_status == FileThread::BufferStatus::Filled;});
	}

	if (this->cThread_status == FileThread::ThreadStatus::Active) {
		unsigned int size = WaveOutThread::bufferSize;
		LPSTR pBuff;

		std::lock_guard<std::mutex> lock(this->data_mutex);
		if (this->bufferSize[this->index] < size) {
			size = this->bufferSize[this->index];
			Trace::out("Last wave here Last wave here Last wave here Last wave here\n");
		}

		pBuff = (LPSTR)this->activeBuffer;

		this->activeBuffer += size;
		this->bufferSize[this->index] -= size;
		
		data.copyWaveBlock(pBuff, size);
		return this->cThread_status;
	}
	return this->cThread_status;
}

FileThread::ThreadStatus CoordinatorThread::getCoordStatus() {
	return this->cThread_status;
}