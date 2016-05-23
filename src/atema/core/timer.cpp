// ----------------------------------------------------------------------
// Copyright (C) 2016 Jordi SUBIRANA
//
// This file is part of ATEMA.
//
// ATEMA is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// ATEMA is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with ATEMA.  If not, see <http://www.gnu.org/licenses/>.
// ----------------------------------------------------------------------


#include <atema/core/timer.hpp>
#include <thread>


namespace at {

    static const Timer from_start;
    static thread_local Timer chrono;

    Timer::Timer() noexcept {
        reset();
    }

    void Timer::reset() noexcept {
        t0 = std::chrono::high_resolution_clock::now();
    }

    Duration Timer::elapsed() const {
        auto t1 =  std::chrono::high_resolution_clock::now();

        std::chrono::duration<unsigned long long, std::micro> time_span = std::chrono::duration_cast<std::chrono::duration<unsigned long long, std::micro>>(t1-t0);

        return usecond<unsigned long long>(time_span.count());
    }


    Duration Timer::elapsed_from_start() {
        return from_start.elapsed();
    }

    void tic() noexcept {
        chrono.reset();
    }

    float toc() {
        return chrono.elapsed().s();
    }
}