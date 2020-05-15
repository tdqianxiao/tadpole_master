#ifndef __TADPOLE_UTIL_H__
#define __TADPOLE_UTIL_H__

#include <unistd.h>
#include <sys/syscall.h>
#include <stdint.h>
#include <chrono>

namespace tadpole{
/**
 * @brief 获得线程id
 */
uint32_t GetThreadId();

/**
 * @brief 获得协程id
 */
uint32_t GetFiberId();

/**
 *获取程序运行到现在的时间戳
 */
uint64_t GetElapse();

}

#endif 
