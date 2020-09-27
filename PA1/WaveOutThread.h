#ifndef WAVEOUTTHREAD_H
#define WAVEOUTTHREAD_H


class WaveOutThread
{
public:
	WaveOutThread();
	WaveOutThread(const WaveOutThread&) = delete;
	WaveOutThread& operator=(const WaveOutThread&) = delete;
	~WaveOutThread();

	void launch(HWAVEOUT h);;
	void prepareHeader(HWAVEOUT h, unsigned int t);
	void copyWaveBlock(LPSTR pMemBuff, unsigned int bufferSize);
	void unprepareHeader(HWAVEOUT& hWaveOut);
	void playableNow();

	bool getStatusAlive();

	static const unsigned int bufferSize = 2 * 1024;
private:
	WAVEHDR* t_Handle;
	LPSTR dataBuffer;
	
	bool playable, alive;

	std::mutex p_mu;
	std::mutex w_mu;
	std::condition_variable cv;

	void writeHeader(HWAVEOUT d);
};

#endif
