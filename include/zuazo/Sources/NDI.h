#pragma once

#include <zuazo/ZuazoBase.h>
#include <zuazo/Video.h>
#include <zuazo/Signal/SourceLayout.h>
#include <zuazo/Utils/Pimpl.h>

#include "../NDI/Source.h"

#include <string>

namespace Zuazo::Sources {

struct NDIImpl;
class NDI 
	: private Utils::Pimpl<NDIImpl>
	, public ZuazoBase
	, public VideoBase
	, public Signal::SourceLayout<Video>
{
	friend NDIImpl;
public:

	class Source {
	public:
		Source() = default;
		Source(std::string name, std::string url);
		Source(const Zuazo::NDI::Source& src);
		Source(const Source& other) = default;
		Source(Source&& other) = default;
		~Source() = default;

		Source&							operator=(const Source& other) = default;
		Source&							operator=(Source&& other) = default;

		operator Zuazo::NDI::Source() const noexcept;

		void							setName(std::string name);
		const std::string&				getName() const noexcept;

		void							setURL(std::string url);
		const std::string&				getURL() const noexcept;

	private:
		std::string						m_name;
		std::string						m_url;
	};



	NDI(Instance& instance, 
		std::string name, 
		Source source );
	NDI(const NDI& other) = delete;
	NDI(NDI&& other);
	virtual ~NDI();

	NDI& 							operator=(const NDI& other) = delete;
	NDI& 							operator=(NDI&& other);

	void							setSource(Source source);
	const Source&					getSource() const noexcept;

	void							setProgramTally(bool tally);
	bool							getProgramTally() const noexcept;

	void							setPreviewTally(bool tally);
	bool							getPreviewTally() const noexcept;
	
};

}