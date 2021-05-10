/*
 * This example shows how instantiate a video clip
 * 
 * How to compile:
 * c++ 00\ -\ Instantiating.cpp -std=c++17 -Wall -Wextra -lzuazo -lzuazo-window -lzuazo-ffmpeg -lzuazo-compositor -lzuazo-ndi -lglfw -ldl
 */

#include <zuazo/Instance.h>
#include <zuazo/Player.h>
#include <zuazo/Modules/Window.h>
#include <zuazo/Renderers/Window.h>
#include <zuazo/Consumers/RendererWrapper.h>
#include <zuazo/NDI/Finder.h>
#include <zuazo/Sources/NDI.h>

#include <mutex>
#include <iostream>

int main(int argc, const char* argv[]) {
	//Instantiate Zuazo as usual. Note that we're loading the Window module
	Zuazo::Instance::ApplicationInfo appInfo(
		"NDI Example 00",							//Application's name
		Zuazo::Version(0, 1, 0),					//Application's version
		Zuazo::Verbosity::GEQ_INFO,					//Verbosity 
		{ Zuazo::Modules::Window::get() }			//Modules
	);
	Zuazo::Instance instance(std::move(appInfo));
	std::unique_lock<Zuazo::Instance> lock(instance);

	//Construct the window object
	Zuazo::Consumers::RendererWrapper<Zuazo::Renderers::Window> window(
		instance, 						//Instance
		"Output Window",				//Layout name
		Zuazo::Math::Vec2i(1280, 720)	//Window size (in screen coordinates)
	);

	//Set the negotiation callback
	window.setVideoModeNegotiationCallback(
		[] (Zuazo::VideoBase&, const std::vector<Zuazo::VideoMode>& compatibility) -> Zuazo::VideoMode {
			auto result = compatibility.front();
			result.setFrameRate(Zuazo::Utils::MustBe<Zuazo::Rate>(result.getFrameRate().highest()));
			return result;
		}
	);

	//Open the window (now becomes visible)
	window.asyncOpen(lock);

	//Choose a NDI source
	const Zuazo::NDI::Finder finder(true, nullptr, nullptr);
	Zuazo::Utils::BufferView<const Zuazo::NDI::Source> sources;
	while(sources.empty()) {
		std::cout << "No sources... Retrying in 1s" << std::endl;
		finder.waitForSources(1000);
		sources = finder.getCurrentSources();
	}

	std::cout << "Source found! name: " << sources[0].getName() << " url: " << sources[0].getURL() << std::endl;

	//Create a video source
	Zuazo::Sources::NDI ndiSource(
		instance,
		"NDI test input",
		sources[0]
	);
	ndiSource.setVideoModeNegotiationCallback(
		[] (Zuazo::VideoBase&, const std::vector<Zuazo::VideoMode>& compatibility) -> Zuazo::VideoMode {
			return compatibility.front();
		}
	);
	ndiSource.asyncOpen(lock);

	//Route the signal
	window << ndiSource;

	//Done!
	lock.unlock();
	getchar();
	lock.lock();

	std::cout << "\nSource's video-mode:\n";
	std::cout << "\t-" << ndiSource.getVideoMode() << "\n";
}