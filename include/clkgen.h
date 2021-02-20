/**
 * @file clkgen.h
 * @author Sunip K. Mukherjee (sunipkmukherjee@gmail.com)
 * @brief Create a timer that executes the registered handler upon repeated expiration.
 * @version 1.1
 * @date 2021-02-20
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#ifndef __CLKGEN_H
#define __CLKGEN_H
#include <stdio.h>
#include "timer_gen.h"
/**
 * @brief Clock generator datatype
 * 
 */
typedef size_t clkgen_t;

#ifndef NSEC_PER_SEC
#define NSEC_PER_SEC 1000000000L
#endif

#ifndef NSEC_PER_MSEC
#define NSEC_PER_MSEC 1000000L
#endif

#ifndef NSEC_PER_USEC
#define NSEC_PER_USEC 1000L
#endif

/**
 * @brief Create a clock generator instance
 * 
 * @param interval_nsec Clock interval in nanoseconds
 * @param handler Registered handler function
 * @param data Any data to be passed to the handler function
 * @return clkgen_t Instance ID of clock (Note: on macOS this is the total number of timers registered up to invocation)
 */
clkgen_t create_clk(unsigned long long int interval_nsec, time_handler handler, void *data);

/**
 * @brief Stop the clock generator instance
 * 
 * @param clkgen Clock ID to be disabled
 */
void destroy_clk(clkgen_t clkgen);

#endif // __CLKGEN_H