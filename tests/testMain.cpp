#include <base/cyclicCounter.imp.h>
#include <base/clock.imp.h>
#include <common/common.h>

using namespace gbemu;

void testClockT()
{
    ClockT<2, 0> c2(0);
    JFX_ASSERT(!c2.increment());
    JFX_ASSERT(c2.increment());
    JFX_ASSERT(!c2.increment());
    JFX_ASSERT(c2.increment());

    ClockT<8, 7> c8(0);
    JFX_ASSERT(!c8.increment());
    JFX_ASSERT(!c8.increment());
    JFX_ASSERT(!c8.increment());
    JFX_ASSERT(!c8.increment());
    JFX_ASSERT(!c8.increment());
    JFX_ASSERT(!c8.increment());
    JFX_ASSERT(c8.increment());
    JFX_ASSERT(!c8.increment());
    JFX_ASSERT(!c8.increment());
    JFX_ASSERT(!c8.increment());
    JFX_ASSERT(!c8.increment());
    JFX_ASSERT(!c8.increment());
    JFX_ASSERT(!c8.increment());
    JFX_ASSERT(!c8.increment());
    JFX_ASSERT(c8.increment());
    JFX_ASSERT(!c8.increment());

    ClockT<4, 3> c4(0);
    JFX_ASSERT(!c4.increment());
    JFX_ASSERT(!c4.increment());
    JFX_ASSERT(c4.increment());
    JFX_ASSERT(!c4.increment());
    JFX_ASSERT(!c4.increment());
    JFX_ASSERT(!c4.increment());
    JFX_ASSERT(c4.increment());
    JFX_ASSERT(!c4.increment());

    ClockT<4194304 / 512, 0> _512hzClock(0);
    JFX_CMP_ASSERT((ClockT<4194304 / 512, 0>::kLength), ==, 8192);
    for (int i = 0; i < 8191; ++i) {
        JFX_ASSERT(!_512hzClock.increment());
    }
    JFX_ASSERT(_512hzClock.increment());
}


int main(const int argc, char const * const* const argv)
{
    testClockT();

    return 0;
}
