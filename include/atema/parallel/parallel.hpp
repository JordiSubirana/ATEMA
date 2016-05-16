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


#ifndef ATEMA_PARALLEL_PARALLEL
#define ATEMA_PARALLEL_PARALLEL

#include <atema/core/error.hpp>



#include <fstream>
#include <sstream>
#include <string>



namespace at {

    struct ComputeSize {
        ComputeSize(size_t a = 1, size_t b = 1, size_t c = 1) : x(a), y(b), z(c) {}
        size_t x, y, z;
    };

    template<class Par>
    class Parallel {

        Par p;


    public:
        template <typename... Ts>
        Parallel<Par>(Ts&&... ts) : p(std::forward(ts)...) {}

        void add_file(std::string const& filename) {
            std::string str, buf;
            std::ifstream ifs(filename, std::ifstream::in);

            if ( !ifs.is_open() ) {
                std::ostringstream os;
                os << "ERROR " << __FILE__ << ":" << __LINE__ << ": "<< "file not found: " << filename;
                throw std::runtime_error(os.str());
            }


            while (!getline(ifs, buf).eof()) {
                str += buf;
                str += "\n";
            }

            str += buf;

            p.add_src(str);
        }

        template<typename... Files>
        void add_file(std::string const& s, Files&&... files) {
            add_file(s);
            add_file(std::forward<Files>(files)...);
        }


        void add_src(std::string const& s) {
            p.add_src(s);
        }

        template<typename... Srcs>
        void add_src(std::string const& s, Srcs&&... srcs) {
            add_src(s);
            add_src(std::forward<Srcs>(srcs)...);
        }

        void build() {
            p.build();
        }

        void setRange(ComputeSize groupCount, ComputeSize groupSize) {
            p.setRange(groupCount, groupSize);
        }

        template<typename T>
        void setArg(unsigned i, T arg) {
            p.setArg(i, arg);
        }

        void run() {
            p.run();
        }

        // functor
        template<typename... Ts>
        void operator() (Ts... ts) {
            p.prerun();
            setArgs<0>(std::forward<Ts>(ts)...);
            run();
        }

        Par& get() noexcept {
            return p;
        }


    private:

        template<unsigned index, typename T0, typename... T1s>
        void setArgs(T0&& t0, T1s&&... t1s)
        {
            setArgs<index>(t0);
            setArgs<index + 1, T1s...>(std::forward<T1s>(t1s)...);
        }

        template<unsigned index, typename T0>
        void setArgs(T0&& t0)
        {
            setArg(index, t0);
        }

        template<unsigned index>
        void setArgs()
        {
        }


    };

}

#endif
