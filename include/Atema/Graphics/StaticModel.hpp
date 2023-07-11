/*
	Copyright 2023 Jordi SUBIRANA

	Permission is hereby granted, free of charge, to any person obtaining a copy of
	this software and associated documentation files (the "Software"), to deal in
	the Software without restriction, including without limitation the rights to use,
	copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the
	Software, and to permit persons to whom the Software is furnished to do so, subject
	to the following conditions:

	The above copyright notice and this permission notice shall be included in all
	copies or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
	INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
	PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
	HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
	CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE
	OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef ATEMA_GRAPHICS_STATICMODEL_HPP
#define ATEMA_GRAPHICS_STATICMODEL_HPP

#include <Atema/Graphics/Config.hpp>
#include <Atema/Graphics/Renderable.hpp>
#include <Atema/Graphics/Model.hpp>
#include <Atema/Core/Pointer.hpp>
#include <Atema/Math/AABB.hpp>
#include <Atema/Math/Transform.hpp>

namespace at
{
	class ATEMA_GRAPHICS_API StaticModel : public Renderable
	{
	public:
		StaticModel() = default;
		StaticModel(const StaticModel& other) = default;
		StaticModel(StaticModel&& other) noexcept = default;
		~StaticModel() = default;

		Ptr<RenderObject> createRenderObject(RenderScene& renderScene) override;

		void setModel(const Ptr<Model>& model);
		void setTransform(const Transform& transform);

		const Ptr<Model>& getModel() const noexcept;
		const Transform& getTransform() const noexcept override;
		const AABBf& getAABB() const noexcept override;

		StaticModel& operator=(const StaticModel& other) = default;
		StaticModel& operator=(StaticModel&& other) noexcept = default;

		Signal<> onModelUpdate;
		Signal<> onTransformUpdate;

	private:
		void updateAABB();

		Ptr<Model> m_model;
		Transform m_transform;
		AABBf m_aabb;

		ConnectionGuard m_modelConnectionGuard;
	};
}

#endif