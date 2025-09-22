#pragma once

struct Uart {
    void (*init)(void);
};

extern const struct Uart uart;
