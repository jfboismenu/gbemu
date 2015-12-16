#pragma once

namespace gbemu {

template<int CycleLength>
class CyclicCounter
{
public:
    CyclicCounter(int count);
    CyclicCounter& operator++();
    CyclicCounter operator+(int i) const;
    CyclicCounter operator-(int i) const;
    bool operator!=(const CyclicCounter&) const;
    bool operator==(const CyclicCounter&) const;
    operator int() const;
private:
    int _count;
};

}
