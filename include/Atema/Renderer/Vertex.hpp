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

#ifndef ATEMA_RENDERER_VERTEX_HEADER
#define ATEMA_RENDERER_VERTEX_HEADER

#include <Atema/Renderer/Config.hpp>
#include <Atema/Core/Color.hpp>
#include <Atema/Math/Vector.hpp>

#include <vector>

#define ATEMA_DECLARE_VERTEX_COMPONENT_TYPE(at_type, at_comp_type) \
	namespace at \
	{ \
		template <> \
		inline VertexComponent::Type VertexComponent::get_type<at_type>() \
		{ \
			return (at_comp_type); \
		}\
	}

#define ATEMA_DECLARE_VERTEX_FORMAT(at_type, ...) \
	namespace at \
	{ \
		template <> \
		inline const VertexFormat& VertexFormat::get<at_type>() \
		{ \
			static VertexFormat format( { __VA_ARGS__ } ); \
			return (format); \
		} \
	}

namespace at
{
	struct ATEMA_RENDERER_API VertexComponent
	{
		enum class Type : unsigned int
		{
			Unknown = 0,
			Int,		Int2,		Int3,		Int4,
			Unsigned,	Unsigned2,	Unsigned3,	Unsigned4,
			Float,		Float2,		Float3,		Float4,
			Double,		Double2,	Double3,	Double4
		};
		
		enum class Usage : unsigned int
		{
			Custom = 0, Position, Normal, Tangent, Color, Texture
		};
		
		Type type;
		Usage usage;
		unsigned int byte_offset;
		
		VertexComponent();
		virtual ~VertexComponent() = default;
		
		template <typename T>
		static VertexComponent get(Usage usage);
		
		static unsigned int type_byte_size(Type type);
		
		private:
			template <typename T>
			static inline Type get_type();
	};
	
	class ATEMA_RENDERER_API VertexFormat
	{
		public:
			VertexFormat() = default;
			virtual ~VertexFormat() = default;
			
			template <typename T>
			static const VertexFormat& get();
			
			size_t get_size() const;
			size_t get_byte_size() const;
			
			VertexComponent operator[](size_t index);
			
		private:
			VertexFormat(std::initializer_list<VertexComponent> components);
			
			std::vector<VertexComponent> m_components;
			size_t m_byte_size;
	};
}

//Base types component declaration
ATEMA_DECLARE_VERTEX_COMPONENT_TYPE(int,			VertexComponent::Type::Int)
ATEMA_DECLARE_VERTEX_COMPONENT_TYPE(Vector2i,		VertexComponent::Type::Int2)
ATEMA_DECLARE_VERTEX_COMPONENT_TYPE(Vector3i,		VertexComponent::Type::Int3)
ATEMA_DECLARE_VERTEX_COMPONENT_TYPE(Vector4i,		VertexComponent::Type::Int4)
ATEMA_DECLARE_VERTEX_COMPONENT_TYPE(unsigned int,	VertexComponent::Type::Unsigned)
ATEMA_DECLARE_VERTEX_COMPONENT_TYPE(Vector2u,		VertexComponent::Type::Unsigned2)
ATEMA_DECLARE_VERTEX_COMPONENT_TYPE(Vector3u,		VertexComponent::Type::Unsigned3)
ATEMA_DECLARE_VERTEX_COMPONENT_TYPE(Vector4u,		VertexComponent::Type::Unsigned4)
ATEMA_DECLARE_VERTEX_COMPONENT_TYPE(float,			VertexComponent::Type::Float)
ATEMA_DECLARE_VERTEX_COMPONENT_TYPE(Vector2f,		VertexComponent::Type::Float2)
ATEMA_DECLARE_VERTEX_COMPONENT_TYPE(Vector3f,		VertexComponent::Type::Float3)
ATEMA_DECLARE_VERTEX_COMPONENT_TYPE(Vector4f,		VertexComponent::Type::Float4)
ATEMA_DECLARE_VERTEX_COMPONENT_TYPE(double,			VertexComponent::Type::Double)
ATEMA_DECLARE_VERTEX_COMPONENT_TYPE(Vector2d,		VertexComponent::Type::Double2)
ATEMA_DECLARE_VERTEX_COMPONENT_TYPE(Vector3d,		VertexComponent::Type::Double3)
ATEMA_DECLARE_VERTEX_COMPONENT_TYPE(Vector4d,		VertexComponent::Type::Double4)
ATEMA_DECLARE_VERTEX_COMPONENT_TYPE(Color,			VertexComponent::Type::Float4)

//VBO of Vector3f is by default a 3D position buffer for convenience
ATEMA_DECLARE_VERTEX_FORMAT
(
	Vector3f,
	VertexComponent::get<Vector3f>(VertexComponent::Usage::Position)
)

#include <Atema/Renderer/Vertex.tpp>

#endif