/* Copyright 2013-2018 Heiko Burau, Rene Widera
 *
 * This file is part of PMacc.
 *
 * PMacc is free software: you can redistribute it and/or modify
 * it under the terms of either the GNU General Public License or
 * the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * PMacc is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License and the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License
 * and the GNU Lesser General Public License along with PMacc.
 * If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include "pmacc/types.hpp"
#include "pmacc/math/vector/compile-time/Vector.hpp"


namespace pmacc
{

template <class TYPE>
class SuperCell
{
public:

    HDINLINE SuperCell() :
        firstFramePtr(nullptr),
        lastFramePtr(nullptr),
        numParticles(0),
        mustShiftVal(false)
    {
    }

    HDINLINE TYPE* FirstFramePtr()
    {
        return firstFramePtr;
    }

    HDINLINE TYPE* LastFramePtr()
    {
        return lastFramePtr;
    }

    HDINLINE const TYPE* FirstFramePtr() const
    {
        return firstFramePtr;
    }

    HDINLINE const TYPE* LastFramePtr() const
    {
        return lastFramePtr;
    }

    HDINLINE bool mustShift()
    {
        return mustShiftVal;
    }

    HDINLINE void setMustShift(bool value)
    {
        mustShiftVal = value;
    }

    HDINLINE uint32_t getSizeLastFrame()
    {
        constexpr uint32_t frameSize = math::CT::volume<
            typename TYPE::SuperCellSize
        >::type::value;
        uint32_t numParLastFrame = numParticles % frameSize;
        // numParLastFrame == 0 means the last frame is fully filled
        return numParLastFrame == 0 ? frameSize : numParLastFrame;
    }

    HDINLINE uint32_t getNumParticles()
    {
        return numParticles;
    }

    HDINLINE void setNumParticles(uint32_t size)
    {
        numParticles = size;
    }

public:
    PMACC_ALIGN(firstFramePtr, TYPE*);
    PMACC_ALIGN(lastFramePtr, TYPE*);
private:
    PMACC_ALIGN(numParticles, uint32_t);
    PMACC_ALIGN(mustShiftVal, bool);
};

} //namespace pmacc
