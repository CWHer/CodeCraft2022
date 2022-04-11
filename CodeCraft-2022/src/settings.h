#ifndef __SETTINGS_H__
#define __SETTINGS_H__

#include "common.h"

namespace Settings
{
    // >>> minimize maximum local search
    const i32 init_delta = 1000000;
    const f32 init_delta_factor = 0.8;

    const i32 min_delta = 100;
    const f32 min_delta_factor = 0.001;

    const f32 step_factor = 1.1;
    const f32 discounting_factor = 0.9;

    const f32 max_epsilon = 0.9;
    const f32 min_epsilon = 0.01;

    const i32 fail_threshold = 20;

    const f32 init_proportion = 0.1;

    // <<< minimize maximum local search

    // Solutions.evalute()
    const f32 quantile = 0.95;

};

#endif