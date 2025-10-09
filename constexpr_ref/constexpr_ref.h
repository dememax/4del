#pragma once
#include <iostream>

namespace n {
    constexpr int a = 60;
}
constexpr const int& b = n::a;
