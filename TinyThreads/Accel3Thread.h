#ifndef ACCEL3_THREAD_H
#define ACCEL3_THREAD_H

#include "Thread.h"

namespace tinythreads {

typedef class Accel3Thread : Thread {
public:
    Accel3Thread();
    void setup();
    void loop();

protected:
} Accel3Thread;

} // namespace tinythreads

#endif
