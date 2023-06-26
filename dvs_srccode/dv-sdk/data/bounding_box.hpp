#ifndef DV_SDK_BOUNDING_BOX_HPP
#define DV_SDK_BOUNDING_BOX_HPP

#include "wrappers.hpp"

#include <dv-processing/data/bounding_box_base.hpp>
#include <opencv2/core.hpp>
#include <opencv2/core/utility.hpp>

namespace dv {

template<>
class InputVectorDataWrapper<dv::BoundingBoxPacket, dv::BoundingBox> :
	public _InputVectorDataWrapperCommon<dv::BoundingBoxPacket, dv::BoundingBox> {
public:
	InputVectorDataWrapper(std::shared_ptr<const dv::BoundingBoxPacket> p) :
		_InputVectorDataWrapperCommon<dv::BoundingBoxPacket, dv::BoundingBox>(std::move(p)) {
	}
};

template<>
class OutputVectorDataWrapper<dv::BoundingBoxPacket, dv::BoundingBox> :
	public _OutputVectorDataWrapperCommon<dv::BoundingBoxPacket, dv::BoundingBox> {
public:
	OutputVectorDataWrapper(dv::BoundingBoxPacket *p, dvModuleData m, const std::string &n) :
		_OutputVectorDataWrapperCommon<dv::BoundingBoxPacket, dv::BoundingBox>(p, m, n) {
	}

	// Un-hide copy assignment.
	using _OutputDataWrapperCommon<dv::BoundingBoxPacket>::operator=;
};

/**
 * Describes an input for bounding boxes. Offers convenience functions to obtain informations
 * about the bounding box sizes as well as to get the data.
 */
template<>
class RuntimeVectorInput<dv::BoundingBoxPacket, dv::BoundingBox> :
	public _RuntimeVectorInputCommon<dv::BoundingBoxPacket, dv::BoundingBox> {
public:
	RuntimeVectorInput(const std::string &name, dvModuleData moduleData) :
		_RuntimeVectorInputCommon<dv::BoundingBoxPacket, dv::BoundingBox>(name, moduleData) {
	}

	/**
	 * @return The width of the input region in pixels. Any event on this input will have a x-coordinate
	 * smaller than the return value of this function.
	 */
	int sizeX() const {
		return (infoNode().getInt("sizeX"));
	}

	/**
	 * @return The height of the input region in pixels. Any event on this input will have a y-coordinate
	 * smaller than the return value of this function
	 */
	int sizeY() const {
		return (infoNode().getInt("sizeY"));
	}

	/**
	 * @return the input region size in pixels as an OpenCV size object
	 */
	const cv::Size size() const {
		return (cv::Size(sizeX(), sizeY()));
	}
};

/**
 * Specialization of the runtime output for bounding boxes.
 * Provides convenience setup functions for setting up the bounding boxes output.
 */
template<>
class RuntimeVectorOutput<dv::BoundingBoxPacket, dv::BoundingBox> :
	public _RuntimeVectorOutputCommon<dv::BoundingBoxPacket, dv::BoundingBox> {
public:
	RuntimeVectorOutput(const std::string &name, dvModuleData moduleData) :
		_RuntimeVectorOutputCommon<dv::BoundingBoxPacket, dv::BoundingBox>(name, moduleData) {
	}

	// Hide setup without size.
	void setup(const std::string &originDescription) = delete;

	/**
	 * Sets up this bounding boxes output by setting the provided arguments to the output info node
	 * @param sizeX The width of this output
	 * @param sizeY The height of this output
	 * @param originDescription A description that describes the original generator of the data
	 */
	void setup(int sizeX, int sizeY, const std::string &originDescription) {
		createSourceAttribute(originDescription);
		createSizeAttributes(sizeX, sizeY);
	}

	/**
	 * Sets this bounding box output up with the same parameters as the supplied input.
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
	 * Sets this bounding box output up with the same parameters as the supplied vector input.
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
		return (infoNode().getInt("sizeX"));
	}

	/**
	 * @return The height of the input region in pixels. Any event on this input will have a y-coordinate
	 * smaller than the return value of this function
	 */
	int sizeY() const {
		return (infoNode().getInt("sizeY"));
	}

	/**
	 * @return the input region size in pixels as an OpenCV size object
	 */
	const cv::Size size() const {
		return (cv::Size(sizeX(), sizeY()));
	}
};

} // namespace dv

#endif // DV_SDK_BOUNDING_BOX_HPP
