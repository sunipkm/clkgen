#ifndef __CLKGEN_H
#define __CLKGEN_H
#include <stdio.h>
#include "timer_gen.h"
// clock data type
typedef size_t clkgen_t;

// clock create function
clkgen_t create_clk(unsigned long long int interval_nsec, time_handler handler, void *data);

// clock destroy function
void destroy_clk(clkgen_t clkgen);

#endif // __CLKGEN_H