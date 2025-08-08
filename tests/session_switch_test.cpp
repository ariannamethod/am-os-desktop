#include <map>
#include <memory>
#include <cassert>

struct Account {};

struct Session {
    explicit Session(Account *a) : account(a) {}
    Account *account;
};

struct UpdateChecker {
    void setMtproto(Session *session) {
        bound = (session != nullptr) ? session->account : nullptr;
    }
    Account *bound = nullptr;
};

int main() {
    std::map<Account*, std::unique_ptr<UpdateChecker>> checkers;
    Account a1, a2;
    Session s1(&a1);
    Session s2(&a2);
    Account *active = nullptr;

    auto activate = [&](Session *session) {
        if (active) {
            checkers[active]->setMtproto(nullptr);
            active = nullptr;
        }
        if (session) {
            auto account = session->account;
            auto &checker = checkers[account];
            if (!checker) {
                checker = std::make_unique<UpdateChecker>();
            }
            checker->setMtproto(session);
            active = account;
        }
    };

    activate(&s1);
    assert(checkers[&a1]->bound == &a1);
    activate(&s2);
    assert(checkers[&a1]->bound == nullptr);
    assert(checkers[&a2]->bound == &a2);
    activate(nullptr);
    assert(checkers[&a2]->bound == nullptr);
    assert(checkers.size() == 2);
    return 0;
}
