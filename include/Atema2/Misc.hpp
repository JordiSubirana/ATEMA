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

#ifndef ATEMA_MISC_HEADER
#define ATEMA_MISC_HEADER

#define ATEMA_STRINGIFY(at_object) #at_object

#define ATEMA_PI 3.14159265358979323846

#define ATEMA_DEG_TO_RAD(at_degrees) ((at_degrees)*ATEMA_PI/180.0)
#define ATEMA_RAD_TO_DEG(at_radians) ((at_radians)*180.0/ATEMA_PI)

#endif