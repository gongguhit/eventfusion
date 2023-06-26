#pragma once

#include "../data/frame_base.hpp"
#include "../data/imu_base.hpp"
#include "../data/trigger_base.hpp"
#include "../exception/exceptions/generic_exceptions.hpp"
#include "core.hpp"
#include "stream_slicer.hpp"

#include <unordered_map>
#include <variant>

namespace dv {

/**
 * Time handling approaches for number based slicing.
 */
enum class TimeSlicingApproach {
	/// Assign gap elements between previous numeric slice and current one.
	BACKWARD,
	/// Assign gap elements between current numeric slice and next one.
	FORWARD
};

/**
 * MultiStreamSlicer takes multiple streams of timestamped data, slices data with configured intervals
 * and calls a given callback method on each interval. It is an extension of StreamSlicer class that can
 * synchronously slice multiple streams. Each stream has to be named uniquely, the name is carried over
 * to the callback method to identify each stream.
 *
 * The class relies heavily on templating, so it supports different containers of data, as long as the
 * container is an iterable and each element contains an accessible timestamp in microsecond format.
 *
 * The slicing is driven by the main stream, which needs to be specified during construction time. The type of the
 * main stream is the first template argument and the name for the main stream is provided as the constructor's
 * first argument.
 *
 * By default, these types are supported without additional configuration: dv::EventStore, dv::EventPacket,
 * dv::TriggerPacket, dv::cvector<dv::Trigger>, dv::IMUPacket, dv::cvector<dv::IMU>, dv::cvector<dv::Frame>.
 * Additional types can be supported by specifying them as additional template parameters.
 *
 * @tparam MainStreamType The type of the main stream.
 * @tparam AdditionalTypes Parameter pack to specify an arbitrary number of additional stream types to be supported.
 */
template<class MainStreamType, class... AdditionalTypes>
requires((concepts::CompatibleWithSlicer<MainStreamType>) && (concepts::CompatibleWithSlicer<AdditionalTypes> && ...))
class MultiStreamSlicer {
private:
	// MainType is either a `std::monostate` or the given main-stream type. This is needed to reduce usage complexity -
	// if the main-stream type is already one of the existing types, it shouldn't be duplicated in the variant list.
	// If it is not, it has to be added to the list of variants. `std::variant` specifically states that type `void`
	// can't be used, so in the case when main stream type is a duplicate, a `std::monostate` is added, which is
	// just a placeholder to be ignored. Otherwise, the main type will be added to the variant list.
	using MainType = typename std::conditional_t<
		dv::concepts::is_type_one_of<MainStreamType, dv::EventStore, dv::EventPacket, dv::IMUPacket, dv::TriggerPacket,
			dv::cvector<dv::Frame>, dv::cvector<dv::IMU>, dv::cvector<dv::Trigger>, AdditionalTypes...>,
		std::monostate, MainStreamType>;

public:
	/// Alias for the variant that holds a packet type
	using InputType = std::variant<MainType, dv::EventStore, dv::EventPacket, dv::IMUPacket, dv::TriggerPacket,
		dv::cvector<dv::Frame>, dv::cvector<dv::IMU>, dv::cvector<dv::Trigger>, AdditionalTypes...>;

	/**
	 * Initialize the multi-stream slicer, provide the type of the main stream and a name for the main stream.
	 * The slicing is performed by applying a typical slicer on the main stream, all other stream follow it. When a
	 * window of slicing executes, the slicer extracts according data from all the other streams and calls
	 * a registered callback method for data processing.
	 *
	 * Main stream is used to evaluate the jobs, but it also waits for the other types of data to arrive.
	 * The callbacks are not executed until all data has arrived on all streams.
	 *
	 * By default, these types are supported without additional configuration: dv::EventStore, dv::EventPacket,
	 * dv::TriggerPacket, dv::cvector<dv::Trigger>, dv::IMUPacket, dv::cvector<dv::IMU>, dv::cvector<dv::Frame>.
	 * Additional types can be supported by specifying them as additional template parameters.
	 * @param mainStreamName Name of the main stream.
	 */
	explicit MultiStreamSlicer(std::string mainStreamName) : mMainStreamName(std::move(mainStreamName)) {
		mBuffer[MultiStreamSlicer::mMainStreamName]         = MainStreamType();
		mSeekTimestamps[MultiStreamSlicer::mMainStreamName] = -1LL;
	}

	/**
	 * Class that is passed to the slicer callback. It is an unordered map where key is the configured
	 * stream name and the value is a variant. The class provides convenience methods to access and cast
	 * the types.
	 */
	class MapOfVariants : public std::unordered_map<std::string, InputType> {
	public:
		/**
		 * Get a reference to the data packet of a given stream name.
		 * @tparam Type Type of data for the stream.
		 * @param streamName Stream name.
		 * @return Data packet casted to the given type.
		 */
		template<class Type>
		Type &get(const std::string &streamName) {
			return std::get<Type>(this->at(streamName));
		}

		/**
		 * Get a reference to the data packet of a given stream name.
		 * @tparam Type Type of data for the stream.
		 * @param streamName Stream name.
		 * @return Data packet casted to the given type.
		 */
		template<class Type>
		const Type &get(const std::string &streamName) const {
			return std::get<Type>(this->at(streamName));
		}
	};

	/**
	 * Add a stream to the slicer.
	 * @tparam DataType Data packet type of the stream.
	 * @param streamName Name for the stream.
	 */
	template<class DataType>
	requires dv::concepts::is_type_one_of<DataType, MainType, dv::EventStore, dv::EventPacket, dv::IMUPacket,
		dv::TriggerPacket, dv::cvector<dv::Frame>, dv::cvector<dv::IMU>, dv::cvector<dv::Trigger>, AdditionalTypes...>
	void addStream(const std::string &streamName) {
		mBuffer[streamName]         = DataType();
		mSeekTimestamps[streamName] = -1LL;
	}

	/**
	 * Accept incoming data for a stream and evaluate processing jobs. Can be either a packet or a single
	 * timestamped element of the stream.
	 * @param streamName Name of the stream.
	 * @param data Incoming data, either a data packet or timestamp data element.
	 * @throws RuntimeError Exception is thrown if passed data type does not match the stream data type.
	 */
	template<class DataType>
	requires dv::concepts::CompatibleWithSlicer<DataType> || dv::concepts::Timestamped<DataType>
	void accept(const std::string &streamName, const DataType &data) {
		// Handle datat incoming as a packet
		if constexpr (dv::concepts::CompatibleWithSlicer<DataType>) {
			try {
				mergePackets(data, std::get<DataType>(mBuffer.at(streamName)));
			}
			catch (const std::bad_variant_access &_) {
				// Rethrow the bad variant access with a more readable exception.
				throw dv::exceptions::RuntimeError(
					fmt::format("Invalid packet type supplied for stream [{}]", streamName));
			}
		}
		// Handle data incoming as single elements
		else if constexpr (dv::concepts::Timestamped<DataType>) {
			try {
				// Push the data element into a respective stream buffer. The visit method will try to match
				// element type to the buffer type, if the type does not match, it will throw a bad variant access
				// exception.
				std::visit(
					[&data](auto &&streamBuffer) {
						// This check is needed to avoid compiler confusion with the `std::monostate` type placeholder.
						if constexpr (std::is_same_v<std::remove_cvref_t<decltype(streamBuffer)>, std::monostate>) {
							return;
						}
						// Next checks are needed to differentiate data-packets which have actual
						// data storage in the `elements` member, or they are directly iterable containers.
						else if constexpr (dv::concepts::DataPacket<decltype(streamBuffer)>) {
							if constexpr (std::is_same_v<DataType,
											  std::remove_cvref_t<dv::concepts::iterable_element_type<
												  decltype(streamBuffer.elements)>>>) {
								streamBuffer.elements.push_back(data);
							}
						}
						else if constexpr (dv::concepts::TimestampedIterable<decltype(streamBuffer)>) {
							if constexpr (std::is_same_v<DataType,
											  std::remove_cvref_t<
												  dv::concepts::iterable_element_type<decltype(streamBuffer)>>>) {
								streamBuffer.push_back(data);
							}
						}
					},
					mBuffer.at(streamName));
			}
			catch (const std::bad_variant_access &_) {
				// Rethrow the bad variant access with a more readable exception.
				throw dv::exceptions::RuntimeError(
					fmt::format("Invalid packet type supplied for stream [{}]", streamName));
			}
		}

		evaluate();
	}

	/**
	 * Register a callback to be performed at a given interval. Data is passed as an argument to the method.
	 * Callback method passes TimeWindow parameter along the data for the callback to be aware of time slicing
	 * windows.
	 * @param interval Interval at which the callback has to be executed.
	 * @param callback Callback method that is called at the given interval, receives time window information
	 * and sliced data.
	 * @return An id that can be used to modify this job.
	 */
	int doEveryTimeInterval(
		const dv::Duration interval, std::function<void(const dv::TimeWindow &, const MapOfVariants &)> callback) {
		auto internalCallback = [&, callback](const dv::TimeWindow &time, const MainStreamType &packet) {
			MapOfVariants data;
			data.insert(std::make_pair(mMainStreamName, packet));

			for (const auto &[stream, streamBuffer] : mBuffer) {
				if (stream == mMainStreamName) {
					continue;
				}

				data[stream] = slicePacket(time.startTime, time.endTime, streamBuffer);
			}

			callback(time, data);

			mSeekTime = time.endTime;
		};

		const int64_t duration = interval.count();

		mRetainDuration = std::max(mRetainDuration, duration);

		const int index = mMainSlicer.doEveryTimeInterval(interval, internalCallback);

		mConfig.emplace(std::make_pair(index, SliceJob(duration, callback)));
		return index;
	}

	/**
	 * Register a callback to be performed at a given interval. Data is passed as an argument to the method.
	 * @param interval Interval at which the callback has to be executed.
	 * @param callback Callback method that is called at the given interval.
	 * @return An id that can be used to modify this job.
	 */
	int doEveryTimeInterval(const dv::Duration interval, std::function<void(const MapOfVariants &)> callback) {
		const auto callbackWrapper = [callback](const dv::TimeWindow &, const MapOfVariants &data) {
			callback(data);
		};
		return doEveryTimeInterval(interval, callbackWrapper);
	}

	/**
	 * Adds a number-of-elements triggered job to the Slicer. A job is defined
	 * by its interval and callback function. The slicer calls the callback
	 * function every time `n` elements are added to the stream buffer, with the corresponding data.
	 * The (cpu) time interval between individual calls to the function depends
	 * on the physical event rate as well as the bulk sizes of the incoming
	 * data.
	 *
	 * Parameter timeSlicingApproach - is an enum that defines timing approach for multi-stream slicing by number.
	 * The slicing by number happens by slicing the main stream by a given number of elements. Secondary streams are
	 * sliced by the time window of the numbered slice, this introduces a problem of gaps between two number slices -
	 * the gap  values can either be assigned to current or the next slice, this enum allows to control which of the
	 * data parts these gap data will be assigned - backwards will assign all gap data from previous slice end time
	 * to current slice start time to current, the forwards approach will assign the gap data from current slice
	 * end time to next slice start time to the current slice. The forwards slice timing will result in processing
	 * delay of exactly one slice, as it requires to wait for the next slice to happen to correctly retrieve next slice
	 * start time. Backwards slicing does not wait for any additional data and processes everything immediately.
	 *
	 * @param n the interval (in number of elements) in which the callback should be called.
	 * @param callback the callback function that gets called on the data every interval.
	 * @param timeSlicingApproach Select approach for handling secondary stream gap data.
	 * @return A handle to uniquely identify the job.
	 */
	int doEveryNumberOfElements(const size_t n,
		std::function<void(const dv::TimeWindow &, const MapOfVariants &)> callback,
		const TimeSlicingApproach timeSlicingApproach = TimeSlicingApproach::BACKWARD) {
		mRetainSize = std::max(mRetainSize, n);

		std::function<void(const MainStreamType &)> internalCallback;

		if (timeSlicingApproach == TimeSlicingApproach::BACKWARD) {
			internalCallback = [&, callback, lastTime = -1LL](const MainStreamType &packet) mutable {
				dv::runtime_assert(!isPacketEmpty(packet),
					"Number based slicing received an empty packet, this should never "
					"happen! Please report a bug.");

				MapOfVariants data;
				data.insert(std::make_pair(mMainStreamName, packet));

				const auto timeWindow = getPacketTimeWindow(packet);

				for (const auto &[stream, streamBuffer] : mBuffer) {
					if (stream == mMainStreamName) {
						continue;
					}

					data[stream] = slicePacket(lastTime + 1, timeWindow.endTime + 1, streamBuffer);
				}

				lastTime = timeWindow.endTime;

				callback(timeWindow, data);
			};
		}
		else {
			// Forward slicing, delayed by one slice
			internalCallback = [&, callback, lastSlice = MainStreamType()](const MainStreamType &packet) mutable {
				dv::runtime_assert(!isPacketEmpty(packet),
					"Number based slicing received an empty packet, this should never "
					"happen! Please report a bug.");

				if (isPacketEmpty(lastSlice)) {
					lastSlice = packet;
					return;
				}

				MapOfVariants data;
				data.insert(std::make_pair(mMainStreamName, lastSlice));

				auto timeWindowLast    = getPacketTimeWindow(lastSlice);
				auto timeWindowCurrent = getPacketTimeWindow(packet);

				for (const auto &[stream, streamBuffer] : mBuffer) {
					if (stream == mMainStreamName) {
						continue;
					}

					data[stream] = slicePacket(timeWindowLast.startTime, timeWindowCurrent.startTime, streamBuffer);
				}

				lastSlice = packet;

				callback(dv::TimeWindow(timeWindowLast.startTime, timeWindowCurrent.startTime), data);
			};
		}

		int index = mMainSlicer.doEveryNumberOfElements(n, internalCallback);

		mConfig.emplace(std::make_pair(index, SliceJob(n, timeSlicingApproach, callback)));

		return index;
	}

	/**
	 * Adds a number-of-elements triggered job to the Slicer. A job is defined
	 * by its interval and callback function. The slicer calls the callback
	 * function every time `n` elements are added to the stream buffer, with the corresponding data.
	 * The (cpu) time interval between individual calls to the function depends
	 * on the physical event rate as well as the bulk sizes of the incoming
	 * data.
	 *
	 * Parameter timeSlicingApproach - is an enum that defines timing approach for multi-stream slicing by number.
	 * The slicing by number happens by slicing the main stream by a given number of elements. Secondary streams are
	 * sliced by the time window of the numbered slice, this introduces a problem of gaps between two number slices -
	 * the gap  values can either be assigned to current or the next slice, this enum allows to control which of the
	 * data parts these gap data will be assigned - backwards will assign all gap data from previous slice end time
	 * to current slice start time to current, the forwards approach will assign the gap data from current slice
	 * end time to next slice start time to the current slice. The forwards slice timing will result in processing
	 * delay of exactly one slice, as it requires to wait for the next slice to happen to correctly retrieve next slice
	 * start time. Backwards slicing does not wait for any additional data and processes everything immediately.
	 *
	 * @param n the interval (in number of elements) in which the callback should be called.
	 * @param callback the callback function that gets called on the data every interval.
	 * @param timeSlicingApproach Select approach for handling secondary stream gap data.
	 * @return A handle to uniquely identify the job.
	 */
	int doEveryNumberOfElements(const size_t n, std::function<void(const MapOfVariants &)> callback,
		const TimeSlicingApproach timeSlicingApproach = TimeSlicingApproach::BACKWARD) {
		const auto callbackWrapper = [callback](const dv::TimeWindow &, const MapOfVariants &data) {
			callback(data);
		};
		return doEveryNumberOfElements(n, callbackWrapper, timeSlicingApproach);
	}

	/**
	 * Modify the execution interval of a job.
	 * @param jobId Callback id that is received from callback registration.
	 * @param timeInterval New time interval to be executed.
	 * @throws invalid_argument Exception is thrown if trying to modify a number based slicing job.
	 */
	void modifyTimeInterval(const int jobId, const dv::Duration timeInterval) {
		if (!hasJob(jobId)) {
			return;
		}

		mMainSlicer.modifyTimeInterval(jobId, timeInterval);
		mConfig.at(jobId).mInterval = timeInterval.count();

		int64_t maxInterval = -1;
		for (const auto &[_, job] : mConfig) {
			if (job.mType == SliceJob::SliceType::TIME && job.mInterval > maxInterval) {
				maxInterval = job.mInterval;
			}
		}
		if (maxInterval > -1) {
			mRetainDuration = maxInterval;
		}
	}

	/**
	 * Modify the execution number of elements of a job.
	 * @param jobId Job id that is received from callback registration.
	 * @param n New number of elements to slice for the given job id.
	 * @throws invalid_argument Exception is thrown if trying to modify a time based slicing job.
	 */
	void modifyNumberInterval(const int jobId, const size_t n) {
		if (!hasJob(jobId)) {
			return;
		}

		mMainSlicer.modifyNumberInterval(jobId, n);
		mConfig.at(jobId).mNumberOfElements = n;

		size_t maxNumOfElements = 0;
		for (const auto &[_, job] : mConfig) {
			if (job.mType == SliceJob::SliceType::NUMBER && job.mNumberOfElements > maxNumOfElements) {
				maxNumOfElements = job.mInterval;
			}
		}
		if (maxNumOfElements > 0) {
			mRetainSize = maxNumOfElements;
		}
	}

	/**
	 * Returns true if the slicer contains the slice-job with the provided id
	 * @param jobId the id of the slice-job in question
	 * @return true, if the slicer contains the given slice-job
	 */
	[[nodiscard]] bool hasJob(const int jobId) const {
		return mConfig.contains(jobId);
	}

	/**
	 * Removes the given job from the list of current jobs.
	 * @param jobId The job id to be removed
	 */
	void removeJob(const int jobId) {
		if (!hasJob(jobId)) {
			return;
		}
		mConfig.erase(jobId);
		mMainSlicer.removeJob(jobId);
	}

	/**
	 * Update a stream's seek time manually and evaluate jobs.
	 *
	 * Data synchronization is automatically inferred from received data. This works well with data streams
	 * that produce data at guaranteed periodic intervals. For aperiodic data streams, which produce data
	 * spontaneously, a manual synchronization is required. This method allows to manually instruct the slicer
	 * that the given stream has provided data up to, but not including, this given seek timestamp; even in case when
	 * there was no data. Slicer is then able to progress other streams until the given time, since it assumes no data
	 * will ever arrive for this stream until this point. Be sure to call this method when you are sure no data will
	 * arrive, otherwise that data can be lost.
	 *
	 * @param streamName Name of the stream.
	 * @param seekTimestamp Seek time for this stream; all data until this time has been provided to the slicer.
	 */
	void setStreamSeekTime(const std::string &streamName, const int64_t seekTimestamp) {
		mSeekTimestamps[streamName] = seekTimestamp;
		evaluate();
	}

protected:
	/**
	 * Internal container of slice jobs.
	 */
	class SliceJob {
	public:
		enum class SliceType {
			NUMBER,
			TIME
		};

		/// Callback method signature alias
		using JobCallback = std::function<void(const dv::TimeWindow &, const MapOfVariants &)>;

		/**
		 * Create a slice job
		 * @param intervalUS Job execution interval in microseconds
		 * @param callback The callback method
		 */
		SliceJob(const int64_t intervalUS, JobCallback callback) :
			mType(SliceType::TIME),
			mCallback(std::move(callback)),
			mInterval(intervalUS) {
		}

		/**
		 *  Create a slice by number job
		 * @param number Number of elements to be sliced
		 * @param slicing Slicing method for gaps between numeric slices
		 * @param callback The callback method
		 */
		SliceJob(const size_t number, const TimeSlicingApproach slicing, JobCallback callback) :
			mType(SliceType::NUMBER),
			mCallback(std::move(callback)),
			mNumberOfElements(number),
			mTimeSlicing(slicing) {
		}

		SliceType mType;

		/// The callback method
		JobCallback mCallback;

		/// Job execution interval in microseconds
		int64_t mInterval = -1;

		/// Slice by number configuration value
		size_t mNumberOfElements = 0;

		/// Time slicing method for slicing by number
		TimeSlicingApproach mTimeSlicing = TimeSlicingApproach::BACKWARD;
	};

	/// Maximum retain duration, this holds maximum interval from all configured jobs
	int64_t mRetainDuration = -1;

	/// Minimum number of elements to retain in the buffer
	size_t mRetainSize = 0;

	/// Seek time contains the highest timestamp of last processed batch of data
	int64_t mSeekTime = -1;

	/// Main buffer seek time, this is the timestamp of last fed data into main slicer
	int64_t mMainBufferSeekTime = -1;

	/// Storage container for configured slice jobs
	std::map<int, SliceJob> mConfig;

	/// Buffered data that is in queue for slicing
	std::map<std::string, InputType> mBuffer;

	/// Placeholder for manually provided seek timestamp of stream seek times
	std::map<std::string, int64_t> mSeekTimestamps;

	/// Name of the main stream
	std::string mMainStreamName;

	/// Slicer for the main stream, all other streams follow the main stream slicer
	dv::StreamSlicer<MainStreamType> mMainSlicer;

private:
	/**
	 * Slice a vector type within given time bounds [start, end). Start time is inclusive, end time is exclusive.
	 * @tparam VectorType
	 * @param start Start timestamp
	 * @param end End timestamp
	 * @param packet Packet of a vector type
	 * @return Copy of the data within the bounds
	 */
	template<class VectorType>
	[[nodiscard]] static inline VectorType sliceVector(
		const int64_t start, const int64_t end, const VectorType &packet) {
		auto lowerBound = std::lower_bound(packet.begin(), packet.end(), start, [](const auto &elem, const auto &time) {
			return dv::packets::getTimestamp(elem) < time;
		});

		if (lowerBound == packet.end()) {
			return VectorType();
		}

		// Upper is not going to be before lower, since time is guaranteed to be increasing. We can use
		// lowerBound as starting point
		auto upperBound = std::lower_bound(lowerBound, packet.end(), end, [](const auto &elem, const auto &time) {
			return dv::packets::getTimestamp(elem) < time;
		});

		return VectorType(lowerBound, upperBound);
	}

	/**
	 * Templated method for packet slicing. Returns the data slice between given timestamps.
	 * Start time is inclusive, end time is exclusive.
	 * @tparam PacketType
	 * @param start Start timestamp
	 * @param end End timestamp
	 * @param packet Packet of data
	 * @return Copy of the data within the bounds
	 */
	template<class PacketType>
	[[nodiscard]] static inline PacketType slicePacketSpecific(
		const int64_t start, const int64_t end, const PacketType &packet) {
		if constexpr (std::is_same_v<PacketType, std::monostate>) {
			return std::monostate{};
		}
		else if constexpr (dv::concepts::EventStorage<PacketType>) {
			return packet.sliceTime(start, end);
		}
		else if constexpr (dv::concepts::DataPacket<PacketType>) {
			PacketType sliced;
			sliced.elements = sliceVector(start, end, packet.elements);
			return sliced;
		}
		else if constexpr (dv::concepts::TimestampedIterable<PacketType>) {
			return sliceVector(start, end, packet);
		}
	}

	/**
	 * Templated method for packet contained in a variant. Returns the data slice between given timestamps.
	 * Start time is inclusive, end time is exclusive.
	 * @param start Start of time range.
	 * @param end End of time range.
	 * @param packet Input data packet.
	 * @return Sliced data from the packet according to given time ranges.
	 */
	[[nodiscard]] static inline InputType slicePacket(const int64_t start, const int64_t end, const InputType &packet) {
		return std::visit(
			[start, end](auto &&castedPacket) {
				return InputType(slicePacketSpecific(start, end, castedPacket));
			},
			packet);
	}

	/**
	 * Merge successive packets, this copies data from one to another. Performs shallow copy if possible.
	 * @tparam PacketType
	 * @param from Source packet
	 * @param into Destination packet
	 */
	template<class PacketType>
	static inline void mergePackets(const PacketType &from, PacketType &into) {
		if constexpr (std::is_same_v<PacketType, std::monostate>) {
			return;
		}
		else if constexpr (dv::concepts::EventStorage<PacketType>) {
			into.add(from);
		}
		else if constexpr (dv::concepts::DataPacket<PacketType>) {
			into.elements.insert(into.elements.end(), from.elements.begin(), from.elements.end());
		}
		else if constexpr (dv::concepts::TimestampedIterable<PacketType>) {
			into.insert(into.end(), from.begin(), from.end());
		}
	}

	/**
	 * Erase data within the packet up to the given time point while retaining at least given minimum size. Specific
	 * implementation for vector containers.
	 * @tparam PacketType
	 * @param timeLimit Timestamp to delete until, this is exclusive
	 * @param minimumSize Minimum amount of elements to retain in the packet
	 * @param packet Packet to modify
	 */
	template<class PacketType>
	requires dv::concepts::Iterable<PacketType>
	static inline void eraseUpToIterable(const int64_t timeLimit, const size_t minimumSize, PacketType &packet) {
		if (packet.size() == 0) {
			return;
		}

		const auto lowerBound
			= std::lower_bound(packet.begin(), packet.end(), timeLimit, [](const auto &elem, const auto &time) {
				  return dv::packets::getTimestamp(elem) < time;
			  });

		if (minimumSize == 0 || std::distance(lowerBound, packet.end()) >= minimumSize) {
			packet.erase(packet.begin(), lowerBound);
		}
		else {
			const int64_t sizeBoundIndex = packet.size() - minimumSize - 1;
			if (sizeBoundIndex < 0) {
				return;
			}

			const auto sizeBound = packet.begin() + sizeBoundIndex;
			if (dv::packets::getTimestamp(*sizeBound) < timeLimit) {
				packet.erase(packet.begin(), sizeBound);
			}
		}
	}

	/**
	 * Erase data within the packet up to the given time point.
	 * @tparam PacketType
	 * @param timeLimit Timestamp to delete until, this is exclusive
	 * @param minimumSize Minimum amount of elements to retain in the packet
	 * @param packet Packet to modify
	 */
	template<class PacketType>
	static inline void eraseUpTo(const int64_t timeLimit, const size_t minimumSize, PacketType &packet) {
		if constexpr (std::is_same_v<PacketType, std::monostate>) {
			return;
		}
		else if constexpr (dv::concepts::EventStorage<PacketType>) {
			const auto sliced = packet.sliceTime(timeLimit, packet.getHighestTime() + 1);
			if (minimumSize == 0 || sliced.size() >= minimumSize) {
				packet = sliced;
			}
			else {
				const auto slicedBySize = packet.sliceBack(minimumSize);
				if (slicedBySize.getLowestTime() < timeLimit) {
					packet = slicedBySize;
				}
			}
		}
		else if constexpr (dv::concepts::DataPacket<PacketType>) {
			eraseUpToIterable(timeLimit, minimumSize, packet.elements);
		}
		else if constexpr (dv::concepts::TimestampedIterable<PacketType>) {
			eraseUpToIterable(timeLimit, minimumSize, packet);
		}
	}

	/**
	 * Retrieve highest and lowest timestamps of a given packet
	 * @tparam PacketType
	 * @param packet
	 * @return Time window containing start and end timestamps.
	 */
	template<class PacketType>
	[[nodiscard]] static inline dv::TimeWindow getPacketTimeWindow(const PacketType &packet) {
		if constexpr (std::is_same_v<PacketType, std::monostate>) {
			return {0, 0};
		}
		else if constexpr (dv::concepts::EventStorage<PacketType>) {
			return dv::TimeWindow(packet.getLowestTime(), packet.getHighestTime());
		}
		else if constexpr (dv::concepts::DataPacket<PacketType>) {
			return dv::TimeWindow(dv::packets::getTimestamp(*packet.elements.begin()),
				dv::packets::getTimestamp(*std::prev(packet.elements.end())));
		}
		else if constexpr (dv::concepts::TimestampedIterable<PacketType>) {
			return dv::TimeWindow(
				dv::packets::getTimestamp(*packet.begin()), dv::packets::getTimestamp(*std::prev(packet.end())));
		}
	}

	/**
	 * Check if a packet is empty.
	 * @tparam PacketType
	 * @param packet
	 * @return True if the given packet is empty, false otherwise.
	 */
	template<class PacketType>
	[[nodiscard]] static inline bool isPacketEmpty(const PacketType &packet) {
		if constexpr (std::is_same_v<PacketType, std::monostate>) {
			return true;
		}
		else if constexpr (dv::concepts::EventStorage<PacketType>) {
			return packet.isEmpty();
		}
		else if constexpr (dv::concepts::DataPacket<PacketType>) {
			return packet.elements.empty();
		}
		else {
			return packet.empty();
		}
	}

	/**
	 * Evaluate the current state of the slicer. Performs data book-keeping and executes the callback methods.
	 */
	void evaluate() {
		int64_t minHighestTime = std::numeric_limits<int64_t>::max();

		// Find the lowest maximum time within the buffered stream data and seek to it
		for (const auto &[stream, streamBuffer] : mBuffer) {
			// Adding alias to stream, since CLang does not resolve stream in lambda capture correctly
			const std::string &streamName = stream;
			int64_t highTime              = -1;
			int64_t lowTime;
			const bool success = std::visit(
				[this, &minHighestTime, &streamName](auto &&packet) {
					const int64_t streamSeek = mSeekTimestamps.at(streamName);

					if (isPacketEmpty(packet)) {
						if (streamSeek >= 0) {
							minHighestTime = std::min(minHighestTime, streamSeek);
							return true;
						}
						else {
							return false;
						}
					}

					minHighestTime = std::min(getPacketTimeWindow(packet).endTime, minHighestTime);
					return true;
				},
				streamBuffer);

			if (!success) {
				return;
			}
		}

		const int64_t timeLimit = minHighestTime + 1;

		// If the seek did not progress, skip evaluation
		if (mMainBufferSeekTime == timeLimit) {
			return;
		}

		auto &mainBuffer = std::get<MainStreamType>(mBuffer[mMainStreamName]);
		mMainSlicer.accept(slicePacketSpecific(0, timeLimit, mainBuffer));
		eraseUpTo(timeLimit, 0, mainBuffer);
		mMainBufferSeekTime = timeLimit;

		for (auto &[stream, container] : mBuffer) {
			if (stream == mMainStreamName) {
				continue;
			}

			const int64_t eraseLimit = (mRetainDuration >= 0) ? (mSeekTime - mRetainDuration) : timeLimit;
			std::visit(
				[eraseLimit, retainSize = mRetainSize](auto &&packet) {
					eraseUpTo(eraseLimit, retainSize, packet);
				},
				container);
		}
	}
};

// Sanity check for the well known default types
static_assert(concepts::CompatibleWithSlicer<dv::EventStore>);
static_assert(concepts::CompatibleWithSlicer<dv::EventPacket>);
static_assert(concepts::CompatibleWithSlicer<dv::cvector<dv::Frame>>);
static_assert(concepts::CompatibleWithSlicer<dv::IMUPacket>);
static_assert(concepts::CompatibleWithSlicer<dv::cvector<dv::IMU>>);
static_assert(concepts::CompatibleWithSlicer<dv::TriggerPacket>);
static_assert(concepts::CompatibleWithSlicer<dv::cvector<dv::Trigger>>);

} // namespace dv
