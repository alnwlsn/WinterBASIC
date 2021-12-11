#include <arduino.h>
class Stopwatch {  //emulates a stopwatch (start, stop, reset) using millis()
    bool running;
    unsigned long offset;
    unsigned long val;

   public:
    Stopwatch() {
        running = 0;
        offset = millis();
        val = 0;
    }
    void start() {
        running = 1;
    }
    void stop() {
        running = 0;
    }
    void set(uint32_t v) {
        val = v;
        offset = millis() - val;
    }
    void reset() {
        set(0);
    }
    bool isRunning() {
        return running;
    }
    uint32_t value() {
        uint32_t c = millis();
        if (!running) {
            offset = c - val;
        }
        val = c - offset;
        return val;
    }
};