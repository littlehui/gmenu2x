#include "clock.h"

#include "debug.h"
#include "inputmanager.h"
#include "utilities.h"

#include <SDL.h>
#include <atomic>
#include <chrono>
#include <algorithm>

class Clock::Timer {
public:
	Timer();
	~Timer();
	void start();
	void getTime(unsigned int &hours, unsigned int &minutes);
	unsigned int callback();

private:
	unsigned int update();

	SDL_TimerID timerID;
	struct Timestamp { unsigned char hours, minutes; };
	std::atomic<Timestamp> timestamp;
};

static std::weak_ptr<Clock::Timer> globalTimer;

/**
 * Gets the global timer instance, or create it if it doesn't exist already.
 * This function is not thread safe: only one thread at a time may call it.
 */
static std::shared_ptr<Clock::Timer> globalTimerInstance()
{
	std::shared_ptr<Clock::Timer> timer = globalTimer.lock();
	if (timer) {
		return timer;
	} else {
		// Note: Separate start method is necessary because globalTimer must
		//       be written before callbacks can occur.
		timer.reset(new Clock::Timer());
		globalTimer = timer;
		timer->start();
		return timer;
	}
}

extern "C" Uint32 callbackFunc(Uint32 /*timeout*/, void */*d*/)
{
	std::shared_ptr<Clock::Timer> timer = globalTimer.lock();
	return timer ? timer->callback() : 0;
}

Clock::Timer::Timer()
	: timerID(NULL) { }

Clock::Timer::~Timer()
{
	if (timerID) {
		SDL_RemoveTimer(timerID);
	}
}

void Clock::Timer::start()
{
	if (timerID) {
		ERROR("SDL timer was already started\n");
		return;
	}
	unsigned int ms = update();
	timerID = SDL_AddTimer(ms, callbackFunc, this);
	if (!timerID) {
		ERROR("Could not initialize SDL timer: %s\n", SDL_GetError());
	}
}

void Clock::Timer::getTime(unsigned int &hours, unsigned int &minutes)
{
	struct Timestamp ts = timestamp.load();
	hours = ts.hours;
	minutes = ts.minutes;
}

unsigned int Clock::Timer::update()
{
	using namespace std::chrono;
	
	auto time = system_clock::now();
	time_t tt = system_clock::to_time_t(time);
	tm local = *localtime(&tt);
	
	timestamp.store({
		static_cast<unsigned char>(local.tm_hour),
		static_cast<unsigned char>(local.tm_min)
		});
	DEBUG("Time updated: %02i:%02i:%02i\n",
		local.tm_hour, local.tm_min, local.tm_sec);

	// Compute number of milliseconds to next minute boundary.
	// We don't need high precision, but it is important that any deviation is
	// past the minute mark, so the fetched hour and minute number belong to
	// the freshly started minute.
	// TODO: Does the SDL timer in fact guarantee we're never called early?
	//       "ms = t->interval - SDL_TIMESLICE;" worries me.
	// Clamping it at 1 sec both avoids overloading the system in case our
	// computation goes haywire and avoids passing 0 to SDL, which would stop
	// the recurring timer.
	return std::max(1, (60 - local.tm_sec)) * 1000;
}

unsigned int Clock::Timer::callback()
{
	unsigned int ms = update();

	request_repaint();

	// TODO: SDL timer forgets adjusted interval if a timer was inserted or
	//       removed during the callback. So we should either fix that bug
	//       in SDL or ensure we don't insert/remove timers at runtime.
	//       The blanking timer is inserted/removed quite a lot at time moment,
	//       but it could be reprogrammed to adjust the interval instead.
	return ms;
}

std::tuple<unsigned int, unsigned int> Clock::getCurrentTime()
{
	unsigned int hours, minutes;
	timer->getTime(hours, minutes);

	return std::make_tuple(hours, minutes);
}

std::string Clock::getTime(bool is24)
{
	unsigned int hours, minutes;

	std::tie(hours, minutes) = getCurrentTime();


    if (hours >= 24) {
        hours -= 24; // Adjust hours to valid 24-hour range
    }

	bool pm = hours >= 12;
	if (!is24 && pm)
		hours -= 12;

	char buf[9];
	sprintf(buf, "%02i:%02i%s", hours, minutes, is24 ? "" : (pm ? "pm" : "am"));
	return std::string(buf);
}

Clock::Clock()
	: timer(globalTimerInstance())
{
}
