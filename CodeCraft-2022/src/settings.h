#ifndef __SETTINGS_H__
#define __SETTINGS_H__

#include "common.h"

namespace Settings
{
    // >>> minimize maximum local search
    const i32 init_delta = 1000000;
    const i32 min_delta = 100;
    const f32 discounting_factor = 0.95;

    // <<< minimize maximum local search

};

#endif