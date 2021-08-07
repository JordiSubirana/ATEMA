#include <Atema/Atema.hpp>

#include <entt/entt.hpp>

#include <map>
#include <iostream>

using namespace at;

// #1 - BENCHMARK SPARSE SET
void benchmarkSparseSet()
{
	std::unordered_map<int, std::string> map;
	SparseSet<std::string> set;

	std::vector<int> dataInt;
	std::vector<std::string> dataStr;

	//srand(time(NULL));

	auto count = 1'000'000;
	auto maxValue = 1'000'000;
	dataInt.resize(count);
	dataStr.resize(count);
	for (int i = 0; i < count; i++)
	{
		dataInt[i] = (rand() * rand() + rand()) % maxValue;
		dataStr[i] = std::to_string(dataInt[i]);

		//std::cout << dataStr[i] << "\n";
	}

	std::cout << "Insertion\n";

	{
		ScopedTimer timer;

		for (int i = 0; i < count; i++)
		{
			map[dataInt[i]] = dataStr[i];
		}
	}

	{
		ScopedTimer timer;

		for (int i = 0; i < count; i++)
		{
			set.insert(dataInt[i], dataStr[i]);
		}
	}

	std::cout << "Traversal\n";

	{
		ScopedTimer timer;

		int total = 0;

		for (auto& v : map)
		{
			total += v.second.size();
		}

		std::cout << total << "\n";
	}

	{
		ScopedTimer timer;

		int total = 0;

		for (auto& v : set)
		{
			total += v.size();
		}

		std::cout << total << "\n";
	}

	std::cout << "Deletion\n";

	{
		ScopedTimer timer;

		for (int i = 0; i < count; i++)
		{
			map.erase(dataInt[i]);
		}
	}

	{
		ScopedTimer timer;

		for (int i = 0; i < count; i++)
		{
			set.erase(dataInt[i]);
		}
	}
}

// ECS DEFINITION
struct Position
{
	Vector3f value;
};

struct Velocity
{
	Vector3f value;
};

struct Name
{
	std::string value;
};

// #2 - ENTITY MANAGER
void entityManager()
{
	EntityManager mgr;

	auto e1 = mgr.createEntity();
	auto& n1 = mgr.createComponent<Name>(e1);
	auto& p1 = mgr.createComponent<Position>(e1);
	auto& v1 = mgr.createComponent<Velocity>(e1);

	auto e2 = mgr.createEntity();
	auto& n2 = mgr.createComponent<Name>(e2);
	auto& p2 = mgr.createComponent<Position>(e2);

	auto e3 = mgr.createEntity();
	auto& n3 = mgr.createComponent<Name>(e3);
	auto& v3 = mgr.createComponent<Velocity>(e3);

	auto u = mgr.getUnion<Name, Position>();
}

// #3 - BENCHMARK SPARSE SET UNION
void benchmarkSparseSetUnion()
{
	std::vector<int> commonData;

	int c = 1000000;
	for (int i = 0; i < c; i++)
	{
		commonData.push_back(i);
	}

	SparseSet<int> setInt;
	SparseSet<std::string> setStr;
	SparseSet<float> setFloat;

	entt::registry _reg;


	for (auto& i : commonData)
	{
		auto e = _reg.create();

		if (i > c / 4)
		{
			setInt.insert(i, i);
			_reg.emplace<int>(e, i);
		}

		if (i < 3 * (c / 4))
		{
			auto str = std::to_string(i);
			setStr.insert(i, str);
			_reg.emplace<std::string>(e, str);
		}

		//setInt.insert(i, i);
		//setStr.insert(i, std::to_string(i));
		setFloat.insert(i, (float)i);

		//_reg.emplace<int>(e, i);
		//_reg.emplace<std::string>(e, std::to_string(i));
		_reg.emplace<float>(e, (float)i);
	}

	std::cout << c << " entities\n";
	
	std::cout << "\nSparseSetUnion : ";
	{
		ScopedTimer timer;

		SparseSetUnion<int, std::string, float> view(setInt, setStr, setFloat);

		int s = 0;
		int total = 0;

		for (auto& id : view)
		{
			total += view.get<int>(id);
			s++;
		}

		std::cout << total << " - " << view.size() << " (" << s << " entities)" << std::endl;
	}

	std::cout << "\nenTT view (group not built): ";
	{
		ScopedTimer timer;

		auto view = _reg.view<int, std::string, float>();

		int s = 0;
		int total = 0;

		for (auto& id : view)
		{
			total += view.get<int>(id);
			s++;
		}

		std::cout << total << " - " << view.size_hint() << " (" << s << " entities)" << std::endl;
	}

	std::cout << "\nenTT group (group not built) : ";
	{
		ScopedTimer timer;

		auto group = _reg.group<int, std::string, float>();

		int s = 0;
		int total = 0;

		for (auto& id : group)
		{
			total += group.get<int>(id);
			s++;
		}

		std::cout << total << " - " << group.size() << " (" << s << " entities)" << std::endl;
	}

	std::cout << "\nenTT view (group built): ";
	{
		ScopedTimer timer;

		auto view = _reg.view<int, std::string, float>();

		int s = 0;
		int total = 0;

		for (auto& id : view)
		{
			total += view.get<int>(id);
			s++;
		}

		std::cout << total << " - " << view.size_hint() << " (" << s << " entities)" << std::endl;
	}
	
	std::cout << "\nenTT group (group built) : ";
	{
		ScopedTimer timer;

		auto group = _reg.group<int, std::string, float>();

		int s = 0;
		int total = 0;

		for (auto& id : group)
		{
			total += group.get<int>(id);
			s++;
		}

		std::cout << total << " - " << group.size() << " (" << s << " entities)" << std::endl;
	}
}

// Application

// #4 - Basic application
class TestLayer : public ApplicationLayer
{
public:
	TestLayer()
	{
		c = 1;
		count = 0.0f;

		Renderer::Settings settings;
		//settings.mainWindowSettings.width = 1920;
		//settings.mainWindowSettings.height = 1080;

		Renderer::create<VulkanRenderer>(settings);
		
		window = Renderer::getInstance().getMainWindow();

		auto windowSize = window->getSize();

		swapChain = SwapChain::create({ window, ImageFormat::BGRA8_SRGB });

		RenderPass::Settings renderPassSettings;
		renderPassSettings.attachments.resize(2);
		renderPassSettings.attachments[0].format = ImageFormat::BGRA8_SRGB;
		renderPassSettings.attachments[0].finalLayout = ImageLayout::Present;
		renderPassSettings.attachments[1].format = ImageFormat::D32F;

		renderPass = RenderPass::create(renderPassSettings);

		Image::Settings depthSettings;
		depthSettings.width = windowSize.x;
		depthSettings.height = windowSize.y;
		depthSettings.format = ImageFormat::D32F;
		depthSettings.usages = ImageUsage::RenderTarget;
		
		depthImage = Image::create(depthSettings);
		
		Framebuffer::Settings framebufferSettings;
		framebufferSettings.renderPass = renderPass;
		framebufferSettings.width = windowSize.x;
		framebufferSettings.height = windowSize.y;

		for (auto& image : swapChain->getImages())
		{
			framebufferSettings.images =
			{
				image,
				depthImage
			};

			framebuffers.push_back(Framebuffer::create(framebufferSettings));
		}
	}

	~TestLayer()
	{
		window.reset();
	}
	
	void onEvent(Event& event) override
	{
		
	}

	void drawFrame()
	{
		
	}
	
	void update(TimeStep ms) override
	{
		count += ms.getSeconds();

		if (count >= (float)c)
		{
			c++;
			//std::cout << count << std::endl;
		}
		
		/*if (count >= 10.0f)
			Application::instance().close();*/

		if (window->shouldClose())
		{
			Application::instance().close();
			return;
		}

		window->processEvents();

		//static_cast<VulkanRenderer&>(Renderer::getInstance()).drawFrame();

		window->swapBuffers();
	}

	int c;
	float count;
	Ptr<Window> window;
	Ptr<SwapChain> swapChain;
	Ptr<RenderPass> renderPass;
	Ptr<Image> depthImage;
	std::vector<Ptr<Framebuffer>> framebuffers;
};

void basicApplication()
{
	auto layer = new TestLayer();

	auto& app = Application::instance();

	app.addLayer(layer);

	app.run();

	delete layer;
}

// MAIN
int main(int argc, char** argv)
{
	try
	{
		//benchmarkSparseSet();

		//entityManager();

		//benchmarkSparseSetUnion();

		basicApplication();
	}
	catch (const std::exception& e)
	{
		std::cout << e.what() << std::endl;
		return -1;
	}
	
	return 0;
}