#ifndef TIMERCLOCK_H
#define TIMERCLOCK_H
#include <chrono>

class TimerClock
{
public:
     TimerClock() = default;
     ~TimerClock(){}
     void start() { startTime = std::chrono::high_resolution_clock::now();}
     double getTimerSecond() { return getTimerMicroSec() * 0.000001;}
     double getTimerMilliSec() { return getTimerMicroSec()*0.001;}
     long long getTimerMicroSec()
     {
        return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - startTime).count();
     }
private:
    std::chrono::time_point<std::chrono::high_resolution_clock> startTime;
};

#endif // TIMERCLOCK_H
