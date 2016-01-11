#pragma once

namespace gbemu {

    class Counter
    {
    public:
        Counter(int count = 0, int cycleLength = 1);
        bool increment();
        int count() const;
        void reset();
        int getCycleLength() const;
    private:
        int _count;
        int _cycleLength;
    };

}
