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

#ifndef ATEMA_PARALLEL_OGL_HPP
#define ATEMA_PARALLEL_OGL_HPP


#include <atema/parallel/parallel.hpp>


#include <atema/context/opengl.hpp>


#include <vector>


namespace at {

    struct GLO_Image {
        GLuint id;
        GLO_Image(GLuint i) : id(i) {}
    };

    class Parogl {
    protected:
        GLuint _progID, _shadID;
        std::vector<std::string> _srcs;

        // group size / group count
        GLuint gsx, gsy, gsz, gcx, gcy, gcz;

        GLint _uniform_count;
        unsigned _uniform_i;
        unsigned _texunit_i;

    public:

        Parogl();

        void add_src(std::string const& s);

        void build();
        void setRange(ComputeSize groupCount, ComputeSize groupSize);

        void prerun();
        void run();

        void setArg(unsigned i, unsigned);
        void setArg(unsigned i, int);
        void setArg(unsigned i, float);

        // uniform 1u (Image)
        void setArg(unsigned i, GLO_Image);


    };

}



#endif //ATEMA_PARALLEL_OCL_HPP