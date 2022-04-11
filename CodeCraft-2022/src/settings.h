#ifndef __SETTINGS_H__
#define __SETTINGS_H__

#include "common.h"

namespace Settings
{
    // >>> minimize maximum local search
    const i32 init_delta = 1000000;
    const f32 init_delta_factor = 0.5;

    const i32 min_delta = 100;
    const f32 min_delta_factor = 0.001;

    const f32 discounting_factor = 0.6;
    // <<< minimize maximum local search

    // Solutions.evalute()
    const f32 quantile = 0.95;

};

#endif