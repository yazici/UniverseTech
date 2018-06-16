#include "source/UniEngine.h"
#include <iostream>
#include <limits>

// OS specific macros for the example main entry points
#if defined(_WIN32)
// Windows entry point
#define UNIENGINE_MAIN()																		\
UniEngine *engine;																		\
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)						\
{																									\
	if (engine != NULL)																		\
	{																								\
		engine->handleMessages(hWnd, uMsg, wParam, lParam);									\
	}																								\
	return (DefWindowProc(hWnd, uMsg, wParam, lParam));												\
}																									\
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int)									\
{																									\
	for (int32_t i = 0; i < __argc; i++) { UniEngine::args.push_back(__argv[i]); };  			\
	engine = new UniEngine();															\
	engine->initVulkan();																	\
	engine->setupWindow(hInstance, WndProc);													\
	engine->prepare();																		\
	engine->renderLoop();																	\
	delete(engine);																			\
	return 0;																						\
}																									
#elif defined(VK_USE_PLATFORM_ANDROID_KHR)
// Android entry point
#define UNIENGINE_MAIN()																		\
UniEngine *engine;																		\
void android_main(android_app* state)																\
{																									\
	engine = new UniEngine();															\
	state->userData = engine;																\
	state->onAppCmd = UniEngine::handleAppCommand;												\
	state->onInputEvent = UniEngine::handleAppInput;											\
	androidApp = state;																				\
	vks::android::getDeviceConfig();																\
	engine->renderLoop();																	\
	delete(engine);																			\
}
#elif defined(_DIRECT2DISPLAY)
// Linux entry point with direct to display wsi
#define UNIENGINE_MAIN()																		\
UniEngine *engine;																		\
static void handleEvent()                                											\
{																									\
}																									\
int main(const int argc, const char *argv[])													    \
{																									\
	for (size_t i = 0; i < argc; i++) { UniEngine::args.push_back(argv[i]); };  				\
	engine = new UniEngine();															\
	engine->initVulkan();																	\
	engine->prepare();																		\
	engine->renderLoop();																	\
	delete(engine);																			\
	return 0;																						\
}
#elif defined(VK_USE_PLATFORM_WAYLAND_KHR)
#define UNIENGINE_MAIN()																		\
UniEngine *engine;																		\
int main(const int argc, const char *argv[])													    \
{																									\
	for (size_t i = 0; i < argc; i++) { UniEngine::args.push_back(argv[i]); };  				\
	engine = new UniEngine();															\
	engine->initVulkan();																	\
	engine->setupWindow();					 												\
	engine->prepare();																		\
	engine->renderLoop();																	\
	delete(engine);																			\
	return 0;																						\
}
#elif defined(VK_USE_PLATFORM_XCB_KHR)
#define UNIENGINE_MAIN()																		\
UniEngine *engine;																		\
static void handleEvent(const xcb_generic_event_t *event)											\
{																									\
	if (engine != NULL)																		\
	{																								\
		engine->handleEvent(event);															\
	}																								\
}																									\
int main(const int argc, const char *argv[])													    \
{																									\
	for (size_t i = 0; i < argc; i++) { UniEngine::args.push_back(argv[i]); };  				\
	engine = UniEngine::GetInstance();											\
	engine->initVulkan();																	\
	engine->setupWindow();					 												\
	engine->prepare();																		\
	engine->renderLoop();																	\
	delete(engine);																			\
	return 0;																						\
}
#elif (defined(VK_USE_PLATFORM_IOS_MVK) || defined(VK_USE_PLATFORM_MACOS_MVK))
#define UNIENGINE_MAIN()
#endif


UNIENGINE_MAIN()
