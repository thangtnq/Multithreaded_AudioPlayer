#ifndef PLAYBACKTHREAD_H
#define PLAYBACKTHREAD_H

#include "WaveOutThread.h"
#include "CoordinatorThread.h"
#include "WaveOutThread.h"
#include "WaveHelper.h"

class PlayBackThread
{
public:
	PlayBackThread();
	PlayBackThread(const PlayBackThread&) = delete;
	PlayBackThread& operator=(const PlayBackThread&) = delete;
	~PlayBackThread();

	void operator()(CoordinatorThread& pCoor);
	static void CALLBACK waveOutProc(HWAVEOUT h, UINT u, DWORD dw, DWORD p1, DWORD p2);
private:
	WaveOutThread* waveOut_Objects[20];

	HWAVEOUT hWaveOut;
	WAVEFORMATEX wfx;
	MMRESULT result;

	void play(CoordinatorThread& data, WaveHelper& h);
};


#endif

