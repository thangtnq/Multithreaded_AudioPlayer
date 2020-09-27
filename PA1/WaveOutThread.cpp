#include "WaveOutThread.h"

WaveOutThread::WaveOutThread():
	playable(false), alive(true)
{
	this->dataBuffer = new char[this->bufferSize];

	this->t_Handle = new WAVEHDR();
	memset(this->t_Handle, 0, sizeof(WAVEHDR));

	this->t_Handle->dwBufferLength = this->bufferSize;
	this->t_Handle->lpData = this->dataBuffer;
}

WaveOutThread::~WaveOutThread()
{
	delete this->t_Handle;
	delete[] this->dataBuffer;
	Trace::out("----------------------------------------------------WaveOutThread is dead.---------------------------------------\n");
}

void WaveOutThread::launch(HWAVEOUT data) {
	while (this->alive) {
		this->writeHeader(data);
	}
	Trace::out("WaveOutThread done writing header\n");
}

void WaveOutThread::writeHeader(HWAVEOUT data) {

	std::unique_lock<std::mutex> lock(this->p_mu);
	waveOutWrite(data, this->t_Handle, sizeof(WAVEHDR));
	this->playable = false;
	cv.wait(lock, [=]() { return this->playable; });
}

void WaveOutThread::prepareHeader(HWAVEOUT h, unsigned int t) {
	this->t_Handle->dwUser = (DWORD)t;
	waveOutPrepareHeader(h, this->t_Handle, sizeof(WAVEHDR));
}

void WaveOutThread::copyWaveBlock(LPSTR data, unsigned int size)
{
	std::lock_guard<std::mutex> lock(this->w_mu);
	memcpy(this->t_Handle->lpData, data, size);
}

void WaveOutThread::unprepareHeader(HWAVEOUT& h) {
	memset(this->t_Handle, 0, sizeof(WAVEHDR));
	memset(this->dataBuffer, 0, this->bufferSize);

	this->playable = true;
	this->alive = false;

	waveOutUnprepareHeader(h, this->t_Handle, sizeof(WAVEHDR));
	this->cv.notify_one();
}

void WaveOutThread::playableNow() {
	this->playable = true;
	this->cv.notify_one();
}

bool WaveOutThread::getStatusAlive() {
	std::lock_guard<std::mutex> lock(this->p_mu);
	bool a = this->alive;
	return a;
}