#pragma once

#include <base/cyclicCounter.h>
#include <common/common.h>
#include <iostream>

namespace gbemu {

template<typename Derived>
CyclicBase<Derived>::CyclicBase(const int count) :
    _count(count)
{}

template<typename Derived>
int CyclicBase<Derived>::decrement(int ticks)
{
    int nbTimesUnderflowed = ticks / derivedGetCycleLength();
    int remainder = ticks % derivedGetCycleLength();
    _count -= remainder;
    if ( _count < 0 ) {
        ++nbTimesUnderflowed;
        _count += derivedGetCycleLength();
    }
    return nbTimesUnderflowed;
}

template<typename Derived>
int CyclicBase<Derived>::increment(int ticks)
{
    int nbTimesOverflowed = ticks / derivedGetCycleLength();
    int remainder = ticks % derivedGetCycleLength();
    _count += remainder;
    if ( _count >= derivedGetCycleLength() ) {
        ++nbTimesOverflowed;
        _count %= derivedGetCycleLength();
    }
    return nbTimesOverflowed;
}

template<typename Derived>
Derived& CyclicBase<Derived>::operator++()
{
    _count = (_count + 1) % derivedGetCycleLength();
    return derived();
}

template<typename Derived>
Derived CyclicBase<Derived>::operator+(int i) const
{
    Derived counter(derived());
    ++counter;
    return counter;
}

template<typename Derived>
Derived CyclicBase<Derived>::operator-(int i) const
{
    Derived counter(derived());
    --counter._count;
    if (counter._count < 0) {
        counter._count += derivedGetCycleLength();
    }
    return counter;
}

template<typename Derived>
bool CyclicBase<Derived>::operator!=(const Derived& that) const
{
    return _count != that._count;
}

template<typename Derived>
bool CyclicBase<Derived>::operator==(const Derived& that) const
{
    return _count == that._count;
}

template<typename Derived>
CyclicBase<Derived>::operator int() const
{
    return count();
}

template<typename Derived>
int CyclicBase<Derived>::count() const
{
    return _count;
}

template<typename Derived>
void CyclicBase<Derived>::reset()
{
    _count = 0;
}

template<typename Derived>
Derived& CyclicBase<Derived>::derived()
{
    return static_cast<Derived&>(*this);
}

template<typename Derived>
const Derived& CyclicBase<Derived>::derived() const
{
    return static_cast<const Derived&>(*this);
}

template<typename Derived>
int CyclicBase<Derived>::derivedGetCycleLength() const
{
    return derived().getCycleLength();
}

// CyclicCounterT<0>

JFX_INLINE CyclicCounterT<0>::CyclicCounterT(
    int count,
    int length
) : CyclicBase<CyclicCounterT<0>>(count),
    _length(length)
{}

JFX_INLINE int CyclicCounterT<0>::getCycleLength() const
{
    return _length;
}


// CyclicCounterT<any int but 0>

template<int CycleLength>
int CyclicCounterT<CycleLength>::getCycleLength() const
{
    return CycleLength;
}

}
