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

#ifndef ATEMA_GRAPHICS_INDEXED_ARRAY_IMPLEMENTATION
#define ATEMA_GRAPHICS_INDEXED_ARRAY_IMPLEMENTATION

namespace at
{
	template <typename T>
	IndexedArray<T>::IndexedArray()
	{
		
	}
	
	template <typename T>
	IndexedArray<T>::IndexedArray(const T *elements, size_t elements_size, const unsigned int *indices, size_t indices_size, typename BufferArray<T>::update_mode elements_update_mode, IndexArray::update_mode indices_update_mode) :
		elements(elements, elements_size, elements_update_mode),
		indices(indices, indices_size, indices_update_mode)
	{
		
	}
	
	template <typename T>
	IndexedArray<T>::IndexedArray(const BufferArray<T>& elements, const IndexArray& indices) :
		elements(elements),
		indices(indices)
	{
		
	}
	
	template <typename T>
	IndexedArray<T>::IndexedArray(const IndexedArray<T>& array) :
		IndexedArray(array.elements, array.indices)
	{
		
	}
	
	template <typename T>
	IndexedArray<T>::~IndexedArray() noexcept
	{
		
	}
	
	template <typename T>
	void IndexedArray<T>::create(const T *elements, size_t elements_size, const unsigned int *indices, size_t indices_size, typename BufferArray<T>::update_mode elements_update_mode, IndexArray::update_mode indices_update_mode)
	{
		this->elements.create(elements, elements_size, elements_update_mode);
		this->indices.create(indices, indices_size, indices_update_mode);
	}
	
	template <typename T>
	void IndexedArray<T>::create(const BufferArray<T>& elements, const IndexArray& indices)
	{
		this->elements.create(elements);
		this->indices.create(indices);
	}
	
	template <typename T>
	void IndexedArray<T>::create(const IndexedArray<T>& array)
	{
		create(array.elements, array.indices);
	}
	
	template <typename T>
	T& IndexedArray<T>::operator[](size_t index)
	{
		if (!indices.is_valid())
			return (elements[index]);
		
		return (elements[indices[index]]);
	}
	
	template <typename T>
	const T& IndexedArray<T>::operator[](size_t index) const
	{
		if (!indices.is_valid())
			return (elements[index]);
		
		return (elements[indices[index]]);
	}
	
	template <typename T>
	size_t IndexedArray<T>::get_size() const
	{
		return (indices.get_size());
	}
	
	template <typename T>
	GLuint IndexedArray<T>::get_gl_id() const noexcept
	{
		return (elements.get_gl_id());
	}
	
	template <typename T>
	bool IndexedArray<T>::is_valid() const noexcept
	{
		return (elements.is_valid() && indices.is_valid());
	}
	
	template <typename T>
	void IndexedArray<T>::to_cpu()
	{
		elements.to_cpu();
		indices.to_cpu();
	}
	
	template <typename T>
	void IndexedArray<T>::to_gpu()
	{
		elements.to_gpu();
		indices.to_gpu();
	}
}

#endif