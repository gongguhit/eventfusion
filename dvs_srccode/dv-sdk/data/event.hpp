#ifndef DV_SDK_EVENT_HPP
#define DV_SDK_EVENT_HPP

#include "wrappers.hpp"

#include <dv-processing/core/core.hpp>
#include <dv-processing/data/event_base.hpp>
#include <opencv2/core.hpp>
#include <opencv2/core/utility.hpp>

namespace dv {

/**
 * The EventColor enum contains the
 * color of the Bayer color filter for a
 * specific event address.
 * WHITE means White/No Filter.
 * Please take into account that there are
 * usually twice as many green pixels as
 * there are red or blue ones.
 */
enum class EventColor : int8_t {
	WHITE = 0,
	RED   = 1,
	GREEN = 2,
	BLUE  = 3,
};

/**
 * Address to Color mapping for events based on Bayer filter.
 */
static constexpr EventColor colorKeys[4][4] = {
	{EventColor::RED,   EventColor::GREEN, EventColor::GREEN, EventColor::BLUE },
	{EventColor::GREEN, EventColor::BLUE,  EventColor::RED,   EventColor::GREEN},
	{EventColor::GREEN, EventColor::RED,   EventColor::BLUE,  EventColor::GREEN},
	{EventColor::BLUE,  EventColor::GREEN, EventColor::GREEN, EventColor::RED  },
};

template<>
class InputVectorDataWrapper<dv::EventPacket, dv::Event> :
	public _InputVectorDataWrapperCommon<dv::EventPacket, dv::Event> {
public:
	InputVectorDataWrapper(std::shared_ptr<const dv::EventPacket> p) :
		_InputVectorDataWrapperCommon<dv::EventPacket, dv::Event>(std::move(p)) {
	}

	/**
	 * Conversion operator for dv::EventStore.
	 * Provides backwards compatibility, due to dv::EventStore
	 * having stricter, explicit constructors now.
	 *
	 * @return an EventStore sharing this input EventPacket.
	 */
	operator dv::EventStore() const noexcept {
		return dv::EventStore{ptr};
	}
};

template<>
class OutputVectorDataWrapper<dv::EventPacket, dv::Event> :
	public _OutputVectorDataWrapperCommon<dv::EventPacket, dv::Event> {
private:
	cv::Size mMaxSize;

public:
	OutputVectorDataWrapper(dv::EventPacket *p, dvModuleData m, const std::string &n, const cv::Size maxSize) :
		_OutputVectorDataWrapperCommon<dv::EventPacket, dv::Event>(p, m, n),
		mMaxSize(maxSize) {
	}

	// Do not define * and -> operators.

	// Special copy assignment.
	OutputVectorDataWrapper &operator=(const _InputDataWrapperCommon<dv::EventPacket> &rhs) {
		if (!rhs) {
			throw std::runtime_error("Event Output copy-assign: cannot assign from empty event packet.");
		}

		if (dv::DEBUG_ENABLED) {
			for (const auto &e : rhs.getBasePointer()->elements) {
				// Error on events that are out of place.
				if ((e.x() < 0) || (e.x() >= mMaxSize.width) || (e.y() < 0) || (e.y() >= mMaxSize.height)) {
					throw std::runtime_error("Event Output copy-assign: event has invalid coordinates.");
				}
			}
		}

		// Copy assign content object.
		*ptr = *rhs.getBasePointer();

		return *this;
	}

	void commit() {
		if (dv::DEBUG_ENABLED) {
			for (const auto &e : ptr->elements) {
				// Error on events that are out of place.
				if ((e.x() < 0) || (e.x() >= mMaxSize.width) || (e.y() < 0) || (e.y() >= mMaxSize.height)) {
					throw std::runtime_error("Event Output commit: event has invalid coordinates.");
				}
			}
		}

		// base commit
		_OutputVectorDataWrapperCommon<dv::EventPacket, dv::Event>::commit();
	}

	using _OutputVectorDataWrapperCommon<dv::EventPacket, dv::Event>::operator<<;

	OutputVectorDataWrapper &operator<<(dv::commitType) {
		commit();
		return *this;
	}
};

/**
 * Describes an input for event packets. Offers convenience functions to obtain informations
 * about the event input as well as to get the event data.
 */
template<>
class RuntimeVectorInput<dv::EventPacket, dv::Event> : public _RuntimeVectorInputCommon<dv::EventPacket, dv::Event> {
public:
	RuntimeVectorInput(const std::string &name, dvModuleData moduleData) :
		_RuntimeVectorInputCommon<dv::EventPacket, dv::Event>(name, moduleData) {
		// Can only get infoNode() and color info if connected.
		if (!isConnected()) {
			return;
		}

		// Initialize color filter information, cache it.
		const auto iNode = infoNode();

		if (iNode.exists<dv::CfgType::INT>("colorFilter")) {
			switch (iNode.getInt("colorFilter")) {
				case 0: // RGBG
					colorIndex = 0;
					break;

				case 1: // GRGB
					colorIndex = 1;
					break;

				case 2: // GBGR
					colorIndex = 2;
					break;

				case 3: // BGRG
					colorIndex = 3;
					break;

				default: // Unknown.
					break;
			}
		}
	}

	/**
	 * Returns an iterable container of the latest events that arrived at this input.
	 * @return An iterable container of the newest events.
	 */
	const InputVectorDataWrapper<dv::EventPacket, dv::Event> events() const {
		return data();
	}

	/**
	 * @return The width of the input region in pixels. Any event on this input will have a x-coordinate
	 * smaller than the return value of this function.
	 */
	int sizeX() const {
		return infoNode().getInt("sizeX");
	}

	/**
	 * @return The height of the input region in pixels. Any event on this input will have a y-coordinate
	 * smaller than the return value of this function
	 */
	int sizeY() const {
		return infoNode().getInt("sizeY");
	}

	/**
	 * Determine the color of the Bayer color filter for a
	 * specific event, based on its address.
	 * WHITE means White/No Filter.
	 * Please take into account that there are
	 * usually twice as many green pixels as
	 * there are red or blue ones.
	 *
	 * @param evt event to determine filter color for.
	 *
	 * @return filter color.
	 */
	dv::EventColor colorForEvent(const dv::Event &evt) const {
		if (colorIndex >= 0) {
			const auto x   = static_cast<uint32_t>(evt.x());
			const auto y   = static_cast<uint32_t>(evt.y());
			const auto idx = static_cast<size_t>(((x & 0x01) << 1) | (y & 0x01));

			return colorKeys[colorIndex][idx];
		}

		// No Filter.
		return dv::EventColor::WHITE;
	}

	/**
	 * @return the input region size in pixels as an OpenCV size object
	 */
	const cv::Size size() const {
		return cv::Size(sizeX(), sizeY());
	}

private:
	// Color filter information cache: negative is no filter, positive is lookup index.
	int colorIndex = -1;
};

/**
 * Specialization of the runtime output for event outputs.
 * Provides convenience setup functions for setting up the event output
 */
template<>
class RuntimeVectorOutput<dv::EventPacket, dv::Event> : public _RuntimeVectorOutputCommon<dv::EventPacket, dv::Event> {
public:
	RuntimeVectorOutput(const std::string &name, dvModuleData moduleData) :
		_RuntimeVectorOutputCommon<dv::EventPacket, dv::Event>(name, moduleData) {
	}

	/**
	 * Returns a writeable output wrapper for the given type of this output.
	 * Allocates new output memory if necessary. The output can be committed
	 * by calling commit on the returned object.
	 * @return A wrapper to allocated output memory to write to
	 */
	OutputVectorDataWrapper<dv::EventPacket, dv::Event> data() {
		OutputVectorDataWrapper<dv::EventPacket, dv::Event> wrapper{allocateUnwrapped(), moduleData_, name_, size()};
		return wrapper;
	}

	OutputVectorDataWrapper<dv::EventPacket, dv::Event> events() {
		return data();
	}

	// Hide setup without size.
	void setup(const std::string &originDescription) = delete;

	/**
	 * Sets up this event output by setting the provided arguments to the output info node
	 * @param sizeX The width of this event output
	 * @param sizeY The height of this event output
	 * @param originDescription A description that describes the original generator of the data
	 */
	void setup(int sizeX, int sizeY, const std::string &originDescription) {
		createSourceAttribute(originDescription);
		createSizeAttributes(sizeX, sizeY);
	}

	/**
	 * Sets this event output up with the same parameters as the supplied input.
	 * @param input An input to copy the information from
	 */
	template<typename U>
	void setup(const RuntimeInput<U> &input) {
		input.infoNode().copyTo(infoNode());

		// Check that required attributes exist at least.
		getOriginDescription();
		sizeX();
		sizeY();
	}

	/**
	 * Sets this event output up with the same parameters as the supplied vector input.
	 * @param input A vector input to copy the information from
	 */
	template<typename U, typename TT>
	void setup(const RuntimeVectorInput<U, TT> &input) {
		input.infoNode().copyTo(infoNode());

		// Check that required attributes exist at least.
		getOriginDescription();
		sizeX();
		sizeY();
	}

	/**
	 * @return The width of the input region in pixels. Any event on this input will have a x-coordinate
	 * smaller than the return value of this function.
	 */
	int sizeX() const {
		return infoNode().getInt("sizeX");
	}

	/**
	 * @return The height of the input region in pixels. Any event on this input will have a y-coordinate
	 * smaller than the return value of this function
	 */
	int sizeY() const {
		return infoNode().getInt("sizeY");
	}

	/**
	 * @return the input region size in pixels as an OpenCV size object
	 */
	const cv::Size size() const {
		return cv::Size(sizeX(), sizeY());
	}

	/**
	 * Convenience shortcut to commit the data on this output directly
	 * This gets resolved to one of the various << functions
	 * of the data wrapper.
	 */
	template<typename K>
	RuntimeVectorOutput<dv::EventPacket, dv::Event> &operator<<(const K &rhs) {
		data() << rhs;
		return *this;
	}

	/**
	 * Convenience shortcut to commit the data on this output directly
	 */
	void operator<<(commitType) {
		data() << dv::commit;
	}
};

} // namespace dv

#endif // DV_SDK_EVENT_HPP
