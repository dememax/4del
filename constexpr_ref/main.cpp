#include "constexpr_ref.h"

void print_in_module(); // Forward declaration

int main() {
    std::cout << "From main:" << std::endl;
    std::cout << "  a: " << n::a << "  b: " << b << std::endl;
    print_in_module();
    return 0;
}
