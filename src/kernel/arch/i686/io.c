#include "io.h"

void i686_IOWait() {
    i686_outb(0x80, 0);
}