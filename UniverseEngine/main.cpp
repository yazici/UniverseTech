#include "source/UniEngine.h"
#include <iostream>
#include <limits>

#include <iostream>

template < typename C, typename T = std::char_traits<C> >
struct basic_teebuf : public std::basic_streambuf<C, T> {
	typedef std::basic_streambuf<C, T> streambuf_type;
	typedef typename T::int_type int_type;

	basic_teebuf(streambuf_type* buff_a, streambuf_type* buff_b)
		: first(buff_a), second(buff_b) {}

protected:
	virtual int_type overflow(int_type c) {
		const int_type eof = T::eof();
		if(T::eq_int_type(c, eof)) return T::not_eof(c);
		else {
			const C ch = T::to_char_type(c);
			if(T::eq_int_type(first->sputc(ch), eof) ||
				T::eq_int_type(second->sputc(ch), eof))
				return eof;
			else return c;
		}
	}

	virtual int sync() {
		return !first->pubsync() && !second->pubsync() ? 0 : -1;
	}

private:
	streambuf_type * first;
	streambuf_type* second;
};

template < typename C, typename T = std::char_traits<C> >
struct basic_teestream : public std::basic_ostream<C, T> {
	typedef std::basic_ostream<C, T> stream_type;
	typedef basic_teebuf<C, T> streambuff_type;

	basic_teestream(stream_type& first, stream_type& second)
		: stream_type(&stmbuf), stmbuf(first.rdbuf(), second.rdbuf()) {}

	basic_teestream(streambuff_type* first, streambuff_type* second)
		: stream_type(&stmbuf), stmbuf(first, second) {}

	~basic_teestream() { stmbuf.pubsync(); }

private: streambuff_type stmbuf;
};

typedef basic_teebuf<char> teebuf;
typedef basic_teestream<char> teestream;

#include <fstream>
#include <iomanip>
#include <algorithm>
#include <iterator>

// OS specific macros for the example main entry points
#if defined(_WIN32)
// Windows entry point
#define UNIENGINE_MAIN()																		\
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)						\
{																									\
	UniEngine& engine = UniEngine::GetInstance();																		\
	engine.handleMessages(hWnd, uMsg, wParam, lParam);									\
	return (DefWindowProc(hWnd, uMsg, wParam, lParam));												\
}																									\
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int)									\
{																									\
	std::filebuf fbuf ; \
	fbuf.open("out.txt", std::ios::out); \
	std::streambuf* stdoutbuf = std::cout.rdbuf(); \
    std::streambuf* stderrbuf = std::cerr.rdbuf(); \
	teebuf tbuf(&fbuf, stdoutbuf); \
    teebuf terrbuf(&fbuf, stderrbuf); \
	std::cout.rdbuf(&tbuf); \
    std::cerr.rdbuf(&terrbuf); \
	for (int32_t i = 0; i < __argc; i++) { UniEngine::args.push_back(__argv[i]); };  			\
	UniEngine& engine = UniEngine::GetInstance();															\
	engine.initVulkan();																	\
	engine.setupWindow(hInstance, WndProc);													\
	engine.prepare();																		\
	engine.renderLoop();																	\
    engine.Shutdown(); \
    /* system("pause");*/ \
    std::cout << std::endl ; \
    std::cout.rdbuf(stdoutbuf); \
    std::cerr << std::endl ; \
    std::cerr.rdbuf(stderrbuf); \
	return 0;																						\
}																									
#elif defined(VK_USE_PLATFORM_ANDROID_KHR)
// Android entry point
#define UNIENGINE_MAIN()																		\
void android_main(android_app* state)																\
{																									\
	UniEngine& engine = UniEngine::GetInstance();															\
	state->userData = &engine;																\
	state->onAppCmd = UniEngine::handleAppCommand;												\
	state->onInputEvent = UniEngine::handleAppInput;											\
	androidApp = state;																				\
	vks::android::getDeviceConfig();																\
	engine.renderLoop();																	\
}
#elif defined(_DIRECT2DISPLAY)
// Linux entry point with direct to display wsi
#define UNIENGINE_MAIN()																		\
static void handleEvent()                                											\
{																									\
}																									\
int main(const int argc, const char *argv[])													    \
{																									\
	for (size_t i = 0; i < argc; i++) { UniEngine::args.push_back(argv[i]); };  				\
	UniEngine& engine = UniEngine::GetInstance();															\
	engine.initVulkan();																	\
	engine.prepare();																		\
	engine.renderLoop();																	\
	return 0;																						\
}
#elif defined(VK_USE_PLATFORM_WAYLAND_KHR)
#define UNIENGINE_MAIN()																		\
int main(const int argc, const char *argv[])													    \
{																									\
	for (size_t i = 0; i < argc; i++) { UniEngine::args.push_back(argv[i]); };  				\
	UniEngine& engine = UniEngine::GetInstance();															\
	engine.initVulkan();																	\
	engine.setupWindow();					 												\
	engine.prepare();																		\
	engine.renderLoop();																	\
	return 0;																						\
}
#elif defined(VK_USE_PLATFORM_XCB_KHR)
#define UNIENGINE_MAIN()																		\
static void handleEvent(const xcb_generic_event_t *event)											\
{																									\
	UniEngine& engine = UniEngine::GetInstance();																		\
	engine.handleEvent(event);															\
}																									\
int main(const int argc, const char *argv[])													    \
{																									\
	for (size_t i = 0; i < argc; i++) { UniEngine::args.push_back(argv[i]); };  				\
	UniEngine& engine = UniEngine::GetInstance();											\
	engine.initVulkan();																	\
	engine.setupWindow();					 												\
	engine.prepare();																		\
	engine.renderLoop();																	\
	return 0;																						\
}
#elif (defined(VK_USE_PLATFORM_IOS_MVK) || defined(VK_USE_PLATFORM_MACOS_MVK))
#define UNIENGINE_MAIN()
#endif


UNIENGINE_MAIN()
