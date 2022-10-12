#ifndef SOFT_TIMER_H_
#define SOFT_TIMER_H_

#include <map>
#include <Arduino.h>
#include <functional>

/**
 * @brief store timer handle, id and callback
 * 存储定时器句柄、id和回调函数
 */
typedef struct
{
    // id
    uint32_t id = 0;

    //句柄
    TimerHandle_t handle = NULL;

    // callback 回调函数
    std::function<void(void)> fn = nullptr;
} SoftTimerT;

/**
 * @brief universal timer callback
 * not for user execute
 *
 * 全局timer回调函数
 * 用户不要执行此函数
 */
void _universalSoftTimerCallback(TimerHandle_t);

uint32_t _createTimer(std::function<void(void)> fn, int32_t type = 0, uint32_t timeout = 1);

bool _clearTimer(uint32_t id);

/**
 * @brief create a new timer only execute once
 *
 * 创建一个一次性定时器
 *
 * @param fn callback 回调函数
 * @param timeout timeout in ms 超时时间，毫秒
 * @return uint32_t id of timer, timer的id号
 */
uint32_t setTimeout(std::function<void(void)> fn, uint32_t timeout);

/**
 * @brief stop and delete a timer
 *
 * 停止并删除一个定时器
 *
 * @param id id of timer 定时器的id
 * @return true success 成功
 * @return false failed 失败
 */
bool clearTimeout(uint32_t id);

/**
 * @brief create a timer run repeatedly
 *
 * 创建一个周期性运行的定时器
 *
 * @param fn callback 回调函数
 * @param interval period in ms 周期，毫秒
 * @return uint32_t id of timer, 定时器的id号
 */
uint32_t setInterval(std::function<void(void)> fn, uint32_t interval);

/**
 * @brief stop and delete a timer
 *
 * 停止并删除一个定时器
 *
 * @param id id of timer 定时器的id
 * @return true success 成功
 * @return false failed 失败
 */
bool clearInterval(uint32_t id);

/**
 * @brief suspend all timers
 * and NOT allow to create new timer
 *
 * 挂起所有定时器
 * 且不允许创建新的计时器
 *
 * @return true all timers suspended 所有定时器挂起成功
 * @return false NOT all timers suspended 部分定时器挂起成功
 */
bool suspendAllTimers(void);

/**
 * @brief automtic id for timers when creating
 *
 * 自动设置的id
 *
 */
extern uint32_t _universalSoftTimerID;

/**
 * @brief indicate allow create timer or not
 * 指示是否允许创建新的定时器
 *
 */
extern uint8_t _allowNewTimer;

/**
 * @brief container for timers
 *
 * 定时器的容器
 *
 */
extern std::map<uint32_t, SoftTimerT *> _timerContainer;

#endif