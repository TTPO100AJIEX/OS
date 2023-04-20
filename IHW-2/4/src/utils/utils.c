#include <time.h>

void sleep_milliseconds(unsigned int milliseconds)
{
    struct timespec interval = { milliseconds / 1000, (milliseconds % 1000) * 1000000 };
    nanosleep(&interval, NULL);
}