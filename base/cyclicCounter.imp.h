#pragma once

#include <base/cyclicCounter.h>
#include <common/common.h>
#include <iostream>

namespace gbemu {

template<typename Derived>
JFX_INLINE CyclicBase<Derived>::CyclicBase(const int count) :
    _count(count)
{}

template<typename Derived>
JFX_INLINE bool CyclicBase<Derived>::decrement()
{
    if (--_count < 0) {
        _count += derivedGetCycleLength();
        return true;
    }
    return false;
}

template<typename Derived>
JFX_INLINE bool CyclicBase<Derived>::increment()
{
    if (++_count == derivedGetCycleLength()) {
        _count = 0;
        return true;
    }
    return false;
}

template<typename Derived>
JFX_INLINE Derived& CyclicBase<Derived>::operator++()
{
    increment();
    return derived();
}

template<typename Derived>
JFX_INLINE Derived CyclicBase<Derived>::operator+(int i) const
{
    Derived counter(derived());
    counter.increment();
    return counter;
}

template<typename Derived>
JFX_INLINE Derived CyclicBase<Derived>::operator-(int i) const
{
    Derived counter(derived());
    counter.decrement();
    return counter;
}

template<typename Derived>
JFX_INLINE bool CyclicBase<Derived>::operator!=(const Derived& that) const
{
    return _count != that._count;
}

template<typename Derived>
JFX_INLINE bool CyclicBase<Derived>::operator==(const Derived& that) const
{
    return _count == that._count;
}

template<typename Derived>
JFX_INLINE CyclicBase<Derived>::operator int() const
{
    return count();
}

template<typename Derived>
JFX_INLINE int CyclicBase<Derived>::count() const
{
    return _count;
}

template<typename Derived>
JFX_INLINE void CyclicBase<Derived>::reset()
{
    _count = 0;
}

template<typename Derived>
JFX_INLINE Derived& CyclicBase<Derived>::derived()
{
    return static_cast<Derived&>(*this);
}

template<typename Derived>
JFX_INLINE const Derived& CyclicBase<Derived>::derived() const
{
    return static_cast<const Derived&>(*this);
}

template<typename Derived>
JFX_INLINE int CyclicBase<Derived>::derivedGetCycleLength() const
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
JFX_INLINE int CyclicCounterT<CycleLength>::getCycleLength() const
{
    return CycleLength;
}

}
