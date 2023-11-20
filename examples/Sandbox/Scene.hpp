/*
	Copyright 2022 Jordi SUBIRANA

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

#ifndef ATEMA_SANDBOX_SCENE_HPP
#define ATEMA_SANDBOX_SCENE_HPP

#include <Atema/Core/EntityManager.hpp>
#include <Atema/Core/Signal.hpp>
#include <Atema/Math/AABB.hpp>

#include <unordered_set>

struct CameraComponent;

namespace at
{
	class RenderScene;
}

// Scene base class
// Used for hard coded scenes until the engine supports a proper dynamic scene system
class Scene
{
public:
	Scene();
	Scene(const Scene& other) = default;
	Scene(Scene&& other) noexcept = default;
	virtual ~Scene();

	static Scene& getCurrent();

	void initialize(at::EntityManager& entityManager, at::RenderScene& renderScene);

	// Does nothing by default
	virtual void update();

	at::AABBf& getAABB();
	
	Scene& operator=(const Scene& other) = default;
	Scene& operator=(Scene&& other) noexcept = default;

	at::Signal<at::EntityHandle> onEntityAdded;
	at::Signal<at::EntityHandle> onEntityRemoved;

protected:
	// Default: cube and plane
	// Override to create custom models
	virtual void createModels();
	// Default: directional light with shadows
	// Override to create custom lights
	virtual void createLights();
	// Default: enabled
	// Override to disable automatic camera
	virtual void createCamera();
	// Default: enabled
	// Override to disable manual camera
	virtual void createPlayer();
	// Default: none
	// Override and use setSkyBox method
	virtual void createSkyBox();

	virtual void initCamera(CameraComponent& camera);

	void addEntity(at::EntityHandle entity);
	void removeEntity(at::EntityHandle entity);

	at::EntityManager& getEntityManager() const;
	void setSkyBox(const std::filesystem::path& texturePath, bool cubemap);

private:
	at::EntityManager* m_entityManager;
	at::RenderScene* m_renderScene;
	at::AABBf m_aabb;
	std::unordered_set<at::EntityHandle> m_entities;
};

#endif
