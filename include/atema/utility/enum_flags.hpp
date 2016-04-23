#ifndef ATEMA_UTILITY_ENUM_FLAGS_HEADER
#define ATEMA_UTILITY_ENUM_FLAGS_HEADER

namespace at
{
	using flags = int;
}

#define ATEMA_OPERATOR_FUNCTION1(enum_name, symbol)									\
			inline constexpr at::flags operator symbol(enum_name e1)				\
			{																		\
				return static_cast<at::flags>(symbol static_cast<at::flags>(e1));	\
			}

#define ATEMA_OPERATOR_FUNCTION2(enum_name, symbol)																\
			inline constexpr at::flags operator symbol(enum_name e1, enum_name e2)								\
			{																									\
				return static_cast<at::flags>(static_cast<at::flags>(e1) symbol static_cast<at::flags>(e2));	\
			}

#define ATEMA_ENUM_FLAGS(enum_name)					\
			ATEMA_OPERATOR_FUNCTION2(enum_name, &)	\
			ATEMA_OPERATOR_FUNCTION2(enum_name, |)	\
			ATEMA_OPERATOR_FUNCTION2(enum_name, ^)	\
			ATEMA_OPERATOR_FUNCTION1(enum_name, ~)

#endif