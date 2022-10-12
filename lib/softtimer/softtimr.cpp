#include "softtimer.h"

void _universalSoftTimerCallback(TimerHandle_t handle)
{
    if (!handle)
    {
        return;
    }

    uint32_t id = (uint32_t)pvTimerGetTimerID(handle);

    auto it = _timerContainer.find(id);
    if (it == _timerContainer.end())
    {
        return;
    }

    if (it->second->fn)
    {
        it->second->fn();
    }

    if (uxTimerGetReloadMode(handle) == pdFALSE)
    {
        delete it->second;

        xTimerDelete(handle, 0);

        _timerContainer.erase(it);
    }
}

bool suspendAllTimers(void)
{
    _allowNewTimer = 0;
    auto it = _timerContainer.begin();
    auto end = _timerContainer.end();

    int count = 0;

    for (; it != end; ++it)
    {
        if (xTimerStop(it->second->handle, 0) == pdPASS)
        {
            count++;
        }
    }

    return count >= _timerContainer.size();
}

uint32_t _createTimer(std::function<void(void)> fn, int32_t type, uint32_t timeout)
{
    if (!_allowNewTimer)
    {
        return 0;
    }

    SoftTimerT *timer = new SoftTimerT();

    timer->fn = fn;

    timer->handle = xTimerCreate("nb",
                                 (timeout / portTICK_PERIOD_MS),
                                 (type ? pdTRUE : pdFALSE),
                                 (void *)_universalSoftTimerID,
                                 _universalSoftTimerCallback);

    if (!timer->handle)
    {
        return 0;
    }

    if (xTimerStart(timer->handle, 0) != pdPASS)
    {
        return 0;
    }

    timer->id = _universalSoftTimerID;

    _universalSoftTimerID++;

    _timerContainer.insert(
        {timer->id,
         timer});

    return timer->id;
}

uint32_t setTimeout(std::function<void(void)> fn, uint32_t timeout)
{
    return _createTimer(fn, 0, timeout);
}

uint32_t setInterval(std::function<void(void)> fn, uint32_t interval)
{
    return _createTimer(fn, 1, interval);
}

bool _clearTimer(uint32_t id)
{
    auto it = _timerContainer.find(id);

    if (it != _timerContainer.end())
    {
        delete it->second;
        xTimerDelete(it->second->handle, 0);
        _timerContainer.erase(it);
        std::map<uint32_t, SoftTimerT *>().swap(_timerContainer);
        return true;
    }
    return false;
}

bool clearTimeout(uint32_t id)
{
    return _clearTimer(id);
}

bool clearInterval(uint32_t id)
{
    return _clearTimer(id);
}

uint8_t _allowNewTimer = 0xffu;

uint32_t _universalSoftTimerID = 1;

std::map<uint32_t, SoftTimerT *> _timerContainer;