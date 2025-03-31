#include <stdio.h>
#include <unistd.h>
// #include <kernel.h>

/**
 * The ascii airplane is designed by Joan Stark
 * from: https://www.asciiart.eu/vehicles/airplanes
 */

#define CYCLE_PER_MOVE 10000
#define START_POINT 50 //0~50
#define FLY_OFF 12

static char blank[] = {"                                                                               "};
static char plane1[] = {"    \\\\   "};
static char plane2[] = {" \\====== "};
static char plane3[] = {"    //   "};

int j=0 + FLY_OFF;
volatile int cyc=0;

int main(void)
{
    while (1)
    {
        int clk = sys_get_tick();
        for(int i=START_POINT; i<60;++i){
            sys_set_sche_workload(60-i);
        
            for(int t=0;t<CYCLE_PER_MOVE;++t)cyc++;
            sys_move_cursor(i, j + 0);
            printf("%s", plane1);

            sys_move_cursor(i, j + 1);
            printf("%s", plane2);

            sys_move_cursor(i, j + 2);
            printf("%s", plane3);
            // sys_yield();
        }
        // sys_yield();
        sys_move_cursor(0, j);
        printf("%s", blank);
        sys_move_cursor(0, j + 1);
        printf("%s", blank);
        sys_move_cursor(0, j + 2);
        printf("%s", blank);

        clk = sys_get_tick() - clk;
        sys_move_cursor(0, 15+FLY_OFF);
        printf("[fly1] cycles: %d, used time per round: %d tick.",cyc, clk);
    }
}
