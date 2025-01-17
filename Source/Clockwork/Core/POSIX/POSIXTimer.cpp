// Copyright (c) 2016 Clockwork Engine. All Rights Reserved.
// See License.txt in the project root for license information.

#include "../Timer.h"
#include "../CoreEvents.h"
#include "../Profiler.h"

#include <ctime>
#include <sys/time.h>
#include <unistd.h>

#include "../../DebugNew.h"

namespace Urho3D
{

bool HiresTimer::supported(false);
long long HiresTimer::frequency(1000);

Time::Time(Context* context) :
    Object(context),
    frameNumber_(0),
    timeStep_(0.0f)
{
    HiresTimer::frequency = 1000000;
    HiresTimer::supported = true;
}

Time::~Time()
{
}

static unsigned Tick()
{
    struct timeval time;
    gettimeofday(&time, NULL);
    return (unsigned)(time.tv_sec * 1000 + time.tv_usec / 1000);
}

static long long HiresTick()
{
    struct timeval time;
    gettimeofday(&time, NULL);
    return time.tv_sec * 1000000LL + time.tv_usec;
}

void Time::BeginFrame(float timeStep)
{
    ++frameNumber_;
    if (!frameNumber_)
        ++frameNumber_;

    timeStep_ = timeStep;

    Profiler* profiler = GetSubsystem<Profiler>();
    if (profiler)
        profiler->BeginFrame();

    {
        URHO3D_PROFILE(BeginFrame);

        // Frame begin event
        using namespace BeginFrame;

        VariantMap& eventData = GetEventDataMap();
        eventData[P_FRAMENUMBER] = frameNumber_;
        eventData[P_TIMESTEP] = timeStep_;
        SendEvent(E_BEGINFRAME, eventData);
    }
}

void Time::EndFrame()
{
    {
        URHO3D_PROFILE(EndFrame);

        // Frame end event
        SendEvent(E_ENDFRAME);
    }

    Profiler* profiler = GetSubsystem<Profiler>();
    if (profiler)
        profiler->EndFrame();
}

float Time::GetElapsedTime()
{
    return elapsedTime_.GetMSec(false) / 1000.0f;
}

unsigned Time::GetSystemTime()
{
    return Tick();
}

unsigned Time::GetTimeSinceEpoch()
{
    return (unsigned)time(NULL);
}

String Time::GetTimeStamp()
{
    time_t sysTime;
    time(&sysTime);
    const char* dateTime = ctime(&sysTime);
    return String(dateTime).Replaced("\n", "");
}

void Time::Sleep(unsigned mSec)
{
    usleep(mSec * 1000);
}

Timer::Timer()
{
    Reset();
}

unsigned Timer::GetMSec(bool reset)
{
    unsigned currentTime = Tick();
    unsigned elapsedTime = currentTime - startTime_;
    if (reset)
        startTime_ = currentTime;

    return elapsedTime;
}

void Timer::Reset()
{
    startTime_ = Tick();
}

HiresTimer::HiresTimer()
{
    Reset();
}

long long HiresTimer::GetUSec(bool reset)
{
    long long currentTime = HiresTick();
    long long elapsedTime = currentTime - startTime_;

    if (elapsedTime < 0)
        elapsedTime = 0;

    if (reset)
        startTime_ = currentTime;

    return (elapsedTime * 1000000LL) / frequency;
}

void HiresTimer::Reset()
{
    startTime_ = HiresTick();
}

}
