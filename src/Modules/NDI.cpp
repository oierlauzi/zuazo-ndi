#include <zuazo/Modules/NDI.h>

#include <cassert>


#ifdef _WIN32
#include <windows.h>
#else
#include <stdlib.h>
#include <dlfcn.h>
#endif

#include "../Processing.NDI/Processing.NDI.Lib.h"

namespace Zuazo::Modules {

#ifdef _WIN32
#pragma error "NDI::DynamicLoad not implemented for Windows" //TODO
#else
class NDI::DynamicLoad {
public:
	DynamicLoad() 
		: ndiDl(nullptr)
		, ndiLib(nullptr)
	{
		//Based on SDK examples
		std::string ndiPath;
		
		//Obtain the path to the .so file
		const char* ndiRuntimeFolder = getenv(NDILIB_REDIST_FOLDER);
		if(ndiRuntimeFolder) {
			ndiPath = ndiRuntimeFolder;
		}
		ndiPath += NDILIB_LIBRARY_NAME;

		//Try to load the library
		ndiDl = dlopen(ndiPath.c_str(), RTLD_LOCAL | RTLD_LAZY);
		
		//The main NDI entry point for dynamic loading if we got the library
		const NDIlib_v4* (*ndiLoad)(void) = nullptr;
		if(ndiDl) {
			*((void**)&ndiLoad) = dlsym(ndiDl, "NDIlib_v4_load");
		}

		//Try to load the lib
		if(ndiLoad) {
			ndiLib = ndiLoad();
		}

		//Check if it was successfully loaded
		if(!ndiLib) {
			//Unload the library if we loaded it
			if(ndiDl) {
				dlclose(ndiDl);
			}
			throw Exception("NDI binaries could not be loaded");
		}

		//At this point it should be loaded
		assert(ndiLib);
	}

	~DynamicLoad() {
		assert(ndiDl);
		dlclose(ndiDl);
	}

	const NDIlib_v4& get() const {
		assert(ndiLib);
		return *ndiLib;
	}

private:
	void* ndiDl;
	const NDIlib_v4* ndiLib;
	
};
#endif

std::unique_ptr<NDI> NDI::s_singleton;

NDI::NDI() 
	: Instance::Module(std::string(name), version)
	, m_dynamicLoad(Utils::makeUnique<DynamicLoad>())
	, m_ndi(m_dynamicLoad->get())
{
	//Initialize the library
	getNDI().initialize();
}

NDI::~NDI() {
	//Terminate the library
	getNDI().destroy();
}

const NDI& NDI::get() {
	if(!s_singleton) {
		s_singleton = std::unique_ptr<NDI>(new NDI);
	}

	assert(s_singleton);
	return *s_singleton;
}


const NDIlib_v4& NDI::getNDI() const noexcept {
	return m_ndi;
}

}