#include "Hostname.h"

#include <array>
#include <cassert>

#ifdef _WIN32
	#include "winsock.h"
#else
	#include "unistd.h"
#endif

namespace Zuazo {

std::string getHostname() {
	constexpr size_t MAX_LENGTH = 256;
	std::array<char, MAX_LENGTH> buf;

	//Query the hostname of the machine
	gethostname(buf.data(), buf.size());
	
	return std::string(buf.data());
}

}