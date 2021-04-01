/*
 * This example shows how instantiate a video clip
 * 
 * How to compile:
 * c++ 00\ -\ Instantiating.cpp -std=c++17 -Wall -Wextra -lzuazo -lzuazo-window -lzuazo-ffmpeg -lzuazo-compositor -lzuazo-ndi -lglfw -ldl
 */

#include <zuazo/Instance.h>
#include <zuazo/Player.h>
#include <zuazo/Modules/Window.h>
#include <zuazo/Consumers/WindowRenderer.h>
#include <zuazo/Processors/Layers/VideoSurface.h>
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

	//Construct the desired parameters
	const Zuazo::VideoMode videoMode(
		Zuazo::Utils::MustBe<Zuazo::Rate>(Zuazo::Rate(25, 1)), //Just specify the desired rate
		Zuazo::Utils::Any<Zuazo::Resolution>(),
		Zuazo::Utils::Any<Zuazo::AspectRatio>(),
		Zuazo::Utils::Any<Zuazo::ColorPrimaries>(),
		Zuazo::Utils::Any<Zuazo::ColorModel>(),
		Zuazo::Utils::Any<Zuazo::ColorTransferFunction>(),
		Zuazo::Utils::Any<Zuazo::ColorSubsampling>(),
		Zuazo::Utils::Any<Zuazo::ColorRange>(),
		Zuazo::Utils::Any<Zuazo::ColorFormat>()	
	);

	const Zuazo::Utils::Limit<Zuazo::DepthStencilFormat> depthStencil(
		Zuazo::Utils::MustBe<Zuazo::DepthStencilFormat>(Zuazo::DepthStencilFormat::NONE) //Not interested in the depth buffer
	);

	const auto windowSize = Zuazo::Math::Vec2i(1280, 720);

	const auto& monitor = Zuazo::Consumers::WindowRenderer::NO_MONITOR; //Not interested in the full-screen mode

	//Construct the window object
	Zuazo::Consumers::WindowRenderer window(
		instance, 						//Instance
		"Output Window",				//Layout name
		videoMode,						//Video mode limits
		depthStencil,					//Depth buffer limits
		windowSize,						//Window size (in screen coordinates)
		monitor							//Monitor for setting fullscreen
	);

	//Open the window (now becomes visible)
	window.setResizeable(false); //Disable resizeing, as extra care needs to be taken
	window.asyncOpen(lock);

	//Create a layer for rendering to the window
	Zuazo::Processors::Layers::VideoSurface videoSurface(
		instance,
		"Video Surface",
		&window,
		window.getVideoMode().getResolutionValue()
	);

	window.setLayers({videoSurface});
	videoSurface.setScalingMode(Zuazo::ScalingMode::BOXED);
	videoSurface.setScalingFilter(Zuazo::ScalingFilter::CUBIC);
	videoSurface.asyncOpen(lock);

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
		Zuazo::VideoMode::ANY,
		sources[0]
	);
	ndiSource.asyncOpen(lock);

	//Route the signal
	videoSurface << ndiSource;

	//Done!
	lock.unlock();
	getchar();
	lock.lock();

	std::cout << "\nSource's video-mode:\n";
	std::cout << "\t-" << ndiSource.getVideoMode() << "\n";
}