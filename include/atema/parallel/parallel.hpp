#ifndef ATEMA_PARALLEL_PARALLEL
#define ATEMA_PARALLEL_PARALLEL


#include <atema/context/opengl.hpp>

#define CL_HPP_TARGET_OPENCL_VERSION 200
#include <CL/cl2.hpp>

#include <string>



namespace at {

    template<class Par>
    class Parallel {

        Par p;


    public:

        void add_file(std::string const& s) {
            p.add_src(s+"_loaded");
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
            setArgs<0>(std::forward<Ts>(ts)...);
            run();
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

        std::string loadFile(std::string const& file) {
            return file + "_loaded";
        }

    };

}

#endif
