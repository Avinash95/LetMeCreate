/**
 * This example shows how to use the Alphanum Click wrapper of the LetMeCreate
 * to display characters.
 *
 * It displays "Ci" using both displays by enabling them one after the other at
 * 100Hz to give the illusion that both characters are displayed at the same
 * time. The user has to interrupt the program to exit it by pressing Ctrl+C.
 *
 * The Alphanum Click must be inserted in Mikrobus 1 before running this
 * program.
 */


#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <letmecreate/letmecreate.h>
#include "examples/common.h"

static volatile bool running = true;

static void exit_program(int __attribute__ ((unused))signo)
{
    running = false;
}

int main(void)
{
    /* Set signal handler to exit program when Ctrl+c is pressed */
    struct sigaction action = {
        .sa_handler = exit_program,
        .sa_flags = 0
    };
    sigemptyset(&action.sa_mask);
    sigaction (SIGINT, &action, NULL);

    printf("Press Ctrl+c to exit program.\n");

    spi_init();
    spi_select_bus(MIKROBUS_1);

    if (alphanum_click_init(MIKROBUS_1) < 0)
        return 1;

    if (alphanum_click_write('C', 'i') < 0)
        return 1;

    /* This alternately switches on the output of either the two shift registers
     * to appear to print two characters at the same time.
     */
    while (running) {
        alphanum_click_select_left_display();
        sleep_ms(5);
        alphanum_click_select_right_display();
        sleep_ms(5);
    }

    spi_release();

    return 0;
}
