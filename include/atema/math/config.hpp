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

#ifndef ATEMA_MATH_CONFIG
#define ATEMA_MATH_CONFIG

//-----

#include <atema/config.hpp>

#if defined(ATEMA_STATIC)
	
	#define ATEMA_MATH_API
	
#else
	
	#if defined(ATEMA_MATH_EXPORT)
		
		#define ATEMA_MATH_API ATEMA_EXPORT
		
	#else
		
		#define ATEMA_MATH_API ATEMA_IMPORT
		
	#endif

#endif

//-----
/*
#define ATEMA_VECTOR_OPERATOR_UNARY(SYMBOL, INDEX)	\
	static_cast<T*>(this)[INDEX] SYMBOL static_cast<T*>(arg)[INDEX];
#define ATEMA_VECTOR_OPERATOR_UNARY1(SYMBOL)	\
	ATEMA_VECTOR_OPERATOR_UNARY(SYMBOL, 1)
#define ATEMA_VECTOR_OPERATOR_UNARY2(SYMBOL)	\
	ATEMA_VECTOR_OPERATOR_UNARY1(SYMBOL)		\
	ATEMA_VECTOR_OPERATOR_UNARY(SYMBOL, 2)
#define ATEMA_VECTOR_OPERATOR_UNARY3(SYMBOL)	\
	ATEMA_VECTOR_OPERATOR_UNARY2(SYMBOL)		\
	ATEMA_VECTOR_OPERATOR_UNARY(SYMBOL, 3)
#define ATEMA_VECTOR_OPERATOR_UNARY4(SYMBOL)	\
	ATEMA_VECTOR_OPERATOR_UNARY3(SYMBOL)		\
	ATEMA_VECTOR_OPERATOR_UNARY(SYMBOL, 4)
#define ATEMA_VECTOR_OPERATOR_UNARY(SYMBOL, TYPE)	\
	void operator ## SYMBOL (TYPE arg)				\
	{												\
		ATEMA_VECTOR_OPERATOR_UNARY ## DIM(SYMBOL)	\
	}

#define ATEMA_VECTOR_OPERATOR_BINARY(SYMBOL, INDEX)	\
	static_cast<T*>(this)[INDEX] SYMBOL static_cast<T*>(arg)[INDEX];
#define ATEMA_VECTOR_OPERATOR_BINARY1(SYMBOL)	\
	ATEMA_VECTOR_OPERATOR_BINARY(SYMBOL, 1)
#define ATEMA_VECTOR_OPERATOR_BINARY2(SYMBOL)	\
	ATEMA_VECTOR_OPERATOR_BINARY1(SYMBOL)		\
	ATEMA_VECTOR_OPERATOR_BINARY(SYMBOL, 2)
#define ATEMA_VECTOR_OPERATOR_BINARY3(SYMBOL)	\
	ATEMA_VECTOR_OPERATOR_BINARY2(SYMBOL)		\
	ATEMA_VECTOR_OPERATOR_BINARY(SYMBOL, 3)
#define ATEMA_VECTOR_OPERATOR_BINARY4(SYMBOL)	\
	ATEMA_VECTOR_OPERATOR_BINARY3(SYMBOL)		\
	ATEMA_VECTOR_OPERATOR_BINARY(SYMBOL, 4)
#define ATEMA_VECTOR_OPERATOR_BINARY(SYMBOL, TYPE)	\
	void operator ## SYMBOL (TYPE arg)				\
	{												\
													\
		ATEMA_VECTOR_OPERATOR_BINARY ## DIM(SYMBOL)	\
	}
	

#define ATEMA_NEXT_VAR ,

#define ATEMA_VECTOR_FIELD(VARS)	\
	struct { T VARS; };

#define ATEMA_DECLARE_VECTOR(DIM, FIELDS)	\
	template <typename T>					\
	class Vector ## DIM						\
	{										\
		public:								\
			union							\
			{								\
				FIELDS						\
			};								\
	};
//*/

/*
//USE:
ATEMA_DECLARE_VECTOR(
	4,
	ATEMA_VECTOR_FIELD( x ATEMA_NEXT_VAR y ATEMA_NEXT_VAR z ATEMA_NEXT_VAR z )
	ATEMA_VECTOR_FIELD( r ATEMA_NEXT_VAR g ATEMA_NEXT_VAR b ATEMA_NEXT_VAR a )
);
//*/
//-----

#endif