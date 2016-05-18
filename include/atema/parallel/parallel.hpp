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
#include <atema/parallel/tools.hpp>



#include <fstream>
#include <sstream>
#include <string>


namespace at {


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
                ATEMA_ERROR(os.str().c_str())
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

        template<typename... Args>
        void build(Args&&... args) {
            p.build(std::forward<Args>(args)...);
        }

        void set_range(ComputeSize groupCount, ComputeSize groupSize) {
            p.set_range(groupCount, groupSize);
        }

        template<typename T>
        void set_arg(unsigned i, T&& arg) {
            p.set_arg(i, std::forward<T>(arg));
        }

        void run() {
            p.run();
        }

        // functor
        template<typename... Ts>
        void operator() (Ts&&... ts) {
            p.prerun();
            set_args<0>(std::forward<Ts>(ts)...);
            run();
        }

        Par& get() noexcept {
            return p;
        }


    private:

        template<unsigned index, typename T0, typename... T1s>
        void set_args(T0&& t0, T1s&&... t1s)
        {
            set_args<index>(std::forward<T0>(t0));
            set_args<index + 1, T1s...>(std::forward<T1s>(t1s)...);
        }

        template<unsigned index, typename T0>
        void set_args(T0&& t0)
        {
            set_arg(index, std::forward<T0>(t0));
        }

        template<unsigned index>
        void set_args()
        {
        }


    };



}

#endif
