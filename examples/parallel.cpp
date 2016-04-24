
# define CL_HPP_TARGET_OPENCL_VERSION 200
#include <CL/cl2.hpp>

#include <iostream>

using namespace std;




void scan_machine() {

    //get all platforms (drivers)
    std::vector<cl::Platform> platforms;
    std::vector<cl::Device> devices;

    cl::Platform::get(&platforms);

    printf("Found %d platform(s).\n", (int)platforms.size());

    if(platforms.size() == 0){
        std::cout<<"No platforms found. Check OpenCL installation!\n";
        return ;
    }
    for (unsigned p=0 ; p<platforms.size() ; p++) {
        std::cout << "Platform["<< p <<"]: "<< platforms[p].getInfo<CL_PLATFORM_NAME>() << std::endl;

        platforms[p].getDevices(CL_DEVICE_TYPE_ALL, &devices);

        std::cout << "Found " << devices.size() << " device(s) on platform " << p << std::endl;

        if(devices.size()==0){
            std::cout<<"    No devices found.\n";
        }
        for (unsigned d=0 ; d<devices.size() ; d++) {
            std::cout << "    Found device ["<< d << "]: "<<devices[d].getInfo<CL_DEVICE_NAME>()<<"\n";
        }
    }
    std::cout << "\n";
}




int main() {
    scan_machine();

    return 0;
}



