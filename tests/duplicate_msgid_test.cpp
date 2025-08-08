#include <map>
#include <cassert>
#include <iostream>

int main() {
    std::map<int, int> haveSent;
    auto insert = [&](int msgId, int value) {
        auto [it, inserted] = haveSent.emplace(msgId, value);
        if (!inserted) {
            std::cerr << "duplicate " << msgId << std::endl;
            it->second = value;
        }
        return inserted;
    };

    assert(insert(1, 10));
    assert(!insert(1, 20));
    assert(haveSent.size() == 1);
    assert(haveSent[1] == 20);
    return 0;
}
