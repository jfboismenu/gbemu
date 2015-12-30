#pragma once

#include <base/cyclicCounter.h>
#include <iostream> 

namespace gbemu {

template<int CycleLength>
CyclicCounter<CycleLength>::CyclicCounter(const int count) :
    _count(count % CycleLength)
{}

template<int CycleLength>
CyclicCounter<CycleLength>& CyclicCounter<CycleLength>::operator++()
{
    _count = (_count + 1) % CycleLength;
    return *this;
}

template<int CycleLength>
CyclicCounter<CycleLength> CyclicCounter<CycleLength>::operator+(int i) const
{
    CyclicCounter<CycleLength> counter(*this);
    ++counter;
    return counter;
}

template<int CycleLength>
CyclicCounter<CycleLength> CyclicCounter<CycleLength>::operator-(int i) const
{
    CyclicCounter<CycleLength> counter(*this);
    --counter._count;
    if (counter._count < 0) {
        counter._count += CycleLength;
    }
    return counter;
}

template<int CycleLength>
bool CyclicCounter<CycleLength>::operator!=(const CyclicCounter& that) const
{
    return _count != that._count;
}

template<int CycleLength>
bool CyclicCounter<CycleLength>::operator==(const CyclicCounter& that) const
{
    return _count == that._count;
}

template<int CycleLength>
CyclicCounter<CycleLength>::operator int() const
{
    return _count;
}

}
