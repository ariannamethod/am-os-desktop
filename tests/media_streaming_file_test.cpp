#include <expected>
#include <cassert>

struct DummyPacket {
    int stream_index = 0;
};

struct DummyError {
    int code = 0;
};

std::expected<DummyPacket, DummyError> readPacket(bool succeed) {
    if (succeed) {
        return DummyPacket{1};
    }
    return std::unexpected(DummyError{-1});
}

int main() {
    auto good = readPacket(true);
    assert(good.has_value());
    assert(good.value().stream_index == 1);

    auto bad = readPacket(false);
    assert(!bad.has_value());
    assert(bad.error().code == -1);
    return 0;
}
