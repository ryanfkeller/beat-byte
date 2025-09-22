#pragma once

struct Gui {
    void (*init)(void);
};

extern const struct Gui gui;