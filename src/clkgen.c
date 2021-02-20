#include "timer_gen.h"
#include "clkgen.h"

#define eprintf(...) fprintf(stderr, __VA_ARGS__)

static unsigned long long int __clkgen_num_clks = 0;

clkgen_t create_clk(unsigned long long int interval_nsec, time_handler handler, void *data)
{
    if (__clkgen_num_clks == 0)
    {
        if (initialize() < 0)
        {
            eprintf("%s: Initialization failed, exiting... Err:", __func__);
            perror(" ");
            return -1;
        }
    }
    __clkgen_num_clks++;
    return start_timer(interval_nsec, handler, TIMER_PERIODIC, data);
}

void destroy_clk(clkgen_t clkgen_id)
{
    stop_timer(clkgen_id);
    __clkgen_num_clks--;
    if (__clkgen_num_clks == 0)
        finalize();
}