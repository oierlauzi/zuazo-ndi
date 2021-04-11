#include <zuazo/Sources/NDI.h>

#include "../Hostname.h"

#include <zuazo/NDI/Recv.h>
#include <zuazo/NDI/FrameSync.h>
#include <zuazo/NDI/Conversions.h>
#include <zuazo/Graphics/Uploader.h>
#include <zuazo/Signal/Output.h>


#include <utility>
#include <memory>

namespace Zuazo::Sources {

/*
 * NDIImpl
 */

struct NDIImpl {
	struct Open {
		typedef void (*copy_fn)(const Zuazo::NDI::VideoFrame&, Zuazo::Graphics::StagedFrame&);

		Zuazo::NDI::Recv						receiver;
		Zuazo::NDI::FrameSync					frameSync;
		Zuazo::NDI::VideoFrame					ndiFrame;
		std::unique_ptr<Graphics::Uploader>		uploader;
		std::shared_ptr<Graphics::StagedFrame>	uploadedFrame;
		copy_fn									copyCallback;


		Open(	Zuazo::NDI::Source source, 
				const std::string& name,
				bool pgmTally, bool pvwTally)
			: receiver(createReceiver(source, name))
			, frameSync(receiver)
			, ndiFrame()
			, uploader()
			, uploadedFrame()
			, copyCallback(nullptr)
		{
			receiver.setTally(pgmTally, pvwTally);
		}

		~Open() = default;

		VideoMode getSupportedVideoMode(const Graphics::Vulkan& vulkan) {
			//Convert everything
			const auto frameRate = ndiFrame.getFrameRate();
			const auto resolution = ndiFrame.getResolution();
			const auto pixelAspectRatio = getPixelAspectRatio(resolution, ndiFrame.getPictureAspectRatio());
			const auto [ycbcrColorModel, colorPrimaries] = getColorimetry(resolution);
			const auto [colorFormat, colorSubsampling, colorModel] = fromFourCC(ndiFrame.getFourCC(), ycbcrColorModel);
			constexpr auto colorTransferFunction = ColorTransferFunction::BT601; //Equivalent for 709, 2020
			constexpr auto colorRange = ColorRange::FULL;

			//TODO add alternative formats
			const auto formatCompatibility = Graphics::Uploader::getSupportedFormats(vulkan);
			return VideoMode(
				Utils::MustBe<Rate>(frameRate),
				Utils::MustBe<Resolution>(resolution),
				Utils::MustBe<AspectRatio>(pixelAspectRatio),
				Utils::MustBe<ColorPrimaries>(colorPrimaries),
				Utils::MustBe<ColorModel>(colorModel),
				Utils::MustBe<ColorTransferFunction>(colorTransferFunction),
				Utils::MustBe<ColorSubsampling>(colorSubsampling),
				Utils::MustBe<ColorRange>(colorRange),
				formatCompatibility.intersect(Utils::MustBe<ColorFormat>(colorFormat))
			);
		}

		void recreate(	const Graphics::Vulkan& vulkan, 
						const Graphics::Frame::Descriptor& desc )
		{
			if(uploader) {
				*uploader = Graphics::Uploader(vulkan, desc);
			} else {
				uploader = Utils::makeUnique<Graphics::Uploader>(vulkan, desc);
			}

			copyCallback = selectCopyFunction(ndiFrame.getFourCC(), desc.getColorFormat());
		}

		void recreate() {
			uploader.reset();
			copyCallback = nullptr;
		}

		void setSource(const Zuazo::NDI::Source& src) {
			receiver.connect(src);
		}

		bool pullFrame() {
			//Preserve a copy to check if it changes
			const auto prevFrame = ndiFrame;

			//If there is data associated to the last frame, free it
			if(ndiFrame.getData()) {
				frameSync.free(ndiFrame);
			}

			//Write a new frame to it
			frameSync.capture(ndiFrame, Zuazo::NDI::VideoFrame::Format::PROGRESSIVE);

			//Force uploading
			uploadedFrame.reset();

			//Check if the parameters have changed
			return 	prevFrame.getResolution() != ndiFrame.getResolution() ||
					prevFrame.getFourCC() != ndiFrame.getFourCC() ||
					prevFrame.getFrameRate() != ndiFrame.getFrameRate() ||
					prevFrame.getPictureAspectRatio() != ndiFrame.getPictureAspectRatio() ;
		}

		Video uploadFrame() {
			//Only upload if valid and modified
			if(!uploadedFrame && uploader && ndiFrame.getData()) {
				uploadedFrame = uploader->acquireFrame();
				assert(uploadedFrame);
				
				//In order to copy "normally"
				assert(ndiFrame.getFormat() == Zuazo::NDI::VideoFrame::Format::PROGRESSIVE);

				//Copy the data from one frame to the other
				assert(copyCallback);
				copyCallback(ndiFrame, *uploadedFrame);
				uploadedFrame->flush();

				//Its data is not needed anymore. Return it
				frameSync.free(ndiFrame);
			} else if(!uploader) {
				uploadedFrame.reset();
			}

			return uploadedFrame;
		}

	private:
		static Zuazo::NDI::Recv createReceiver(Zuazo::NDI::Source source, const std::string& name) {
			//Get receiver name
			auto recvIdentifier = getHostname();
			recvIdentifier += " (";
			recvIdentifier += name;
			recvIdentifier += ")";

			return Zuazo::NDI::Recv(
				source,
				Zuazo::NDI::Recv::ColorFormat::BEST, //TODO maybe choose between fastest/best
				Zuazo::NDI::Recv::Bandwidth::HIGHEST, //TODO maybe choose between lowest/highest
				false,
				recvIdentifier.c_str()
			);
		}

		static AspectRatio getPixelAspectRatio(Resolution res, float dar) {
			AspectRatio result;

			if(dar == 0.0f) {
				//If display aspect ratio is 0.0f, square pixels are assumed
				result = AspectRatio(1, 1);
			} else {
				//DAR = SAR*PAR => PAR = DAR/SAR
				result = AspectRatio(dar) / res.getAspectRatio();
			}

			return result;
		}

		static std::tuple<ColorModel, ColorPrimaries> getColorimetry(Resolution res) {
			//This has been elaborated according to the NDI SDK doc.
			std::tuple<ColorModel, ColorPrimaries> result;

			if(res.x>720 || res.y>576) {
				result = { ColorModel::BT709, ColorPrimaries::BT709 };
			} else if(res.x>1920 || res.y>1080) {
				result = { ColorModel::BT2020, ColorPrimaries::BT2020 };
			} else {
				result = { ColorModel::BT601, ColorPrimaries::BT601_625 };
			}

			return result;
		}

		static copy_fn selectCopyFunction(FourCC src, ColorFormat dst) {
			copy_fn result = nullptr;

			switch(src) {
			case FourCC::RGBA:
			case FourCC::RGBX:
				assert(dst == ColorFormat::R8G8B8A8);
				result = Zuazo::NDI::copyRGBA;
				break;

			case FourCC::BGRA:
			case FourCC::BGRX:
				assert(dst == ColorFormat::B8G8R8A8);
				result = Zuazo::NDI::copyRGBA;
				break;

			case FourCC::UYVY:
				if(dst == ColorFormat::B8G8R8G8) {
					result = Zuazo::NDI::copyUYVY;
				} else { 
					assert(dst == ColorFormat::G8_B8R8);
					result = Zuazo::NDI::copyUYVYtoNV16;
				}
				break;

			case FourCC::UYVA:
				assert(dst == ColorFormat::G8_B8R8_A8);
				result = Zuazo::NDI::copyUYVAtoPA8;
				break;

			case FourCC::P216:
				assert(dst == ColorFormat::G8_B8R8);
				result = Zuazo::NDI::copyP216;
				break;

			case FourCC::PA16:
				assert(dst == ColorFormat::G8_B8R8_A8);
				result = Zuazo::NDI::copyPA16;
				break;

			case FourCC::I420:
				assert(dst == ColorFormat::G8_B8_R8);
				result = Zuazo::NDI::copyI420;
				break;

			case FourCC::YV12:
				assert(dst == ColorFormat::G8_B8_R8);
				result = Zuazo::NDI::copyYV12toI420;
				break;

			case FourCC::NV12:
				assert(dst == ColorFormat::G8_B8R8);
				result = Zuazo::NDI::copyNV12;
				break;

			default:
				break;
			}

			assert(result);
			return result;
		}
	};

	using Output = Signal::Output<Video>;

	std::reference_wrapper<NDI>	owner;

	NDI::Source					source;
	bool						pgmTally;
	bool						pvwTally;

	Output						videoOut;
	std::unique_ptr<Open>		opened;

	NDIImpl(NDI& owner, NDI::Source source)
		: owner(owner)
		, source(std::move(source))
		, pgmTally(false)
		, pvwTally(false)
		, videoOut(std::string(Signal::makeOutputName<Video>()))
		, opened()
	{
	}

	~NDIImpl() = default;


	void moved(ZuazoBase& base) {
		owner = static_cast<NDI&>(base);
	}

	void open(ZuazoBase& base, std::unique_lock<Instance>* lock = nullptr) {
		auto& ndiSrc = static_cast<NDI&>(base);
		assert(&owner.get() == &ndiSrc);
		assert(!opened);

		//Create in a unlocked environment
		if(lock) lock->unlock();
		auto newOpened = Utils::makeUnique<Open>(
			source,
			ndiSrc.getName(),
			pgmTally, pvwTally
		);
		if(lock) lock->lock();

		//Write changes after locking back
		opened = std::move(newOpened);
		ndiSrc.enableRegularUpdate(Instance::INPUT_PRIORITY); //At this moment we do not know the rate
		videoOut.setPullCallback(std::bind(&NDIImpl::pullCallback, this));

		assert(opened);
	}

	void asyncOpen(ZuazoBase& base, std::unique_lock<Instance>& lock) {
		assert(lock.owns_lock());
		open(base, &lock);
		assert(lock.owns_lock());
	}

	void close(ZuazoBase& base, std::unique_lock<Instance>* lock = nullptr) {
		auto& ndiSrc = static_cast<NDI&>(base);
		assert(&owner.get() == &ndiSrc);
		assert(opened);
		
		//Remove all possible calls to update
		videoOut.setPullCallback(Output::PullCallback());
		ndiSrc.disablePeriodicUpdate();
		ndiSrc.disableRegularUpdate();
		ndiSrc.setVideoModeCompatibility({});

		//Write changes
		videoOut.reset();
		auto oldOpened = std::move(opened);

		//Reset in a unlocked environment
		if(oldOpened) {
			if(lock) lock->unlock();
			oldOpened.reset();
			if(lock) lock->lock();
		}

		assert(!opened);
	}

	void asyncClose(ZuazoBase& base, std::unique_lock<Instance>& lock) {
		assert(lock.owns_lock());
		close(base, &lock);
		assert(lock.owns_lock());
	}

	void update() {
		//When update is called, a new frame will be pulled from the source
		assert(opened);
		if(opened->pullFrame()) {
			//Videomode has changed. Update it
			auto& ndiSrc = owner.get();
			const auto& vulkan = ndiSrc.getInstance().getVulkan();
			ndiSrc.setVideoModeCompatibility({ opened->getSupportedVideoMode(vulkan) });
		}
	}

	void videoModeCallback(VideoBase& base, const VideoMode& videoMode) {
		auto& ndiSrc = static_cast<NDI&>(base);
		assert(&owner.get() == &ndiSrc);
		assert(opened);

		//Disable previous update config
		ndiSrc.disableRegularUpdate();
		ndiSrc.disablePeriodicUpdate();

		if(static_cast<bool>(videoMode)) {
			//The videomode is valid
			opened->recreate(ndiSrc.getInstance().getVulkan(), videoMode.getFrameDescriptor());
			ndiSrc.enablePeriodicUpdate(Instance::INPUT_PRIORITY, getPeriod(videoMode.getFrameRateValue()));
		} else {
			//Reset the uploader
			opened->recreate();
			ndiSrc.enableRegularUpdate(Instance::INPUT_PRIORITY);
		}		
	}

	void setSource(NDI::Source source) {
		this->source = std::move(source);

		if(opened) {
			opened->setSource(this->source);
		}
	}

	const NDI::Source& getSource() const noexcept {
		return source;
	}


	void setProgramTally(bool tally) {
		if(pgmTally != tally) {
			pgmTally = tally;

			if(opened) {
				opened->receiver.setTally(pgmTally, pvwTally);
			}
		}
	}

	bool getProgramTally() const noexcept {
		return pgmTally;
	}


	void setPreviewTally(bool tally) {
		if(pvwTally != tally) {
			pvwTally = tally;

			if(opened) {
				opened->receiver.setTally(pgmTally, pvwTally);
			}
		}
	}

	bool getPreviewTally() const noexcept {
		return pvwTally;
	}


private:
	void pullCallback() {
		//Only upload when needed
		assert(opened);
		videoOut.push(opened->uploadFrame());
	}

};



/*
 * NDI
 */

NDI::NDI(	Instance& instance, 
			std::string name, 
			Source source )
	: Utils::Pimpl<NDIImpl>({}, *this, std::move(source))
	, ZuazoBase(
		instance, 
		std::move(name),
		PadRef((*this)->videoOut),
		std::bind(&NDIImpl::moved, std::ref(**this), std::placeholders::_1),
		std::bind(&NDIImpl::open, std::ref(**this), std::placeholders::_1, nullptr),
		std::bind(&NDIImpl::asyncOpen, std::ref(**this), std::placeholders::_1, std::placeholders::_2),
		std::bind(&NDIImpl::close, std::ref(**this), std::placeholders::_1, nullptr),
		std::bind(&NDIImpl::asyncClose, std::ref(**this), std::placeholders::_1, std::placeholders::_2),
		std::bind(&NDIImpl::update, std::ref(**this)) )
	, VideoBase(
		std::bind(&NDIImpl::videoModeCallback, std::ref(**this), std::placeholders::_1, std::placeholders::_2) )
	, Signal::SourceLayout<Video>(makeProxy((*this)->videoOut))
{
}

NDI::NDI(NDI&& other) = default;

NDI::~NDI() = default;

NDI& NDI::operator=(NDI&& other) = default;



void NDI::setSource(Source source) {
	(*this)->setSource(std::move(source));
}

const NDI::Source& NDI::getSource() const noexcept {
	return (*this)->getSource();
}


void NDI::setProgramTally(bool tally) {
	(*this)->setProgramTally(tally);
}

bool NDI::getProgramTally() const noexcept {
	return (*this)->getProgramTally();
}


void NDI::setPreviewTally(bool tally) {
	(*this)->setPreviewTally(tally);
}

bool NDI::getPreviewTally() const noexcept {
	return (*this)->getPreviewTally();
}

}