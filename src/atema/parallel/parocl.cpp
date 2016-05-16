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


#include <atema/parallel/parocl.hpp>

#include <vector>

#include <iostream>
#include <cstdio>

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
