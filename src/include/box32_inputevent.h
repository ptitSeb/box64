#ifndef __BOX32_INPUTEVENT_H__
#define __BOX32_INPUTEVENT_H__

#include <linux/input.h>
#include "box32.h"

// check if fd is an input event, and will register it to the list if it is
void addInputEventFD(int fd);
// return 1 if fd is registered as an input event
int isFDInputEvent(int fd);
// remove fd if it was registered as input event
void removeInputEventFD(int fd);

typedef struct my_input_event_32_s {
    ulong_t     sec;
    ulong_t     usec;
    uint16_t    type;
    uint16_t    code;
    int         value;
} my_input_event_32_t;

#endif // __BOX32_INPUTEVENT_H__