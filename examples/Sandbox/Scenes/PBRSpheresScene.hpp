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

#ifndef ATEMA_SANDBOX_PBRSPHERESSCENE_HPP
#define ATEMA_SANDBOX_PBRSPHERESSCENE_HPP

#include "../Scene.hpp"

namespace at
{
	class Model;
}

class PBRSpheresScene : public Scene
{
public:
	PBRSpheresScene();
	PBRSpheresScene(const PBRSpheresScene& other) = default;
	PBRSpheresScene(PBRSpheresScene&& other) noexcept = default;
	~PBRSpheresScene();

	PBRSpheresScene& operator=(const PBRSpheresScene& other) = default;
	PBRSpheresScene& operator=(PBRSpheresScene&& other) noexcept = default;

protected:
	void createModels() override;
	void createLights() override;
	void createSkyBox() override;

	void initCamera(CameraComponent& camera) override;

private:
	at::Ptr<at::Model> m_model;
	uint32_t m_objectRows;
	std::vector<at::EntityHandle> m_objects;
};

#endif
