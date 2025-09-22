#pragma once

struct SdCard {
    void (*init)(void);
};

extern const struct SdCard sd_card;