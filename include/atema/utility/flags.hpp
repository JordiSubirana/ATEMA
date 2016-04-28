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

#ifndef ATEMA_UTILITY_ENUM_FLAGS_HEADER
#define ATEMA_UTILITY_ENUM_FLAGS_HEADER

namespace at
{
	using Flags = int;
}

//Unary functions
#define ATEMA_OPERATOR_FUNCTIONS_1(enum_name, symbol)								\
			inline at::Flags operator symbol(enum_name e1)							\
			{																		\
				return static_cast<at::Flags>(symbol static_cast<at::Flags>(e1));	\
			}

//Binary functions
#define ATEMA_OPERATOR_FUNCTION_REF_2(enum_name, symbol)														\
			inline at::Flags& operator symbol##=(at::Flags& e1, enum_name e2)									\
			{																									\
				e1 = e1 symbol static_cast<at::Flags>(e2);														\
				return e1;																						\
			}

#define ATEMA_OPERATOR_FUNCTION_2(type1, type2, symbol)															\
			inline at::Flags operator symbol(type1 e1, type2 e2)												\
			{																									\
				return static_cast<at::Flags>(static_cast<at::Flags>(e1) symbol static_cast<at::Flags>(e2));	\
			}

#define ATEMA_OPERATOR_FUNCTIONS_2(enum_name, symbol)					\
			ATEMA_OPERATOR_FUNCTION_2(enum_name, enum_name, symbol)		\
			ATEMA_OPERATOR_FUNCTION_2(at::Flags, enum_name, symbol)		\
			ATEMA_OPERATOR_FUNCTION_2(enum_name, at::Flags, symbol)		\

//Build all functions
#define ATEMA_ENUM_FLAGS(enum_name)								\
			ATEMA_OPERATOR_FUNCTIONS_2(enum_name, &)			\
			ATEMA_OPERATOR_FUNCTIONS_2(enum_name, |)			\
			ATEMA_OPERATOR_FUNCTIONS_2(enum_name, ^)			\
			ATEMA_OPERATOR_FUNCTIONS_1(enum_name, ~)			\
			ATEMA_OPERATOR_FUNCTION_REF_2(enum_name, &)			\
			ATEMA_OPERATOR_FUNCTION_REF_2(enum_name, |)			\
			ATEMA_OPERATOR_FUNCTION_REF_2(enum_name, ^)

#endif