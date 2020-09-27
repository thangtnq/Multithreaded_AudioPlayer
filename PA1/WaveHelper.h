#ifndef WAVEHELPER_H
#define WAVEHELPER_H
class WaveHelper
{
public:
	WaveHelper();
	WaveHelper(const WaveHelper&) = delete;
	WaveHelper& operator=(const WaveHelper&) = delete;
	~WaveHelper();

	void incrementWave();
	void decrementWave();

	void doneSignal();
	void closeSignal();

	void waitWave();
	void waitDone();
	void waitClose();

	int getWave();

	bool done, closed;
private:

	int wave;
	std::mutex mu;
	std::condition_variable cv;

};
#endif

