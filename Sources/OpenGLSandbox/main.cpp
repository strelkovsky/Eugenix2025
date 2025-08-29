#include <functional>

// Tests headers
#include "Tests/0-TriangleApp.h"
#include "Tests/1-SquareApp.h"
#include "Tests/2-SimpleCameraApp.h"
#include "Tests/3-TextureApp.h"
#include "Tests/4.1-TinyObjLoader.h"
#include "Tests/4.2-AssimpLoader.h"
#include "Tests/6-DebugDraw.h"
#include "Tests/8-Skybox.h"

#include "Tests/LearnOpenGLApp.h"
#include "Tests/MainSandbox.h"

using namespace Eugenix;

struct TestEntry 
{
	std::string key;
	std::string title;
	std::function<std::shared_ptr<SandboxApp>()> make;
};

static const std::vector<TestEntry> EugenixTests;

#define REGISTER_TEST(KEY, TITLE, TYPE) \
    namespace { \
        struct TYPE##_AutoReg { \
            TYPE##_AutoReg() { \
                const_cast<std::vector<TestEntry>&>(EugenixTests).push_back( \
                    TestEntry{ KEY, TITLE, []{ return std::make_shared<TYPE>(); } } \
                ); \
            } \
        } s_##TYPE##_AutoReg; \
    }

REGISTER_TEST("1", "TriangleApp", TriangleApp);
REGISTER_TEST("2", "SquareApp", SquareApp);
REGISTER_TEST("3", "SimpleCameraApp", SimpleCameraApp);
REGISTER_TEST("4", "TextureApp", TextureApp);
REGISTER_TEST("5", "TinyObjLoader", TinyObjLoaderApp);
REGISTER_TEST("6", "AssimpLoader", AssimpLoaderApp);
REGISTER_TEST("7", "DebugDraw", DebugDrawerApp);
REGISTER_TEST("8", "Skybox", SkyboxApp);

static inline std::string trim(std::string s) 
{
	auto is_space = [](unsigned char c) { return std::isspace(c); };
	s.erase(s.begin(), std::find_if(s.begin(), s.end(), [&](unsigned char c) { return !is_space(c); }));
	s.erase(std::find_if(s.rbegin(), s.rend(), [&](unsigned char c) { return !is_space(c); }).base(), s.end());
	return s;
}

class TestRunner
{
public:
	static int Run(std::shared_ptr<SandboxApp> app)
	{
		return app->Run();
	}
};

int main()
{
    std::string lastKey;

    while (true) 
    {
        // Рисуем меню
        std::cout << "Select Test (q - exit";
        if (!lastKey.empty()) std::cout << ", Enter - rerun \"" << lastKey << "\"";
        std::cout << "):\n\n";

        for (const auto& t : EugenixTests)
        {
            std::cout << "  " << t.key << " - " << t.title << "\n";
        }
        std::cout << std::endl;

        std::string read;
        std::getline(std::cin, read);
        if (!std::cin.good()) break;

        read = trim(read);

        if (read == "q" || read == "Q") 
            break;

        if (read.empty()) 
        {
            if (lastKey.empty()) continue;
            read = lastKey;
        }

        const TestEntry* chosen = nullptr;

        auto it = std::find_if(EugenixTests.begin(), EugenixTests.end(), [&](const TestEntry& t) { return t.key == read; });
        if (it != EugenixTests.end())
        {
            chosen = &*it;
        }

        if (!chosen) 
        {
            std::cout << "Unknown test: \"" << read << "\"\n\n";
            continue;
        }

        lastKey = chosen->key;
        int result = 0;
        try 
        {
            result = TestRunner::Run(chosen->make());
        }
        catch (const std::exception& e) 
        {
            std::cout << "App threw exception: " << e.what() << "\n";
            result = -1;
        }
        catch (...) 
        {
            std::cout << "App threw unknown exception.\n";
            result = -2;
        }

        std::cout << "\nApp exit with code - " << result << "\n";
        std::cout << "===============================================================\n\n";
    }
}