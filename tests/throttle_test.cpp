#include <cassert>
#include <chrono>
#include <thread>
#include <functional>

template <typename Callback>
auto throttle(std::chrono::milliseconds interval, Callback callback) {
    auto last = std::chrono::steady_clock::now() - interval;
    return [=]() mutable {
        auto now = std::chrono::steady_clock::now();
        if (now - last < interval) {
            return;
        }
        last = now;
        callback();
    };
}

int main() {
    int counter = 0;
    auto handler = throttle(std::chrono::milliseconds(50), [&] { ++counter; });
    handler();
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    handler();
    assert(counter == 1);
    std::this_thread::sleep_for(std::chrono::milliseconds(60));
    handler();
    assert(counter == 2);
    return 0;
}
