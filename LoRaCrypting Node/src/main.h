#ifndef MAIN
#define MAIN

#define BAND 868E6

enum device_states {
    Measure,
    checkTimes,
    Send,
    startTimer,
    Read,
    Sleep,
    Error
};

void setup();
void loop();

#endif