/* Copyright 2013-2017 Rene Widera, Marco Garten
 *
 * This file is part of PMacc.
 *
 * PMacc is free software: you can redistribute it and/or modify
 * it under the terms of either the GNU General Public License or
 * the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
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

#include "types.hpp"
#include <pmacc/math/Vector.hpp>
#include <pmacc/mappings/threads/ThreadCollective.hpp>
#include <pmacc/nvidia/functors/Assign.hpp>
#include <pmacc/memory/boxes/CachedBox.hpp>
#include <pmacc/memory/dataTypes/Mask.hpp>
#include <pmacc/dimensions/DataSpaceOperations.hpp>
#include <pmacc/nvidia/rng/RNG.hpp>
#include <pmacc/nvidia/rng/methods/Xor.hpp>
#include <pmacc/nvidia/rng/distributions/Uniform_float.hpp>

namespace gol
{
    namespace kernel
    {
        using namespace pmacc;

        struct Evolution
        {
            template<class BoxReadOnly, class BoxWriteOnly, class Mapping>
            DINLINE void operator()(BoxReadOnly buffRead,
                                      BoxWriteOnly buffWrite,
                                      uint32_t rule,
                                      Mapping mapper) const
            {
                typedef typename BoxReadOnly::ValueType Type;
                typedef SuperCellDescription<
                        typename Mapping::SuperCellSize,
                        math::CT::Int< 1, 1 >,
                        math::CT::Int< 1, 1 >
                        > BlockArea;
                auto cache = CachedBox::create < 0, Type > (BlockArea());

                const Space block(mapper.getSuperCellIndex(Space(blockIdx)));
                const Space blockCell = block * Mapping::SuperCellSize::toRT();
                const Space threadIndex(threadIdx);
                auto buffRead_shifted = buffRead.shift(blockCell);

                ThreadCollective<
                    BlockArea,
                    pmacc::math::CT::volume< typename BlockArea::SuperCellSize >::type::value
                > collective(
                    DataSpaceOperations< BlockArea::Dim >::template map<
                        typename BlockArea::SuperCellSize
                    >( threadIndex )
                );

                nvidia::functors::Assign assign;
                collective(
                          assign,
                          cache,
                          buffRead_shifted
                          );
                __syncthreads();

                Type neighbors = 0;
                for (uint32_t i = 1; i < 9; ++i)
                {
                    Space offset(Mask::getRelativeDirections<DIM2 > (i));
                    neighbors += cache(threadIndex + offset);
                }

                Type isLife = cache(threadIndex);
                isLife = (bool)(((!isLife)*(1 << (neighbors + 9))) & rule) +
                        (bool)(((isLife)*(1 << (neighbors))) & rule);

                buffWrite(blockCell + threadIndex) = isLife;
            }
        };

        struct RandomInit
        {
            template<class BoxWriteOnly, class Mapping>
            DINLINE void operator()(BoxWriteOnly buffWrite,
                                       uint32_t seed,
                                       float fraction,
                                       Mapping mapper) const
            {
                /* get position in grid in units of SuperCells from blockID */
                const Space block(mapper.getSuperCellIndex(Space(blockIdx)));
                /* convert position in unit of cells */
                const Space blockCell = block * Mapping::SuperCellSize::toRT();
                /* convert CUDA dim3 to DataSpace<DIM3> */
                const Space threadIndex(threadIdx);
                const uint32_t cellIdx = DataSpaceOperations<DIM2>::map(
                        mapper.getGridSuperCells() * Mapping::SuperCellSize::toRT(),
                        blockCell + threadIndex);

                /* get uniform random number from seed  */
                auto rng = nvidia::rng::create(
                                    nvidia::rng::methods::Xor(seed, cellIdx),
                                    nvidia::rng::distributions::Uniform_float());

                /* write 1(white) if uniform random number 0<rng<1 is smaller than 'fraction' */
                buffWrite(blockCell + threadIndex) = (rng() <= fraction);
            }
        };
    }

    template<class MappingDesc>
    struct Evolution
    {
        MappingDesc mapping;
        uint32_t rule;

        Evolution(uint32_t rule) : rule(rule)
        {

        }

        void init(const MappingDesc & desc)
        {
            mapping = desc;
        }

        template<class DBox>
        void initEvolution(const DBox & writeBox, float fraction)
        {
            AreaMapping < CORE + BORDER, MappingDesc > mapper(mapping);
            GridController<DIM2>& gc = Environment<DIM2>::get().GridController();
            uint32_t seed = gc.getGlobalSize() + gc.getGlobalRank();

            PMACC_KERNEL(kernel::RandomInit{})
                    (mapper.getGridDim(), MappingDesc::SuperCellSize::toRT())
                    (
                     writeBox,
                     seed,
                     fraction,
                     mapper);
        }

        template<uint32_t Area, class DBox>
        void run(const DBox& readBox, const DBox & writeBox)
        {
            AreaMapping < Area, MappingDesc > mapper(mapping);
            PMACC_KERNEL(kernel::Evolution{})
                    (mapper.getGridDim(), MappingDesc::SuperCellSize::toRT())
                    (readBox,
                     writeBox,
                     rule,
                     mapper);
        }
    };
}


