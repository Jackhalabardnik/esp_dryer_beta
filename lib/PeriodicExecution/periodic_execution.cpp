#include "periodic_execution.h"

namespace PeriodicExecution {
    void updateExecutions(std::vector<Routine> &routines) {
        std::for_each(routines.begin(), routines.end(), [](auto &routine) {
            if (millis() - routine.last_execution_time > routine.period || routine.last_execution_time > millis()) {
                routine.function();
                routine.last_execution_time = millis();
            }
        });
    }
}