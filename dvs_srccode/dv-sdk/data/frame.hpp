#ifndef DV_SDK_FRAME_HPP
#define DV_SDK_FRAME_HPP

#include "wrappers.hpp"

#include <dv-processing/data/frame_base.hpp>
#include <opencv2/core.hpp>
#include <opencv2/core/utility.hpp>
#include <opencv2/imgproc.hpp>

namespace dv {

template<>
class InputDataWrapper<dv::Frame> : public _InputDataWrapperCommon<dv::Frame> {
public:
	InputDataWrapper(std::shared_ptr<const dv::Frame> p) : _InputDataWrapperCommon<dv::Frame>(std::move(p)) {
	}

	/**
	 * Returns the dv frame format of the current frame
	 * @return the dv frame format of the current frame
	 */
	dv::FrameFormat format() const noexcept {
		return static_cast<dv::FrameFormat>(ptr->image.type());
	}

	/**
	 * The width of the current frame
	 * @return the width of the current frame
	 */
	int16_t sizeX() const noexcept {
		return ptr->image.cols;
	}

	/**
	 * The height of the current frame
	 * @return the height of the current frame
	 */
	int16_t sizeY() const noexcept {
		return ptr->image.rows;
	}

	/**
	 * Returns the position in x of the region of interest (ROI) of the current frame
	 * @return the x coordinate of the start of the region of interest
	 */
	int16_t positionX() const noexcept {
		return ptr->positionX;
	}

	/**
	 * Returns the position in y of the region of interest (ROI) of the current frame
	 * @return the y coordinate of the start of the region of interest
	 */
	int16_t positionY() const noexcept {
		return ptr->positionY;
	}

	/**
	 * Returns the timestamp of the start of exposure for this frame,
	 * or whatever closest value is available for this sensor.
	 * @return the timestamp of the start of exposure for this frame
	 */
	int64_t timestamp() const noexcept {
		return ptr->timestamp;
	}

	/**
	 * Returns the exposure time of this frame
	 * @return The exposure time of this frame
	 */
	dv::Duration exposure() const noexcept {
		return ptr->exposure;
	}

	/**
	 * Returns the timestamp of the start of exposure for this frame
	 * @return the timestamp of the start of exposure for this frame
	 */
	int64_t timestampStartOfExposure() const noexcept {
		return ptr->timestamp;
	}

	/**
	 * Returns the timestamp of the end of exposure for this frame
	 * @return the timestamp of the end of exposure for this frame
	 */
	int64_t timestampEndOfExposure() const noexcept {
		return (ptr->timestamp + ptr->exposure.count());
	}

	/**
	 * Returns the source (creation method) of this frame
	 * @return The source (creation method) of this frame
	 */
	dv::FrameSource source() const noexcept {
		return ptr->source;
	}

	/**
	 * Return a read-only OpenCV Mat representing this frame.
	 * It is possible to bypass the read-only restriction on a cv::Mat
	 * rather easily by dereferencing it and copy-constructing a new cv::Mat,
	 * which will point to the same data because that's how cv::Mat works.
	 * Please don't do this, it will introduce subtle and terrifying bugs.
	 *
	 * @return a read-only OpenCV Mat pointer
	 */
	[[nodiscard]] std::unique_ptr<const cv::Mat> getMatPointer() const noexcept {
		// Life-time is managed by cv::Mat internally. This trys do address const-ness primarily.
		return std::make_unique<const cv::Mat>(ptr->image);
	}

	/**
	 * Return an OpenCV Mat representing a copy of this frame.
	 * Contents are freely modifiable.
	 *
	 * @return an OpenCV Mat
	 */
	[[nodiscard]] cv::Mat getMatCopy() const noexcept {
		return ptr->image.clone();
	}

	/**
	 * Returns the size of the current frame. This is generally the same as the input
	 * dimension, but depending on the ROI setting, can be less
	 * @return The size of the current frame
	 */
	cv::Size size() const {
		return cv::Size(sizeX(), sizeY());
	}

	/**
	 * Returns the position of the ROI of the current frame.
	 * @return The position of the start of the ROI of the current frame
	 */
	cv::Point position() const {
		return cv::Point(positionX(), positionY());
	}

	/**
	 * Returns the rectangle defining the region of interest of the current frame
	 * @return The rectangle describing the region of interest of the current frame
	 */
	cv::Rect roi() const {
		return cv::Rect(position(), size());
	}
};

template<>
class OutputDataWrapper<dv::Frame> : public _OutputDataWrapperCommon<dv::Frame> {
private:
	cv::Size mMaxSize;

public:
	OutputDataWrapper(dv::Frame *p, dvModuleData m, const std::string &n, const cv::Size maxSize) :
		_OutputDataWrapperCommon<dv::Frame>(p, m, n),
		mMaxSize(maxSize) {
	}

	// Do not define * and -> operators.

	// Special copy assignment.
	OutputDataWrapper &operator=(const _InputDataWrapperCommon<dv::Frame> &rhs) {
		if (!rhs) {
			throw std::runtime_error("Frame Output copy-assign: cannot assign from empty frame.");
		}

		if ((rhs.getBasePointer()->image.cols > mMaxSize.width)
			|| (rhs.getBasePointer()->image.rows > mMaxSize.height)) {
			throw std::runtime_error("Frame Output copy-assign: input frame is bigger than maximum output frame size.");
		}

		// Copy assign content object, take special care to clone underlying cv::Mat.
		*ptr       = *rhs.getBasePointer();
		ptr->image = ptr->image.clone();

		return *this;
	}

	void commit() {
		// Ignore frames with no pixels.
		if (ptr->image.empty()) {
			return;
		}

		// Error on frames that are too big.
		if ((ptr->image.cols > mMaxSize.width) || (ptr->image.rows > mMaxSize.height)) {
			throw std::runtime_error("Frame Output commit: output frame is bigger than maximum output frame size.");
		}

		// base commit
		_OutputDataWrapperCommon<dv::Frame>::commit();
	}

	OutputDataWrapper &operator<<(dv::commitType) {
		commit();
		return *this;
	}

	/**
	 * Copies the supplied OpenCV matrix and sends it to the output.
	 * @param mat The matrix to be sent to the output
	 */
	void setMat(const cv::Mat &mat) {
		// Error on frames that are too big.
		if ((mat.cols > mMaxSize.width) || (mat.rows > mMaxSize.height)) {
			throw std::runtime_error("Output setMat: cv::Mat is bigger than maximum output frame size.");
		}

		ptr->image = mat.clone();
	}

	/**
	 * Moves the supplied OpenCV matrix and sends it to the output.
	 * Do not use it anymore afterwards!
	 * @param mat The matrix to be sent to the output
	 */
	void setMat(cv::Mat &&mat) {
		// Error on frames that are too big.
		if ((mat.cols > mMaxSize.width) || (mat.rows > mMaxSize.height)) {
			throw std::runtime_error("Output setMat: cv::Mat is bigger than maximum output frame size.");
		}

		ptr->image = mat;
	}

	/**
	 * Copies the supplied OpenCV matrix and sends it to the output.
	 * @param mat The matrix to be sent to the output
	 * @return A reference to this output, to send more data
	 */
	OutputDataWrapper<dv::Frame> &operator<<(const cv::Mat &mat) {
		setMat(mat);
		return *this;
	}

	/**
	 * Moves the supplied OpenCV matrix and sends it to the output.
	 * @param mat The matrix to be sent to the output
	 * @return A reference to this output, to send more data
	 */
	OutputDataWrapper<dv::Frame> &operator<<(cv::Mat &&mat) {
		setMat(std::move(mat));
		return *this;
	}

	/**
	 * Returns the dv frame format of the current frame
	 * @return the dv frame format of the current frame
	 */
	dv::FrameFormat format() const noexcept {
		return static_cast<dv::FrameFormat>(ptr->image.type());
	}

	/**
	 * The width of the current frame
	 * @return the width of the current frame
	 */
	int16_t sizeX() const noexcept {
		return ptr->image.cols;
	}

	/**
	 * The height of the current frame
	 * @return the height of the current frame
	 */
	int16_t sizeY() const noexcept {
		return ptr->image.rows;
	}

	/**
	 * Returns the position in x of the region of interest (ROI) of the current frame
	 * @return the x coordinate of the start of the region of interest
	 */
	int16_t positionX() const noexcept {
		return ptr->positionX;
	}

	/**
	 * Returns the position in y of the region of interest (ROI) of the current frame
	 * @return the y coordinate of the start of the region of interest
	 */
	int16_t positionY() const noexcept {
		return ptr->positionY;
	}

	/**
	 * Returns the timestamp of this frame (start of exposure or closest)
	 * @return The timestamp of this frame (start of exposure or closest)
	 */
	int64_t timestamp() const noexcept {
		return ptr->timestamp;
	}

	/**
	 * Returns the exposure time of this frame
	 * @return The exposure time of this frame
	 */
	dv::Duration exposure() const noexcept {
		return ptr->exposure;
	}

	/**
	 * Returns the timestamp of the start of exposure for this frame
	 * @return the timestamp of the start of exposure for this frame
	 */
	int64_t timestampStartOfExposure() const noexcept {
		return ptr->timestamp;
	}

	/**
	 * Returns the timestamp of the end of exposure for this frame
	 * @return the timestamp of the end of exposure for this frame
	 */
	int64_t timestampEndOfExposure() const noexcept {
		return (ptr->timestamp + ptr->exposure.count());
	}

	/**
	 * Returns the source (creation method) of this frame
	 * @return The source (creation method) of this frame
	 */
	dv::FrameSource source() const noexcept {
		return ptr->source;
	}

	/**
	 * Sets the ROI start position for the output frame
	 */
	void setPosition(int16_t positionX, int16_t positionY) noexcept {
		ptr->positionX = positionX;
		ptr->positionY = positionY;
	}

	/**
	 * Sets the timestamp of the output frame (start of exposure or closest)
	 */
	void setTimestamp(int64_t timestamp) noexcept {
		ptr->timestamp = timestamp;
	}

	/**
	 * Sets the timestamp of the current frame on the output
	 * (start of exposure or closest).
	 * @param timestamp The timestamp to be set
	 * @return A reference to this data, to commit more data
	 */
	OutputDataWrapper<dv::Frame> &operator<<(int64_t timestamp) {
		setTimestamp(timestamp);
		return *this;
	}

	/**
	 * Sets the exposure of this frame
	 */
	void setExposure(const dv::Duration exposure) noexcept {
		ptr->exposure = exposure;
	}

	/**
	 * Sets the source (creation method) of this frame
	 */
	void setSource(const dv::FrameSource source) noexcept {
		ptr->source = source;
	}

	/**
	 * Returns the size of the current frame. This is generally the same as the input
	 * dimension, but depending on the ROI setting, can be less
	 * @return The size of the current frame
	 */
	cv::Size size() const {
		return cv::Size(sizeX(), sizeY());
	}

	/**
	 * Returns the position of the ROI of the current frame.
	 * @return The position of the start of the ROI of the current frame
	 */
	cv::Point position() const {
		return cv::Point(positionX(), positionY());
	}

	/**
	 * Returns the rectangle defining the region of interest of the current frame
	 * @return The rectangle describing the region of interest of the current frame
	 */
	cv::Rect roi() const {
		return cv::Rect(position(), size());
	}

	/**
	 * Sets the position of the ROI of the output frame.
	 */
	void setPosition(const cv::Point &position) noexcept {
		setPosition(static_cast<int16_t>(position.x), static_cast<int16_t>(position.y));
	}
};

template<>
class RuntimeInput<dv::Frame> : public _RuntimeInputCommon<dv::Frame> {
public:
	RuntimeInput(const std::string &name, dvModuleData moduleData) : _RuntimeInputCommon<dv::Frame>(name, moduleData) {
	}

	const InputDataWrapper<dv::Frame> frame() const {
		return (data());
	}

	int sizeX() const {
		return infoNode().getInt("sizeX");
	}

	int sizeY() const {
		return infoNode().getInt("sizeY");
	}

	const cv::Size size() const {
		return cv::Size(sizeX(), sizeY());
	}
};

/**
 * Specialization of the runtime output for frame outputs
 * Provides convenience setup functions for setting up the frame output
 */
template<>
class RuntimeOutput<dv::Frame> : public _RuntimeOutputCommon<dv::Frame> {
public:
	RuntimeOutput(const std::string &name, dvModuleData moduleData) :
		_RuntimeOutputCommon<dv::Frame>(name, moduleData) {
	}

	/**
	 * Returns a writeable output wrapper for the given type of this output.
	 * Allocates new output memory if necessary. The output can be committed
	 * by calling commit on the returned object.
	 * @return A wrapper to allocated output memory to write to
	 */
	OutputDataWrapper<dv::Frame> data() {
		OutputDataWrapper<dv::Frame> wrapper{allocateUnwrapped(), moduleData_, name_, size()};
		return wrapper;
	}

	/**
	 * Returns the current output frame to set up values and to get the back data buffer
	 * @return The output frame to set up
	 */
	OutputDataWrapper<dv::Frame> frame() {
		return data();
	}

	// Hide setup without size.
	void setup(const std::string &originDescription) = delete;

	/**
	 * Sets up this frame output with the provided parameters
	 * @param sizeX The width of the frames supplied on this output
	 * @param sizeY The height of the frames supplied on this output
	 * @param originDescription A description of the original creator of the data
	 */
	void setup(int sizeX, int sizeY, const std::string &originDescription) {
		createSourceAttribute(originDescription);
		createSizeAttributes(sizeX, sizeY);
	}

	/**
	 * Sets this frame output up with the same parameters the the supplied input.
	 * @param input An input to copy the data from
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
	 * Sets this frame output up with the same parameters as the supplied vector input.
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

	int sizeX() const {
		return infoNode().getInt("sizeX");
	}

	int sizeY() const {
		return infoNode().getInt("sizeY");
	}

	const cv::Size size() const {
		return cv::Size(sizeX(), sizeY());
	}

	/**
	 * Convenience shortcut to commit the data on this output directly
	 */
	void operator<<(dv::commitType) {
		data() << dv::commit;
	}

	/**
	 * Sets the timestamp of the current frame on the output
	 * (start of exposure or closest).
	 * @param timestamp The timestamp to be set
	 * @return A reference to this output, to commit more data
	 */
	RuntimeOutput<dv::Frame> &operator<<(int64_t timestamp) {
		data() << timestamp;
		return *this;
	}

	/**
	 * Convenience shorthand to commit an OpenCV mat onto this output.
	 * If not using this function, call `data()` to get an output frame
	 * to fill into.
	 * @param mat The OpenCV Mat to submit
	 * @return A reference to the this
	 */
	RuntimeOutput<dv::Frame> &operator<<(const cv::Mat &mat) {
		data() << mat;
		return *this;
	}

	/**
	 * Convenience shorthand to commit an OpenCV mat onto this output.
	 * If not using this function, call `data()` to get an output frame
	 * to fill into.
	 * @param mat The OpenCV Mat to submit
	 * @return A reference to the this
	 */
	RuntimeOutput<dv::Frame> &operator<<(cv::Mat &&mat) {
		data() << std::move(mat);
		return *this;
	}
};

} // namespace dv

#endif // DV_SDK_FRAME_HPP
