#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "clkgen.h"

void time_handler1(clkgen_t timer_id, void * user_data)
{
    printf("Periodic timer 0x%lx: clk %d\n", timer_id, *(int *) user_data);
    *(int *) user_data = !(*(int *) user_data);
}

int main(int argc, char *argv[])
{
    static int clk = 0;
    clkgen_t clkgen = create_clk(1000000000, time_handler1, &clk);
    sleep(5);
    destroy_clk(clkgen);
    return 0;
}
