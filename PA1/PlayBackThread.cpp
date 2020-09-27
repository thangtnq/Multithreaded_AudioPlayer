#include "PlayBackThread.h"

PlayBackThread::PlayBackThread()
{
	

	wfx.nSamplesPerSec = 22050; // sample rate 
	wfx.wBitsPerSample = 16;    // number of bits per sample of mono data 
	wfx.nChannels = 2;          // number of channels (i.e. mono, stereo...) 
	wfx.wFormatTag = WAVE_FORMAT_PCM;								// format type 
	wfx.nBlockAlign = (wfx.wBitsPerSample >> 3)* wfx.nChannels;	// block size of data 
	wfx.nAvgBytesPerSec = wfx.nBlockAlign * wfx.nSamplesPerSec;		// for buffer estimation 
	wfx.cbSize = 0;

	for (int i = 0; i < 20; i++) {
		this->waveOut_Objects[i] = new WaveOutThread();
	}
}

PlayBackThread::~PlayBackThread()
{
	for (int i = 0; i < 20; i++) {
		delete this->waveOut_Objects[i];
		Trace::out("------------------------------------------------------------Deleting stuff from playback\n");
	}
	Trace::out("----------------------------------------------------PlayBackThread is dead.\n");
}

void PlayBackThread::operator()(CoordinatorThread& pCoord) {
	
	WaveHelper wavePlayer;

	result = waveOutOpen(&this->hWaveOut, WAVE_MAPPER, &wfx, (DWORD_PTR)PlayBackThread::waveOutProc, (DWORD_PTR)&wavePlayer, CALLBACK_FUNCTION);
	if (result != MMSYSERR_NOERROR)
	{
		fprintf(stderr, "unable to open WAVE_MAPPER device\n");
		ExitProcess(1);
	}

	std::thread waveThreads[20];
	for (unsigned int i = 0; i < 20; i++) {
		this->waveOut_Objects[i]->prepareHeader(this->hWaveOut, i);
		pCoord.transfer_to_WaveOut(*waveOut_Objects[i]);
		wavePlayer.incrementWave();
		waveThreads[i] = std::thread(&WaveOutThread::launch, waveOut_Objects[i], std::ref(this->hWaveOut));
	}

	Trace::out("Playing by spinning\n");
	this->play(pCoord, wavePlayer);


	Trace::out("--------------------Done playing by spinning, now enter wait state\n");
	wavePlayer.waitDone();
	Trace::out("------------------------------------------------------------waiting for done, and it's done\n");

	for (unsigned int i = 0; i < 20; i++) {
		waveOut_Objects[i]->unprepareHeader(this->hWaveOut);
		waveThreads[i].detach();
	}

	wavePlayer.waitClose();
	waveOutClose(hWaveOut);

	Trace::out("wavethread joined successfully\n");
	Trace::out("--------------------------------PlayBack Thread going out of scope soon\n");
}

void PlayBackThread::play(CoordinatorThread& pC, WaveHelper& helper) {
	unsigned int i = 0;
	Trace::out("Hello!!!!!!!!!! I am playing here in PlayBack Thread\n");
	FileThread::ThreadStatus stat;
	while (pC.getCoordStatus() == FileThread::ThreadStatus::Active) {
		helper.waitWave();
		stat = pC.transfer_to_WaveOut(*this->waveOut_Objects[i]);
		if (stat == FileThread::ThreadStatus::Dead) {
			break;
		}
		helper.incrementWave();
		this->waveOut_Objects[i]->playableNow();
		i++;
		if (i > 19) {
			i = 0;
		}
	}
}

void CALLBACK PlayBackThread::waveOutProc(HWAVEOUT hWaveOut, UINT uMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2)
{
	UNUSED_VAR(dwParam2);
	UNUSED_VAR(hWaveOut);
	WaveHelper* pWavePlayer = (WaveHelper*)dwInstance;
	WAVEHDR* waveHdr = 0;

	switch (uMsg)
	{
	case WOM_DONE:
		waveHdr = (WAVEHDR*)dwParam1;
		pWavePlayer->decrementWave();

		if ( pWavePlayer->getWave() <= 0)
		{
			pWavePlayer->doneSignal();
		}
		Trace::out("WOM_DONE: remain:%d \n", pWavePlayer->getWave());
		break;

	case WOM_CLOSE:
		pWavePlayer->closeSignal();
		Trace::out("WOM_CLOSE:\n");
		break;

	case WOM_OPEN:
		Trace::out("WOM_OPEN:\n");
		break;

	default:
		assert(false);
	}
}

