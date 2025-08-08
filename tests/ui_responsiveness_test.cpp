#include "crl/crl.h"
#include <atomic>
#include <cassert>

int main() {
    std::atomic_bool called = false;
    crl::on_main([&] { called = true; });
    assert(!called.load());
    return 0;
}
