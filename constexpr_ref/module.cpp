#include "constexpr_ref.h"

void print_in_module() {
    std::cout << "From module:" << std::endl;
    std::cout << "  a: " << n::a << "  b: " << b << std::endl;
}
