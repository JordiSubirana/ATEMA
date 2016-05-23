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


#include <atema/core/error.hpp>
#include <atema/parallel/tools.hpp>
#include <atema/graphics/texture.hpp>
#include <atema/graphics/buffer.hpp>
#include <atema/math/vector.hpp>


#include <atema/context/opengl.hpp>


#include <vector>
#include <string>


namespace at {


    class Parogl {
    protected:
        GLuint _progID, _shadID;
        std::vector<std::string> _srcs;
        std::vector<std::string> _argList;

        // group size / group count
        GLuint gsx, gsy, gsz, gcx, gcy, gcz;

        GLint _ssbo_count;
        GLint _uniform_count;

        unsigned _ssbo_i;
        unsigned _uniform_i, _texunit_i;


        void _build();



        template <typename... Ts>
        void pushName(std::string const& name, Ts&&... ts) {
            _argList.push_back(name);
            pushName(std::forward<Ts>(ts)...);
        }

        void pushName(std::string const& name) {
            _argList.push_back(name);
            if (_argList.size() != _uniform_count+_ssbo_count) {
                ATEMA_ERROR("In compute shader, number of arguments missmatch (just uniforms and ssbo are counted...)")
            }
        }


    public:

        Parogl();
        ~Parogl();

        void add_src(std::string const& s);

        void set_range(ComputeSize groupCount, ComputeSize groupSize);

        void prerun();
        void run();

        // en parametres: la liste des variables
        template <typename... Ts>
        void build(Ts... ts) {
            _build();

            _argList.clear();
            pushName(std::forward<Ts>(ts)...);
        }


        // uniform 1u (Image)
        void set_arg(unsigned i, Texture const& );

        template <typename T>
        void set_arg(unsigned i, Buffer<T> const& );
        void set_arg_buffer(unsigned i, GLuint );

        void set_arg(unsigned i, unsigned);
        void set_arg(unsigned i, int);
        void set_arg(unsigned i, float);
        void set_arg(unsigned i, double);

        void set_arg(unsigned i, Vector2u);
        void set_arg(unsigned i, Vector2i);
        void set_arg(unsigned i, Vector2f);
        void set_arg(unsigned i, Vector2d);

        void set_arg(unsigned i, Vector3u);
        void set_arg(unsigned i, Vector3i);
        void set_arg(unsigned i, Vector3f);
        void set_arg(unsigned i, Vector3d);

        void set_arg(unsigned i, Vector4u);
        void set_arg(unsigned i, Vector4i);
        void set_arg(unsigned i, Vector4f);
        void set_arg(unsigned i, Vector4d);



    };



    template <typename T>
    void Parogl::set_arg(unsigned i, Buffer<T> const& buff) {
        set_arg_buffer(i, buff.get_gl_id());
    }


}



#endif //ATEMA_PARALLEL_OCL_HPP