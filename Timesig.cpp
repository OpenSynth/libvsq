/**
 * Timesig.cpp
 * Copyright © 2012 kbinani
 *
 * This file is part of libvsq.
 *
 * libvsq is free software; you can redistribute it and/or
 * modify it under the terms of the BSD License.
 *
 * libvsq is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */
#include <sstream>
#include <iostream>
#include "Timesig.hpp"

using namespace std;
using namespace VSQ_NS;

Timesig::Timesig( int numerator, int denominator, int barCount )
{
    this->clock = 0;
    this->numerator = numerator;
    this->denominator = denominator;
    this->barCount = barCount;
}

Timesig::Timesig()
{
    this->clock = 0;
    this->numerator = 4;
    this->denominator = 4;
    this->barCount = 0;
}

const string Timesig::toString() const
{
    ostringstream oss;
    oss << "{Clock=" << this->clock << ", Numerator=" << this->numerator << ", Denominator=" << this->denominator << ", BarCount=" << this->barCount << "}";
    return oss.str();
}

int Timesig::compareTo( Timesig &item ) const
{
    return this->barCount - item.barCount;
}

int Timesig::compare( const void *a, const void *b )
{
    Timesig *castedA = *(Timesig **)a;
    Timesig *castedB = *(Timesig **)b;
    return castedA->compareTo( *castedB );
}
