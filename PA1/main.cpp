//----------------------------------------------------------------------------
// Copyright 2019, Ed Keenan, all rights reserved.
//----------------------------------------------------------------------------

#include "ThreadHelper.h"

#include "File_Slow.h"
#include <mmsystem.h>
#include <stdio.h>
#include <future>
#include <conio.h>

#include "FileThread.h"
#include "CoordinatorThread.h"
#include "PlayBackThread.h"
#include "WaveOutThread.h"
#include "WaveHelper.h"
#include "ThreadCount.h"
#include "KillThread.h"

using namespace std::chrono_literals;

int main()
{
	ThreadHelper::SetCurrentThreadName("____MAIN____");
	ThreadHelper::DebugBegin(0);

	// ThreadCounter
	std::condition_variable  cv_ThreadCount;
	ThreadCount::SetCV(&cv_ThreadCount);

	// Signal Main->Kill Thread to start killing threads
	std::condition_variable  cv_KeyPress;

	// Signal Kill->All threads to self exit.. needs to be shared
	std::promise<void>	prom_Kill;
	std::shared_future<void> fut_Kill = prom_Kill.get_future();

	// Signal Kill->Main Thread all threads have ended, safe to leave main 
	std::promise<void>	prom_EndMain;
	std::future<void>	fut_EndMain = prom_EndMain.get_future();

	// Signal File->Coordinator that file is load ready for pulling
	std::promise<int>	prom_FileLoaded;
	std::future<int>	fut_FileLoaded = prom_FileLoaded.get_future();

	//----Thread Creations
	FileThread file_Object(fut_Kill);
	std::thread file_Thread(std::ref(file_Object));

	CoordinatorThread coord_Object;
	std::thread coord_Thread(std::ref(coord_Object), std::ref(file_Object));

	PlayBackThread play_Object;
	std::thread play_Thread(std::ref(play_Object), std::ref(coord_Object));

	// ----------------------------------------------------------------
	// spawn the threads
	//-----------------------------------------------------------------

	KillThread  t_Kill(prom_Kill, prom_EndMain, &cv_KeyPress, &cv_ThreadCount);
	t_Kill.Launch();

	//----Threads join
	file_Thread.join();
	coord_Thread.join();
	play_Thread.join();

	
	/*wavePlayer.waitClose();
	waveOutClose(hWaveOut);*/

	// ---------------------------------------------------------
	// Key press - quick exit
	// ---------------------------------------------------------

	UNUSED_VAR(_getch());       // any key press
	cv_KeyPress.notify_one();   // notify the kill thread

	// Do not leave main until kills are done
	fut_EndMain.get();

	ThreadHelper::DebugEnd(0);
}
