#include "WaveHelper.h"

using namespace std::chrono_literals;

WaveHelper::WaveHelper():
	wave(0),done(false), closed(false)
{}

WaveHelper::~WaveHelper() {}

void WaveHelper::incrementWave() {
	std::lock_guard<std::mutex> lock(this->mu);
	this->wave++;
}

void WaveHelper::decrementWave() {
	std::lock_guard<std::mutex> lock(this->mu);
	this->wave--;
	this->cv.notify_one();
}

void WaveHelper::doneSignal() {
	std::lock_guard<std::mutex> lock(this->mu);
	this->done = true;
	this->cv.notify_one();
}

void WaveHelper::closeSignal() {
	std::lock_guard<std::mutex> lock(this->mu);
	this->closed = true;
	this->cv.notify_one();
	Trace::out("closeSignal sends signal here for closing ");
}

void WaveHelper::waitClose() {
	Trace::out("Did I get called?????????????????????????\n");
	std::unique_lock<std::mutex> lock(this->mu);
	Trace::out("I have a waiting condition right below\n");
	this->cv.wait(lock, [=]() { return this->closed; });
	Trace::out("I woke up from my waiting condition\n");
}

void WaveHelper::waitDone() {
	std::unique_lock<std::mutex> lock(this->mu);
	Trace::out("--------------------I just created a unique lock here in Helper\n");
	this->cv.wait(lock, [=]() { return this->done; });
	Trace::out("--------------------Im done waiting, time to go out of Helper\n");
}

void WaveHelper::waitWave() {
	std::unique_lock<std::mutex> lock(this->mu);
	this->cv.wait(lock, [=]() { return this->wave < 20; });
}

int WaveHelper::getWave() {
	std::lock_guard<std::mutex> lock(this->mu);
	int count = this->wave;
	return count;
}