#include "opengame_core.hpp"

#include <cassert>

int main() {
    opengame::Core core;

    assert(core.state() == opengame::SessionState::stopped);
    assert(core.version() == "0.1.0");
    assert(core.initialize());
    assert(core.state() == opengame::SessionState::initialized);
    assert(!core.initialize());
    assert(core.start(opengame::EngineKind::pc));
    assert(core.state() == opengame::SessionState::running);

    core.stop();
    assert(core.state() == opengame::SessionState::stopped);
    return 0;
}
