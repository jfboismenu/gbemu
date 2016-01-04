#pragma once

namespace gbemu {

template<typename Derived>
class CyclicBase
{
public:
    CyclicBase(int count);
    // Decrements the counter by an arbitrary amount. Returns how many times the
    // counter underflowed.
    int decrement(int ticks = 1);
    // Increments the counter by an arbitrary amount. Returns how many times the
    // counter overflowed.
    int increment(int ticks = 1);
    Derived& operator++();
    Derived operator+(int i) const;
    Derived operator-(int i) const;
    bool operator!=(const Derived&) const;
    bool operator==(const Derived&) const;
    int count() const;
    operator int() const;
    // Sets back to 0.
    void reset();
private:
    Derived& derived();
    const Derived& derived() const;
    int derivedGetCycleLength() const;
    int _count;
};

template<int CycleLength>
class CyclicCounterT;

template<>
class CyclicCounterT<0> : public CyclicBase<CyclicCounterT<0>>
{
public:
    CyclicCounterT(int count, int length);
    int getCycleLength() const;
private:
    int _length;
};

template<int CycleLength>
class CyclicCounterT : public CyclicBase<CyclicCounterT<CycleLength>>
{
public:
    using CyclicBase<CyclicCounterT<CycleLength>>::CyclicBase;
    int getCycleLength() const;
};

using CyclicCounter = CyclicCounterT<0>;

}
