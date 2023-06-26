#pragma once

#include "../data/depth_event_base.hpp"
#include "../data/event_base.hpp"
#include "../data/frame_base.hpp"
#include "../exception/exceptions/generic_exceptions.hpp"
#include "concepts.hpp"
#include "stream_slicer.hpp"
#include "time.hpp"

#include <Eigen/Dense>
#include <opencv2/core.hpp>
#include <opencv2/core/eigen.hpp>

#include <algorithm>
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <numeric>
#include <optional>
#include <vector>

namespace dv {

namespace io {
class MonoCameraWriter;
class NetworkWriter;
} // namespace io

/**
 * __INTERNAL USE ONLY__
 * Compares an events timestamp to that of a timestamp.
 */
template<dv::concepts::AddressableEvent EventType>
class EventTimeComparator {
public:
	bool operator()(const EventType &evt, const int64_t time) const {
		return evt.timestamp() < time;
	}

	bool operator()(const int64_t time, const EventType &evt) const {
		return time < evt.timestamp();
	}
};

/**
 * A structure that contains events represented in eigen matrices. Useful for mathematical operations
 * using the Eigen library..
 */
struct EigenEvents {
	Eigen::Matrix<int64_t, Eigen::Dynamic, 1> timestamps;
	Eigen::Matrix<int16_t, Eigen::Dynamic, 2> coordinates;
	Eigen::Matrix<uint8_t, Eigen::Dynamic, 1> polarities;

	explicit EigenEvents(const size_t size) : timestamps(size), coordinates(size, 2), polarities(size) {
	}
};

/**
 * __INTERNAL USE ONLY__
 * Internal event container class that holds a shard of events.
 * A `PartialEventData` holds a shared pointer to an `EventPacket`, which
 * is the underlying data structure. The underlying data can either be const,
 * in which case no addition is allowed, or non const, in which addition
 * of new data is allowed. Slicing is allowed in both cases, as it only modifies
 * the control structure.
 * All the events in the partial have to
 * be monotonically increasing in time. A `PartialEventData` can be sliced
 * both from the front as well as from the back. By doing so, the memory
 * footprint of the structure is not modified, just the internal bookkeeping
 * pointers are readjusted. The `PartialEventData` keeps track of lowest as
 * well as highest times of events in the structure.
 *
 * The data `PartialEventData` points to can be shared between multiple
 * `PartialEventData`, each with potentially different slicings.
 */
template<concepts::AddressableEvent EventType, class EventPacketType>
class PartialEventData {
	friend class dv::io::MonoCameraWriter;
	friend class dv::io::NetworkWriter;
	using iterator = typename dv::cvector<const EventType>::iterator;

private:
	bool referencesConstData_;
	size_t start_;
	size_t length_;
	size_t capacity_;
	int64_t lowestTime_;
	int64_t highestTime_;
	std::shared_ptr<EventPacketType> modifiableDataPtr_;
	std::shared_ptr<const EventPacketType> data_;

public:
	/**
	 * Creates a new `PartialEventData` shard. Allocates new memory on the
	 * heap to keep the data. Upon constructions, the newly created object
	 * is the sole owner of the data.
	 * @param capacity Number of events this data partial can store.
	 */
	explicit PartialEventData(const size_t capacity = 10000) :
		referencesConstData_(false),
		start_(0),
		length_(0),
		capacity_(capacity),
		lowestTime_(0),
		highestTime_(0),
		modifiableDataPtr_(std::make_shared<EventPacketType>()),
		data_(modifiableDataPtr_) {
		modifiableDataPtr_->elements.reserve(capacity);
	}

	/**
	 * Creates a new `PartialEventData` shard from existing const data. Copies the
	 * supplied shared_ptr into the structure, acquiring shared ownership of
	 * the supplied data.
	 * @param data The shared pointer to the data to which we want to obtain shared
	 * ownership
	 */
	explicit PartialEventData(std::shared_ptr<const EventPacketType> data) :
		referencesConstData_(true),
		start_(0),
		length_(data->elements.size()),
		capacity_(length_),
		lowestTime_(data->elements.front().timestamp()),
		highestTime_(data->elements.back().timestamp()),
		modifiableDataPtr_(nullptr),
		data_(std::move(data)) {
	}

	/**
	 * Copy constructor.
	 * Creates a shallow copy of `other` without copying the actual
	 * data over. As slicing does not alter the underlying data,
	 * the new copy may be sliced without affecting the orignal object.
	 * @param other
	 */
	PartialEventData(const PartialEventData &other) = default;

	/**
	 * Returns an iterator to the first element that is bigger than
	 * the supplied timestamp. If every element is bigger than the supplied
	 * time, an iterator to the first element is returned (same as `begin()`).
	 * If all elements have a smaller timestamp than the supplied, the end
	 * iterator is returned (same as `end()`).
	 * @param time The requested time. The iterator will be the first element
	 * with a timestamp larger than this time.
	 * @return An iterator to the first element larger than the supplied time.
	 */
	iterator iteratorAtTime(const int64_t time) const {
		auto comparator    = EventTimeComparator<EventType>();
		auto sliceEventItr = std::lower_bound(begin(), end(), time, comparator);
		return sliceEventItr;
	}

	/**
	 * Returns an iterator to the first element of the `PartialEventData`.
	 * The iterator is according to the current slice and not to the
	 * underlying datastore. E.g. when slicing the shard from the front,
	 * the `begin()` will change.
	 * @return Returns an iterator at the beginning data partial
	 */
	iterator begin() const {
		return data_->elements.begin() + start_;
	}

	/**
	 * Returns an iterator to one after the last element of the `PartialEventData`.
	 * The iterator is according to the current slice and not to the
	 * underlying datastore. E.g. when slicing the shard from the back,
	 * the result of `end()` will change.
	 * @return Returns an iterator at the end of the data partial
	 */
	iterator end() const {
		return data_->elements.begin() + start_ + length_;
	}

	/**
	 * Slices off `number` events from the front of the `PartialEventData`.
	 * This operation just adjust the bookkeeping of the datastructure
	 * without actually modifying the underlying data representation.
	 * If there are not enough events left, a `range_error` exception is thrown.
	 *
	 * Other instances of `PartialEventData` which share the same underlying
	 * data are not affected by this.
	 * @param number amount of events to be removed from the front.
	 */
	void sliceFront(const size_t number) {
		if (number > length_) {
			throw std::range_error("Can not slice more than length from PartialEventData.");
		}

		start_      = start_ + number;
		length_     = length_ - number;
		lowestTime_ = (length_ == 0) ? (0) : (data_->elements)[start_].timestamp();
	}

	/**
	 * Slices off `number` events from the back of the `PartialEventData`.
	 * This operation just adjust the bookkeeping of the datastructure
	 * without actually modifying the underlying data representation.
	 * If there are not enough events left, a `range_error` exception is thrown.
	 *
	 * Other instances of `PartialEventData` which share the same underlying
	 * data are not affected by this.
	 * @param number amount of events to be removed from the back.
	 */
	void sliceBack(const size_t number) {
		if (number > length_) {
			throw std::range_error("Can not slice more than length from PartialEventData.");
		}

		length_      = length_ - number;
		highestTime_ = (length_ == 0) ? (0) : (data_->elements)[start_ + length_ - 1].timestamp();
	}

	/**
	 * Slices off all the events that occur before the supplied time.
	 * The resulting data structure has a `lowestTime > time` where time
	 * is the supplied time.
	 *
	 * This operation just adjust the bookkeeping of the datastructure
	 * without actually modifying the underlying data representation.
	 * If there are not enough events left, a `range_error` exception is thrown.
	 *
	 * Other instances of `PartialEventData` which share the same underlying
	 * data are not affected by this.
	 *
	 * @param time the threshold time. All events `<= time` will be sliced off
	 * @return number of events that actually got sliced off as a result of
	 * this operation.
	 */
	size_t sliceTimeFront(const int64_t time) {
		auto timeItr = iteratorAtTime(time);
		auto index   = static_cast<size_t>(timeItr - begin());
		sliceFront(index);
		return index;
	}

	/**
	 * Slices off all the events that occur after the supplied time.
	 * The resulting data structure has a `lowestTime < time` where time
	 * is the supplied time.
	 *
	 * This operation just adjust the bookkeeping of the datastructure
	 * without actually modifying the underlying data representation.
	 * If there are not enough events left, a `range_error` exception is thrown.
	 *
	 * Other instances of `PartialEventData` which share the same underlying
	 * data are not affected by this.
	 *
	 * @param time the threshold time. All events `> time` will be sliced off
	 * @return number of events that actually got sliced off as a result of
	 * this operation.
	 */
	size_t sliceTimeBack(const int64_t time) {
		auto timeItr     = iteratorAtTime(time);
		auto index       = static_cast<size_t>(timeItr - begin());
		size_t cutAmount = length_ - index;
		sliceBack(cutAmount);
		return cutAmount;
	}

	/**
	 * __UNSAFE OPERATION__
	 * Copies the data of the supplied event into the underlying data
	 * structure and updates the internal bookkeeping to accommodate the event.
	 *
	 * NOTE: This function does not perform any boundary checks.
	 * Any call to function is expected to have performed the following
	 * boundary checks: `canStoreMoreEvents()` to see if there is space to accommodate
	 * the new event. `getHighestTime()` has to be smaller or equal than
	 * the new event's timestamp, as we require events to be monotonically
	 * increasing.
	 *
	 * @param event The event to be added
	 */
	void _unsafe_addEvent(const EventType &event) {
		highestTime_ = event.timestamp();
		if (length_ == 0) {
			lowestTime_ = event.timestamp();
		}
		modifiableDataPtr_->elements.emplace_back(event);
		length_++;
	}

	/**
	 * __UNSAFE OPERATION__
	 * Moves the data of the supplied event into the underlying data
	 * structure and updates the internal bookkeeping to accommodate the event.
	 *
	 * NOTE: This function does not perform any boundary checks.
	 * Any call to function is expected to have performed the following
	 * boundary checks: `canStoreMoreEvents()` to see if there is space to accommodate
	 * the new event. `getHighestTime()` has to be smaller or equal than
	 * the new event's timestamp, as we require events to be monotonically
	 * increasing.
	 *
	 * @param event The event to be added
	 */
	void _unsafe_moveEvent(EventType &&event) {
		highestTime_ = event.timestamp();
		if (length_ == 0) {
			lowestTime_ = event.timestamp();
		}
		// this should cause a move action instead of a copy
		modifiableDataPtr_->elements.push_back(std::move(event));
		length_++;
	}

	/**
	 * Get a reference to the first available event in the partial.
	 * @return 		Reference to first element in the partial.
	 */
	[[nodiscard]] EventType &front() {
		return *modifiableDataPtr_->elements[start_];
	}

	/**
	 * Get a reference to the last available event in the partial.
	 * @return 		Reference to last element in the partial.
	 */
	[[nodiscard]] EventType &back() {
		return modifiableDataPtr_->elements[start_ + length_ - 1];
	}

	/**
	 * The length of the current slice of data. This value can be in
	 * range [0; capacity].
	 * @return the current length of the slice in number of events.
	 */
	[[nodiscard]] inline size_t getLength() const {
		return length_;
	}

	/**
	 * Gets the lowest timestamp of an event that is represented in this
	 * Partial. The lowest timestamp is always identical to the timestamp
	 * of the first event of the slice.
	 *
	 * @return The timestamp of the first event in the slice.
	 * This is also the lowest time present in this slice.
	 */
	[[nodiscard]] inline int64_t getLowestTime() const {
		return lowestTime_;
	}

	/**
	 * Gets the highest timestamp of an event that is represented in this
	 * Partial. The lowest timestamp is always identical to the timestamp
	 * of the last event of the slice.
	 *
	 * @return The timestamp of the last event in the slice. This is also
	 * the highest timestamp present in this slice.
	 */
	[[nodiscard]] inline int64_t getHighestTime() const {
		return highestTime_;
	}

	/**
	 * Returns a reference to the element at the given offset of
	 * the slice.
	 * @param offset The offset in the slice of which element
	 * a reference should be obtained
	 * @return A reference to the object at offset offset
	 */
	[[nodiscard]] inline const EventType &operator[](size_t offset) const {
		dv::runtime_assert(offset <= length_, "offset out of bounds");
		return (data_->elements)[start_ + offset];
	}

	/**
	 * Checks if it is safe to add more events to this partial.
	 * It is safe to add more events when the following conditions are fulfilled:
	 * * The partial does not represent const data. In that case, any modification of the underlying buffer
	 *   is impossible.
	 * * The partial does not exceed the sharding count limit
	 * * The partial hasn't been sliced from the back
	 *
	 * If it has been sliced from the back, adding new events would
	 * put them in unreachable space.
	 *
	 * @return true if there is space available to store more events in
	 * this partial.
	 */
	[[nodiscard]] inline bool canStoreMoreEvents() const {
		return !referencesConstData_
			&& (data_->elements.size() < capacity_ && start_ + length_ == data_->elements.size());
	}

	/**
	 * Amount of space still available in this data partial.
	 * @return 		Amount of events this data partial can store additionally.
	 */
	[[nodiscard]] inline size_t availableCapacity() const {
		if (referencesConstData_) {
			return 0;
		}

		return capacity_ - data_->elements.size();
	}

	/**
	 * Merge the other data partial into this one by copying the contents, if that is possible. If merge is not
	 * possible, the function returns false and does nothing.
	 * @param other 	Other data partial to be merged into this one.
	 * @return 			True if merge was successful, false otherwise.
	 */
	[[nodiscard]] bool merge(const PartialEventData &other) {
		// Test whether merge is allowed.
		if (!canStoreMoreEvents() || availableCapacity() < other.getLength()) {
			return false;
		}

		// If the other shard is empty, we do not have to do anything
		if (other.getLength() == 0) {
			return true;
		}

		// Actual merge
		modifiableDataPtr_->elements.insert(end(), other.begin(), other.end());
		length_      += other.length_;
		highestTime_ = back().timestamp();
		return true;
	}
};

/**
 * __INTERNAL USE ONLY__
 * Comparator Functor that checks if a given time lies within bounds of the event packet
 */
template<concepts::AddressableEvent EventType, class EventPacketType>
class PartialEventDataTimeComparator {
private:
	const bool lower_;

public:
	explicit PartialEventDataTimeComparator(const bool lower) : lower_(lower) {
	}

	/**
	 * Returns true, if the comparator is set to not lower and the given time is higher than the highest
	 * timestamp of the partial, or when it is set to lower and the timestamp is higher than the lowest
	 * timestamp of the partial.
	 * @param partial The partial to be analysed
	 * @param time The time to be compared against
	 * @return true, if time is higher than either lowest or highest timestamp of partial depending on state
	 */
	bool operator()(const PartialEventData<EventType, EventPacketType> &partial, const int64_t time) const {
		return lower_ ? partial.getLowestTime() < time : partial.getHighestTime() < time;
	}

	/**
	 * Returns true, if the comparator is set to not lower and the given time is higher than the lowest
	 * timestamp of the partial, or when it is set to lower and the timestamp is higher than the highest
	 * timestamp of the partial.
	 * @param partial The partial to be analysed
	 * @param time The time to be compared against
	 * @return true, if time is higher than either lowest or lowest timestamp of partial depending on state
	 */
	bool operator()(const int64_t time, const PartialEventData<EventType, EventPacketType> &partial) const {
		return lower_ ? time < partial.getLowestTime() : time < partial.getHighestTime();
	}
};

/**
 * Iterator for the EventStore class.
 */
template<concepts::AddressableEvent EventType, class EventPacketType>
class AddressableEventStorageIterator {
private:
	const std::vector<PartialEventData<EventType, EventPacketType>> *dataPartialsPtr_;
	/** The current partial (shard) we point to */
	size_t partialIndex_;
	/** The current offset inside the shard we point to */
	size_t offset_;

	/**
	 * Increments the iterator to the next event.
	 * If the iterator goes beyond available data, it remains
	 * at this position.
	 */
	inline void increment() {
		offset_++;
		if (offset_ >= (*dataPartialsPtr_)[partialIndex_].getLength()) {
			offset_ = 0;
			if (partialIndex_ < dataPartialsPtr_->size()) { // increment only to one partial after end
				partialIndex_++;
			}
		}
	}

	/**
	 * Decrements the iterator to the previous event.
	 * If the iterator goes below zero, it remains
	 * at zero.
	 */
	inline void decrement() {
		if (partialIndex_ >= dataPartialsPtr_->size()) {
			partialIndex_ = dataPartialsPtr_->size() - 1;
			offset_       = (*dataPartialsPtr_)[partialIndex_].getLength() - 1;
		}
		else {
			if (offset_ > 0) {
				offset_--;
			}
			else {
				if (partialIndex_ > 0) {
					partialIndex_--;
					offset_ = (*dataPartialsPtr_)[partialIndex_].getLength() - 1;
				}
			}
		}
	}

public:
	using iterator_category = std::bidirectional_iterator_tag;
	using value_type        = const EventType;
	using pointer           = const EventType *;
	using reference         = const EventType &;
	using size_type         = size_t;
	using difference_type   = ptrdiff_t;

	/**
	 * Default constructor. Creates a new iterator at the beginning of
	 * the packet
	 */
	AddressableEventStorageIterator() : AddressableEventStorageIterator(nullptr, true) {
	}

	/**
	 * Creates a new Iterator either at the beginning or at the end
	 * of the package
	 * @param dataPartialsPtr to the partials (shards) of the packet
	 * @param front iterator will be at the beginning (true) of the packet,
	 * or at the end (false) of the packet.
	 */
	explicit AddressableEventStorageIterator(
		const std::vector<PartialEventData<EventType, EventPacketType>> *dataPartialsPtr, const bool front) :
		dataPartialsPtr_(dataPartialsPtr),
		offset_(0) {
		partialIndex_ = front ? 0 : dataPartialsPtr->size();
	}

	/**
	 * __INTERNAL USE ONLY__
	 * Creates a new iterator at the specific internal position supplied
	 * @param dataPartialsPtr Pointer to the partials (shards) of the packet
	 * @param partialIndex Index pointing to the active shard
	 * @param offset Offset in the active shard
	 */
	AddressableEventStorageIterator(const std::vector<PartialEventData<EventType, EventPacketType>> *dataPartialsPtr,
		const size_t partialIndex, const size_t offset) :
		dataPartialsPtr_(dataPartialsPtr),
		partialIndex_(partialIndex),
		offset_(offset) {
	}

	/**
	 * @return A reference to the Event at the current iterator position
	 */
	inline reference operator*() const noexcept {
		return (*dataPartialsPtr_)[partialIndex_][offset_];
	}

	/**
	 * @return A pointer to the Event at current iterator position
	 */
	inline pointer operator->() const noexcept {
		return &(this->operator*());
	}

	/**
	 * Increments the iterator by one
	 * @return A reference to the the same iterator, incremented by one
	 */
	AddressableEventStorageIterator &operator++() noexcept {
		increment();
		return *this;
	}

	/**
	 * Post-increments the iterator by one
	 * @return A new iterator at the current position. Increments original
	 * iterator by one.
	 */
	const AddressableEventStorageIterator operator++(int) noexcept {
		auto currentIterator = AddressableEventStorageIterator(dataPartialsPtr_, partialIndex_, offset_);
		increment();
		return currentIterator;
	}

	/**
	 * Increments iterator by a fixed number and returns reference to itself
	 * @param add amount one whishes to increment the iterator
	 * @return reference to itseld incremented by `add`
	 */
	AddressableEventStorageIterator &operator+=(const size_type add) noexcept {
		for (size_t i = 0; i < add; i++) {
			increment();
		}
		return *this;
	}

	/**
	 * Decrements the iterator by one
	 * @return A reference to the the same iterator, decremented by one
	 */
	AddressableEventStorageIterator &operator--() noexcept {
		decrement();
		return *this;
	}

	/**
	 * Post-decrement the iterator by one
	 * @return A new iterator at the current position. Decrements original
	 * iterator by one.
	 */
	const AddressableEventStorageIterator operator--(int) noexcept {
		auto currentIterator = AddressableEventStorageIterator(dataPartialsPtr_, partialIndex_, offset_);
		decrement();
		return currentIterator;
	}

	/**
	 * Decrements iterator by a fixed number and returns reference to itself
	 * @param sub amount one whishes to decrement the iterator
	 * @return reference to itseld decremented by `sub`
	 */
	AddressableEventStorageIterator &operator-=(const size_type sub) noexcept {
		for (size_t i = 0; i < sub; i++) {
			decrement();
		}
		return *this;
	}

	/**
	 * @param rhs iterator to compare to
	 * @return true if both iterators point to the same element
	 */
	bool operator==(const AddressableEventStorageIterator &rhs) const noexcept {
		return (partialIndex_ == rhs.partialIndex_) && (offset_ == rhs.offset_);
	}

	/**
	 * @param rhs iterator to compare to
	 * @return true if both iterators point to different elements
	 */
	bool operator!=(const AddressableEventStorageIterator &rhs) const noexcept {
		return !(this->operator==(rhs));
	}
};

using EventStoreIterator [[deprecated("Use dv::EventStore::iterator instead")]]
= AddressableEventStorageIterator<dv::Event, dv::EventPacket>;

/**
 * EventStore class.
 * An `EventStore` is a collection of consecutive events, all monotonically
 * increasing in time. EventStore is the basic data structure for handling
 * event data. Event packets hold their data in shards of fixed size.
 * Copying an `EventStore` results in a shallow copy with shared ownership
 * of the shards that are common to both EventStores.
 * EventStores can be sliced by number of events or by time. Slicing creates
 * a shallow copy of the `EventPackage`.
 */
template<concepts::AddressableEvent EventType, class EventPacketType>
class AddressableEventStorage {
	friend class dv::io::MonoCameraWriter;
	friend class dv::io::NetworkWriter;

public:
	// Container traits.
	using value_type        = EventType;
	using const_value_type  = const EventType;
	using pointer           = EventType *;
	using const_pointer     = const EventType *;
	using reference         = EventType &;
	using const_reference   = const EventType &;
	using size_type         = size_t;
	using difference_type   = ptrdiff_t;
	using packet_type       = EventPacketType;
	using const_packet_type = const EventPacketType;

	// Iterator support.
	using iterator       = AddressableEventStorageIterator<EventType, EventPacketType>;
	using const_iterator = iterator;

protected:
	using PartialEventDataType = PartialEventData<EventType, EventPacketType>;

	/** internal list of the shards. */
	std::vector<PartialEventDataType> dataPartials_;
	/** The exact number-of-events global offsets of the shards */
	std::vector<size_t> partialOffsets_;
	/** The total length of the event package */
	size_t totalLength_{0};
	/** Default capacity for the data partials **/
	size_t shardCapacity_{10000};

	/**
	 * __INTERNAL USE ONLY__
	 * Creates a new EventStore based on the supplied `PartialEventData`
	 * objects. Offsets and meta information is recomputed from the supplied
	 * list. The packet gets shared ownership of all underlying data
	 * of the `PartialEventData` slices in `dataPartials`.
	 * @param dataPartials vector of `PartialEventData` to construct this
	 * package from.
	 */
	explicit AddressableEventStorage(const std::vector<PartialEventDataType> &dataPartials) {
		this->dataPartials_ = dataPartials;

		// Build up length and offsets
		for (const auto &partial : dataPartials) {
			partialOffsets_.emplace_back(totalLength_);
			totalLength_ += partial.getLength();
		}
	}

	/**
	 * Retrieve the last partial that can store events. If available partial is full or no partials available at all,
	 * this function will instantiate, add the partial to the store, and return a reference to that partial.
	 * @return 		Last data partial that can store an additional event.
	 */
	[[nodiscard]] PartialEventData<EventType, EventPacketType> &_getLastNonFullPartial() {
		if (!dataPartials_.empty() && dataPartials_.back().canStoreMoreEvents()) {
			return dataPartials_.back();
		}

		partialOffsets_.emplace_back(totalLength_);
		return dataPartials_.emplace_back(shardCapacity_);
	}

public:
	/**
	 * Default constructor.
	 * Creates an empty `EventStore`. This does not allocate any memory
	 * as long as there is no data.
	 */
	AddressableEventStorage() = default;

	/**
	 * Merges the contents of the supplied Event Store into the current event store. This operation can cause
	 * event data copies if that results in more optimal memory layout, otherwise the operation only performs
	 * shallow copies of the data by sharing the ownership with previous event storage. The two event stores
	 * have to be in ascending order.
	 * @param store the store to be added to this store
	 */
	void add(const AddressableEventStorage &store) {
		if (store.isEmpty()) {
			return;
		}

		if (getHighestTime() > store.getLowestTime()) {
			throw std::out_of_range{"Tried adding event store to store out of order. Ignoring packet."};
		}

		for (const auto &partial : store.dataPartials_) {
			// Try to merge with the last partial if possible
			if (dataPartials_.empty() || !dataPartials_.back().merge(partial)) {
				// If we have no dataPartials yet or merging with the last partial fails, just append the new partial
				// to the partial list
				dataPartials_.push_back(partial);
				partialOffsets_.push_back(totalLength_);
			}

			totalLength_ += partial.getLength();
		}
	}

	/**
	 * Retrieve timestamps of events into a one-dimensional eigen matrix. This performs a copy of the values. The
	 * values are guaranteed to be monotonically increasing.
	 * @return	A one-dimensional eigen matrix containing timestamps of events.
	 */
	[[nodiscard]] Eigen::Matrix<int64_t, Eigen::Dynamic, 1> timestamps() const {
		Eigen::Matrix<int64_t, Eigen::Dynamic, 1> timestamps(size());
		int64_t *target = timestamps.data();
		for (const auto &event : *this) {
			*target = event.timestamp();
			target++;
		}
		return timestamps;
	}

	/**
	 * Retrieve coordinates of events in a 2xN eigen matrix. Method performs a copy of the values. Coordinates
	 * maintain the same order as within the event store. First column is the x coordinate, second column is the
	 * y coordinate.
	 * @return 	A two-dimensional eigen matrix containing x and y coordinates of events.
	 */
	[[nodiscard]] Eigen::Matrix<int16_t, Eigen::Dynamic, 2> coordinates() const {
		Eigen::Matrix<int16_t, Eigen::Dynamic, 2> coordinates(size(), 2);
		int16_t *x = coordinates.data();
		int16_t *y = coordinates.data() + size();
		for (const auto &event : *this) {
			*x = event.x();
			x++;
			*y = event.y();
			y++;
		}
		return coordinates;
	}

	/**
	 * Retrieve polarities of events in a one-dimensional eigen matrix. Method performs a copy of the values.
	 * Polarities maintain the same order as within the event store. Polarities are converted into unsigned 8-bit
	 * integer values, where 0 stands for negative polarity event and 1 stands for positive polarity event.
	 * @return	A one-dimensional eigen matrix containing polarities of events.
	 */
	[[nodiscard]] Eigen::Matrix<uint8_t, Eigen::Dynamic, 1> polarities() const {
		Eigen::Matrix<uint8_t, Eigen::Dynamic, 1> polarities(size());
		uint8_t *target = polarities.data();
		for (const auto &event : *this) {
			*target = event.polarity();
			target++;
		}
		return polarities;
	}

	/**
	 * Convert the event store into eigen matrices. This function performs a deep copy of the memory.
	 * @return 	Events in represented in eigen matrices.
	 */
	[[nodiscard]] EigenEvents eigen() const {
		EigenEvents vectors(size());
		int64_t *timestamps = vectors.timestamps.data();
		uint8_t *polarities = vectors.polarities.data();
		int16_t *x          = vectors.coordinates.data();
		int16_t *y          = vectors.coordinates.data() + size();
		for (const auto &event : *this) {
			*x = event.x();
			x++;
			*y = event.y();
			y++;
			*timestamps = event.timestamp();
			timestamps++;
			*polarities = event.polarity();
			polarities++;
		}
		return vectors;
	}

	/**
	 * Creates a new `EventStore` with the data from an `EventPacket`.
	 * This is a shallow operation. No data is copied. The EventStore gains shared
	 * ownership of the supplied data.
	 * This constructor also allows the implicit conversion from `dv::InputVectorDataWrapper<dv::EventPacket,
	 * dv::Event>` to `dv::AddressableEventStorage<dv::Event, dv::EventPacket>` Implicit conversion intended.
	 * @param packet the packet to construct the EventStore from
	 */
	explicit AddressableEventStorage(std::shared_ptr<const EventPacketType> packet) {
		if (packet->elements.empty()) {
			return;
		}

		if (!dataPartials_.empty() && dataPartials_.back().getHighestTime() > packet->elements.front().timestamp()) {
			throw std::out_of_range{"Tried adding event packet to store out of order. Ignoring packet."};
		}

		dataPartials_.emplace_back(std::move(packet));
		partialOffsets_.emplace_back(totalLength_);
		totalLength_ += dataPartials_.back().getLength();
	}

	/**
	 * Assignment operator for packet const-pointer type. Will construct a new EventStore within the variable.
	 * @param packet A pointer to the event data packet.
	 * @return
	 */
	AddressableEventStorage &operator=(std::shared_ptr<const EventPacketType> packet) {
		*this = AddressableEventStorage{std::move(packet)};
		return *this;
	}

	/**
	 * Adds a single Event to the EventStore. This will potentially
	 * allocate more memory when the currently available shards are exhausted.
	 * Any new memory receives exclusive ownership by this packet.
	 * @param event A reference to the event to be added.
	 */
	[[deprecated("Use push_back() or emplace_back() instead.")]] void add(const EventType &event) {
		push_back(event);
	}

	/**
	 * Adds a single Event to the EventStore. This will potentially
	 * allocate more memory when the currently available shards are exhausted.
	 * Any new memory receives exclusive ownership by this packet.
	 * @param event A reference to the event to be added.
	 */
	void push_back(const EventType &event) {
		if (getHighestTime() > event.timestamp()) {
			throw std::out_of_range{"Tried adding event to store out of order. Ignoring packet."};
		}

		_getLastNonFullPartial()._unsafe_addEvent(event);
		this->totalLength_++;
	}

	/**
	 * Moves a single Event into the EventStore. This will potentially
	 * allocate more memory when the currently available shards are exhausted.
	 * Any new memory receives exclusive ownership by this packet.
	 * @param event A movable reference to the event to be added.
	 */
	void push_back(EventType &&event) {
		if (getHighestTime() > event.timestamp()) {
			throw std::out_of_range{"Tried adding event to store out of order. Ignoring packet."};
		}

		_getLastNonFullPartial()._unsafe_moveEvent(std::move(event));
		this->totalLength_++;
	}

	/**
	 * Construct an event at the end of the storage.
	 * @tparam _constr_args 	Argument template
	 * @param _args 			Argument values
	 * @return 					Reference to the last newly created element
	 */
	template<class... Args>
	EventType &emplace_back(Args &&...args) {
		EventType event(std::forward<Args>(args)...);

		if (getHighestTime() > event.timestamp()) {
			throw std::out_of_range{"Tried adding event to store out of order. Ignoring packet."};
		}

		// This shouldn't cause any copy operations
		auto &targetPartial = _getLastNonFullPartial();
		targetPartial._unsafe_addEvent(std::move(event));
		this->totalLength_++;

		return targetPartial.back();
	}

	/**
	 * Returns a new EventStore that is the sum of this event store as well as the supplied event store.
	 * This is a const operation that does not modify this event store. The returned event store holds
	 * all the data of this store and the other. This is a shallow operation, no event data has to be copied
	 * for this.
	 * @param other The other store to be added
	 * @return A new EventStore, containing the events from this and the other store
	 */
	[[nodiscard]] inline AddressableEventStorage operator+(const AddressableEventStorage &other) const {
		auto s = AddressableEventStorage(*this);
		s.add(other);
		return s;
	}

	/**
	 * Returns a new event store that contains the same data as this event store, but with the
	 * given event added. This is a shallow operation. No event data has to be copied for this.
	 * @param event The event to be added to this event store
	 * @return A new event store containing the same data as the old event store plus the supplied event
	 */
	[[nodiscard]] [[deprecated("Use push_back() or emplace_back() instead.")]] inline AddressableEventStorage operator+(
		const EventType &event) const {
		auto s = AddressableEventStorage(*this);
		s.add(event);
		return s;
	}

	/**
	 * Adds all the events of the other event store to this event store.
	 * @param other The event store to be added
	 */
	inline void operator+=(const AddressableEventStorage &other) {
		add(other);
	}

	/**
	 * Adds the provided event to the end of this event store
	 * @param event The event to be added
	 */
	[[deprecated("Use push_back() or emplace_back() instead.")]] inline void operator+=(const EventType &event) {
		push_back(event);
	}

	/**
	 * Adds the given event to the end of this EventStore.
	 * @param event The event to be added
	 * @return A reference to this EventStore.
	 */
	inline AddressableEventStorage &operator<<(const EventType &event) {
		push_back(event);
		return *this;
	}

	/**
	 * Returns the total size of the EventStore.
	 * @return The total size (in events) of the packet.
	 */
	[[nodiscard]] inline size_t size() const noexcept {
		return totalLength_;
	}

	/**
	 * Returns a new EventStore which is a shallow representation of
	 * a slice of this EventStore. The slice is from `start` (number of,
	 * events, minimum 0, maximum `getLength()`) and has a length of `length`.
	 *
	 * As a slice is a shallow representation, no EventData gets copied by
	 * this operation. The resulting EventStore receives shared ownership
	 * over the relevant parts of the data. Should the original EventStore
	 * get out of scope, memory that is not relevant to the sliced EventStore
	 * will get freed.
	 *
	 * @param start The start index of the slice (in number of events)
	 * @param length The desired length of the slice (in number of events)
	 * @return A new EventStore object which references to the sliced,
	 * shared data. No Event data is copied.
	 */
	[[nodiscard]] AddressableEventStorage slice(const size_t start, const size_t length) const {
		if (start + length > totalLength_) {
			throw std::range_error("Slice exceeds EventStore range");
		}

		if (length == 0) {
			return AddressableEventStorage();
		}

		std::vector<PartialEventDataType> newPartials;
		auto lowerPartial = std::upper_bound(partialOffsets_.begin(), partialOffsets_.end(), start);
		auto upperPartial = std::lower_bound(partialOffsets_.begin(), partialOffsets_.end(), start + length);
		auto lowIndex     = static_cast<size_t>(lowerPartial - partialOffsets_.begin()) - 1;
		auto highIndex    = static_cast<size_t>(upperPartial - partialOffsets_.begin());
		for (size_t i = lowIndex; i < highIndex; i++) {
			newPartials.emplace_back(dataPartials_[i]);
		}
		size_t frontSliceAmount = start - partialOffsets_[lowIndex];
		size_t backSliceAmount  = partialOffsets_[highIndex - 1] + newPartials.back().getLength() - (start + length);
		newPartials.front().sliceFront(frontSliceAmount);
		newPartials.back().sliceBack(backSliceAmount);

		if (newPartials.front().getLength() <= 0) {
			newPartials.erase(newPartials.begin());
		}

		if (newPartials.back().getLength() <= 0) {
			newPartials.erase(newPartials.end() - 1);
		}

		return AddressableEventStorage(newPartials);
	}

	/**
	 * Returns a new EventStore which is a shallow representation of
	 * a slice of this EventStore. The slice is from `start` (number of,
	 * events, minimum 0, maximum `getLength()`) and goes to the end of the
	 * EventStore. This method slices off the front of an EventStore.
	 *
	 * As a slice is a shallow representation, no EventData gets copied by
	 * this operation. The resulting EventStore receives shared ownership
	 * over the relevant parts of the data. Should the original EventStore
	 * get out of scope, memory that is not relevant to the sliced EventStore
	 * will get freed.
	 *
	 * @param start The start index of the slice (in number of events). The
	 * slice will be from this index to the end of the packet.
	 * @return A new EventStore object which references to the sliced,
	 * shared data. No Event data is copied.
	 */
	[[nodiscard]] AddressableEventStorage<EventType, EventPacketType> slice(const size_t start) const {
		if (start == 0 || totalLength_ == 0) {
			return *this;
		}

		if (start >= totalLength_) {
			return AddressableEventStorage<EventType, EventPacketType>();
		}

		auto lowerPartial = std::upper_bound(partialOffsets_.begin(), partialOffsets_.end(), start);
		auto lowIndex     = static_cast<size_t>(lowerPartial - partialOffsets_.begin()) - 1;
		std::vector<PartialEventDataType> newPartials(dataPartials_.begin() + lowIndex, dataPartials_.end());
		size_t frontSliceAmount = start - partialOffsets_[lowIndex];
		newPartials.front().sliceFront(frontSliceAmount);

		if (newPartials.front().getLength() <= 0) {
			newPartials.erase(newPartials.begin());
		}

		return AddressableEventStorage(newPartials);
	}

	/**
	 * Returns a new EventStore which is a shallow representation of
	 * a slice of this EventStore. The slice is from a specific startTime (in
	 * event timestamps, microseconds) to a specific endTime (event timestamps,
	 * microseconds). The actual size (in events) of the resulting packet
	 * depends on the event rate in the requested time interval. The resulting
	 * packet may be empty, if there is no event that happened in the requested
	 * interval.
	 *
	 * As a slice is a shallow representation, no EventData gets copied by
	 * this operation. The resulting EventStore receives shared ownership
	 * over the relevant parts of the data. Should the original EventStore
	 * get out of scope, memory that is not relevant to the sliced EventStore
	 * will get freed.
	 *
	 * The sliced output will be in the time range [startTime, endTime), endTime
	 * is exclusive.
	 * @param startTime The start time of the required slice (inclusive)
	 * @param endTime The end time of the required time (exclusive)
	 * @param retStart parameter that will get set to the actual index (in
	 * number of events) at which the start of the slice occured.
	 * @param retEnd parameter that will get set to the actual index (in
	 * number of events) at which the end of the slice occured
	 * @return A new EventStore object that is a shallow representation
	 * to the sliced, shared data. No data is copied over.
	 */
	AddressableEventStorage sliceTime(
		const int64_t startTime, const int64_t endTime, size_t &retStart, size_t &retEnd) const {
		// we find the relevant partials and slice the first and last one to fit
		std::vector<PartialEventDataType> newPartials;

		auto lowerPartial = std::lower_bound(dataPartials_.begin(), dataPartials_.end(), startTime,
			PartialEventDataTimeComparator<EventType, EventPacketType>(false));
		auto upperPartial = std::lower_bound(dataPartials_.begin(), dataPartials_.end(), endTime,
			PartialEventDataTimeComparator<EventType, EventPacketType>(true));

		size_t newLength = 0;
		for (auto it = lowerPartial; it < upperPartial; it++) {
			newLength += it->getLength();
			newPartials.emplace_back(*it);
		}

		if (newLength == 0) {
			// We are returning an empty slice, return indices are set to zeros
			retStart = 0;
			retEnd   = 0;

			return AddressableEventStorage();
		}

		size_t cutFront = newPartials.front().sliceTimeFront(startTime);
		size_t cutBack  = newPartials.back().sliceTimeBack(endTime);
		newLength       = newLength - cutFront - cutBack;

		if (newPartials.front().getLength() <= 0) {
			newPartials.erase(newPartials.begin());
		}

		if (!newPartials.empty() && newPartials.back().getLength() <= 0) {
			newPartials.erase(newPartials.end() - 1);
		}

		retStart = partialOffsets_[static_cast<size_t>(lowerPartial - dataPartials_.begin())] + cutFront;
		retEnd   = retStart + newLength;

		return AddressableEventStorage(newPartials);
	}

	/**
	 * Returns a new EventStore which is a shallow representation of
	 * a slice of this EventStore. The slice is from a specific startTime (in
	 * event timestamps, microseconds) to a specific endTime (event timestamps,
	 * microseconds). The actual size (in events) of the resulting packet
	 * depends on the event rate in the requested time interval. The resulting
	 * packet may be empty, if there is no event that happend in the requested
	 * interval.
	 *
	 * As a slice is a shallow representation, no EventData gets copied by
	 * this operation. The resulting EventStore receives shared ownership
	 * over the relevant parts of the data. Should the original EventStore
	 * get out of scope, memory that is not relevant to the sliced EventStore
	 * will get freed.
	 *
	 * The sliced output will be in the time range [startTime, endTime), endTime
	 * is exclusive.
	 * @param startTime The start time of the required slice (inclusive)
	 * @param endTime The end time of the required time (exclusive)
	 * @return A new EventStore object that is a shallow representation
	 * to the sliced, shared data. No data is copied over.
	 */
	[[nodiscard]] AddressableEventStorage sliceTime(const int64_t startTime, const int64_t endTime) const {
		size_t retStart, retEnd;
		return sliceTime(startTime, endTime, retStart, retEnd);
	}

	/**
	 * Returns a new EventStore which is a shallow representation of
	 * a slice of this EventStore. Returns a slice which contains events
	 * from the back of the storage, it will contain no more events than
	 * given length variable.
	 *
	 * As a slice is a shallow representation, no EventData gets copied by
	 * this operation. The resulting EventStore receives shared ownership
	 * over the relevant parts of the data. Should the original EventStore
	 * get out of scope, memory that is not relevant to the sliced EventStore
	 * will get freed.
	 * @param length Maximum number of events contained in the resulting slice.
	 * @return A new EventStore object that is a shallow representation
	 * to the sliced, shared data. No data is copied over.
	 */
	[[nodiscard]] AddressableEventStorage sliceBack(const size_t length) const {
		if (length >= totalLength_) {
			return slice(0);
		}
		else {
			return slice(totalLength_ - length, length);
		}
	}

	/**
	 * Returns a new EventStore which is a shallow representation of
	 * a slice of this EventStore. The slice is from a specific startTime (in
	 * event timestamps, microseconds) to the end of the packet.
	 * The actual size (in events) of the resulting packet
	 * depends on the event rate in the requested time interval. The resulting
	 * packet may be empty, if there is no event that happened in the requested
	 * interval.
	 *
	 * As a slice is a shallow representation, no EventData gets copied by
	 * this operation. The resulting EventStore receives shared ownership
	 * over the relevant parts of the data. Should the original EventStore
	 * get out of scope, memory that is not relevant to the sliced EventStore
	 * will get freed.
	 * @param startTime The start time of the required slice, if positive. If negative, the
	 * number of microseconds from the end of the store
	 * @return A new EventStore object that is a shallow representation
	 * to the sliced, shared data. No data is copied over.
	 */
	[[nodiscard]] AddressableEventStorage sliceTime(const int64_t startTime) const {
		int64_t s = startTime < 0 ? (getHighestTime() + startTime) : startTime;
		return sliceTime(s, getHighestTime() + 1); // + 1 to include the events that happen at the last time.
	}

	/**
	 * Slices events from back of the EventStore, so that the EventStore would only contain a number of events of a
	 * given event rate. Useful for performance limited applications when it is required to limit the rate of events
	 * to maintain stable execution time.
	 * @param targetRate Target event rate in events per second.
	 * @return New event store which contains number of events within the target event rate.
	 */
	[[nodiscard]] AddressableEventStorage sliceRate(const double targetRate) const {
		if (targetRate <= 0.0) {
			throw dv::exceptions::InvalidArgument<double>(
				"sliceRate() expects a positive target rate value.", targetRate);
		}

		if (rate() < targetRate) {
			return *this;
		}

		const auto durationSeconds = std::chrono::duration<double>(duration()).count();
		const auto count           = static_cast<size_t>(targetRate * durationSeconds);
		return sliceBack(count);
	}

	/**
	 * Returns an iterator to the begin of the EventStore
	 * @return an iterator to the begin of the EventStore
	 */
	[[nodiscard]] const_iterator begin() const noexcept {
		return (iterator(&dataPartials_, true));
	}

	/**
	 * Returns an iterator to the end of the EventStore
	 * @return  an iterator to the end of the EventStore
	 */
	[[nodiscard]] const_iterator end() const noexcept {
		return (iterator(&dataPartials_, false));
	}

	/**
	 * Returns a reference to the first element of the packet
	 * @return a reference to the first element to the packet
	 */
	[[nodiscard]] const_reference front() const {
		return *iterator(&dataPartials_, true);
	}

	/**
	 * Returns a reference to the last element of the packet
	 * @return a reference to the last element to the packet
	 */
	[[nodiscard]] const_reference back() const {
		iterator it(&dataPartials_, false);
		it -= 1;
		return *it;
	}

	/**
	 * Returns the timestamp of the first event in the packet.
	 * This is also the lowest timestamp in the packet, as
	 * the events are required to be monotonic.
	 * @return The lowest timestamp present in the packet. 0 if the packet is
	 * empty.
	 */
	[[nodiscard]] inline int64_t getLowestTime() const {
		if (isEmpty()) {
			return 0;
		}
		return dataPartials_.front().getLowestTime();
	}

	/**
	 * Returns the timestamp of the last event in the packet.
	 * This is also the highest timestamp in the packet, as
	 * the events are required to be monotonic.
	 * @return The highest timestamp present in the packet. 0 if the packet
	 * is empty
	 */
	[[nodiscard]] inline int64_t getHighestTime() const {
		if (isEmpty()) {
			return 0;
		}
		return dataPartials_.back().getHighestTime();
	}

	/**
	 * Returns the total length (in number of events) of the packet
	 * @return the total number of events present in the packet.
	 */
	[[nodiscard]] [[deprecated("Use size() instead.")]] inline size_t getTotalLength() const {
		return totalLength_;
	}

	/**
	 * Returns true if the packet is empty (does not contain any events).
	 * @return Returns true if the packet is empty (does not contain any events).
	 */
	[[nodiscard]] inline bool isEmpty() const {
		return totalLength_ == 0;
	}

	/**
	 * Erase given range of events from the event store. This does not necessarily delete the underlying data
	 * since event store maps the data using smart pointers, the data will be cleared only in the case that
	 * none of the stores is mapping the data. This erase function does not affect data shared with other
	 * event stores.
	 * @param start     Start index of events to erase
	 * @param length    Number of events to erase
	 */
	void erase(const size_t start, const size_t length) {
		if (start > totalLength_) {
			throw std::out_of_range("First index is beyond the size of the store");
		}

		if ((start + length) > totalLength_) {
			throw std::out_of_range("Erase range exceeds EventStore range");
		}

		if (length == 0) {
			return;
		}

		// Find everything for indexing
		const auto lowerPartial = std::upper_bound(partialOffsets_.begin(), partialOffsets_.end(), start);
		auto upperPartial       = std::lower_bound(partialOffsets_.begin(), partialOffsets_.end(), start + length);
		const auto lowIndex     = static_cast<size_t>(std::distance(partialOffsets_.begin(), lowerPartial) - 1);
		const auto highIndex    = static_cast<size_t>(std::distance(partialOffsets_.begin(), upperPartial));
		auto lowerIter          = std::next(dataPartials_.begin(), static_cast<ptrdiff_t>(lowIndex));
		auto upperIter          = std::next(dataPartials_.begin(), static_cast<ptrdiff_t>(highIndex));

		if ((highIndex - lowIndex) == 1) {
			// We are dealing with changes within a single partial

			// If we are dealing within the last packet, partial offsets will not contain
			// the value for it
			size_t highLimit;
			if (highIndex < partialOffsets_.size()) {
				highLimit = partialOffsets_[highIndex];
			}
			else {
				highLimit = totalLength_;
			}

			if (start == partialOffsets_[lowIndex]) {
				// Erase off front
				lowerIter->sliceFront(std::min(length, lowerIter->getLength()));
			}
			else if ((start + length) == highLimit) {
				// Erase off back
				lowerIter->sliceBack(std::min(length, lowerIter->getLength()));
			}
			else if ((start + length) < highLimit) {
				// Erase in between. Make a copy of the packet and erase back of first
				// and erase front of second.
				const size_t correctedStart = (start - partialOffsets_[lowIndex]);

				dv::PartialEventData copy = *lowerIter;
				copy.sliceFront(correctedStart + length);

				lowerIter->sliceBack(lowerIter->getLength() - correctedStart);
				upperIter = dataPartials_.insert(lowerIter + 1, copy);
				lowerIter = std::next(dataPartials_.begin(), static_cast<ptrdiff_t>(lowIndex));
			}

			// Remove the empty partials if that is required
			if (lowerIter->getLength() <= 0) {
				lowerIter = dataPartials_.erase(lowerIter);
			}
			else {
				lowerIter++;
			}
		}
		else {
			// Erase events spanning within several partials
			if (start > partialOffsets_[lowIndex]) {
				lowerIter->sliceBack(lowerIter->getLength() - (start - partialOffsets_[lowIndex]));
				lowerIter++;
			}

			// Go to the last affected partial and delete
			upperIter--;
			upperPartial--;

			const size_t upperBackSliceAmount = std::min((start + length) - *upperPartial, upperIter->getLength());
			upperIter->sliceFront(upperBackSliceAmount);

			if (upperIter->getLength() <= 0) {
				upperIter = dataPartials_.erase(upperIter);
			}
		}

		// Erase the data in between the bounds
		if (!dataPartials_.empty() && lowerIter != upperIter) {
			dataPartials_.erase(lowerIter, upperIter);
		}

		// Rebuild the partials offset LUT
		partialOffsets_.erase(lowerPartial, partialOffsets_.end());
		totalLength_ = partialOffsets_.back();

		auto dataIter = std::next(dataPartials_.begin(), static_cast<ptrdiff_t>(partialOffsets_.size() - 1));
		partialOffsets_.reserve(dataPartials_.size());

		while (dataIter != dataPartials_.end()) {
			totalLength_ += dataIter->getLength();
			partialOffsets_.emplace_back(totalLength_);
			dataIter++;
		}
	}

	/**
	 * Erase events in the range between given timestamps. This does not necessarily delete the underlying data
	 * since event store maps the data using smart pointers, the data will be cleared only in the case that
	 * none of the stores is mapping the data. This erase function does not affect data shared with other
	 * event stores.
	 * @param startTime     Start timestamp for events to be erased, including this exact timestamp
	 * @param endTime       End timestamp for events to be erased, up to this time, events with this exact timestamp
	 *                      are not going to be erased.
	 * @return              Number of events deleted
	 */
	size_t eraseTime(const int64_t startTime, const int64_t endTime) {
		if (startTime > endTime) {
			throw std::invalid_argument("Start time is greater than end time in eraseTime function call");
		}

		const auto lowerPartial = std::lower_bound(dataPartials_.begin(), dataPartials_.end(), startTime,
			PartialEventDataTimeComparator<EventType, EventPacketType>(false));
		auto upperPartial       = std::lower_bound(dataPartials_.begin(), dataPartials_.end(), endTime,
				  PartialEventDataTimeComparator<EventType, EventPacketType>(true));

		if (upperPartial == dataPartials_.end()) {
			upperPartial--;
		}

		const auto cutFront
			= static_cast<size_t>(std::distance(lowerPartial->begin(), lowerPartial->iteratorAtTime(startTime)));
		const auto cutBack
			= static_cast<size_t>(std::distance(upperPartial->begin(), upperPartial->iteratorAtTime(endTime)));
		const auto partialIndexStart = static_cast<size_t>(std::distance(dataPartials_.begin(), lowerPartial));
		const auto partialIndexEnd   = static_cast<size_t>(std::distance(dataPartials_.begin(), upperPartial));

		const auto eraseIndexStart = partialOffsets_[partialIndexStart] + cutFront;
		const auto eraseLength     = (partialOffsets_[partialIndexEnd] + cutBack) - eraseIndexStart;
		erase(eraseIndexStart, eraseLength);

		return eraseLength;
	}

	/**
	 * Return an event at given index.
	 * @param index     Index of the event
	 * @return          Reference to the event at the index.
	 */
	[[nodiscard]] const EventType &operator[](const size_t index) const {
		dv::runtime_assert(index < totalLength_, "Index exceeds EventStore range");

		const auto lowerPartial = std::upper_bound(partialOffsets_.begin(), partialOffsets_.end(), index);
		const auto lowIndex     = static_cast<size_t>(std::distance(partialOffsets_.begin(), lowerPartial) - 1);

		return dataPartials_[lowIndex][index - partialOffsets_[lowIndex]];
	}

	/**
	 * Return an event at given index.
	 * @param index     Index of the event
	 * @return          Reference to the event at the index.
	 */
	[[nodiscard]] const EventType &at(const size_t index) const {
		if (index >= totalLength_) {
			throw std::out_of_range("Index exceeds EventStore range");
		}

		const auto lowerPartial = std::upper_bound(partialOffsets_.begin(), partialOffsets_.end(), index);
		const auto lowIndex     = static_cast<size_t>(std::distance(partialOffsets_.begin(), lowerPartial) - 1);

		return dataPartials_[lowIndex][index - partialOffsets_[lowIndex]];
	}

	/**
	 * Retain a certain duration of event data in the event store. This will retain latest events and delete oldest
	 * data. The duration is just a hint of minimum amount of duration to keep, the exact duration will always be
	 * slightly greater (depending on event rate and memory allocation).
	 * @param duration  Minimum amount of time to keep in the event store. Events are erased in batches, so this
	 *                  guarantees only to maintain the batches of events within this duration.
	 */
	void retainDuration(const dv::Duration duration) {
		const auto startTime = getHighestTime() - duration.count();
		auto lowerPartial    = std::lower_bound(dataPartials_.begin(), dataPartials_.end(), startTime,
			   PartialEventDataTimeComparator<EventType, EventPacketType>(false));

		if (lowerPartial != dataPartials_.begin()) {
			dataPartials_.erase(dataPartials_.begin(), --lowerPartial);
			partialOffsets_.clear();
			partialOffsets_.reserve(dataPartials_.size());
			totalLength_ = 0;

			for (const auto &partial : dataPartials_) {
				partialOffsets_.push_back(totalLength_);
				totalLength_ += partial.getLength();
			}
		}
	}

	/**
	 * Get the duration of events contained.
	 * @return  Duration of stored events in microseconds.
	 */
	[[nodiscard]] dv::Duration duration() const {
		return dv::Duration(getHighestTime() - getLowestTime());
	}

	/**
	 * Checks whether given timestamp is within the time range of the event store.
	 * @param timestamp Microsecond Unix timestamp to check.
	 * @return True if the timestamp is within the time of event store, false otherwise.
	 */
	[[nodiscard]] bool isWithinStoreTimeRange(const int64_t timestamp) const {
		return timestamp >= getLowestTime() && timestamp <= getHighestTime();
	}

	/**
	 * Get currently used default shard (data partial) capacity value.
	 * @return 		Default capacity for new shards.
	 */
	[[nodiscard]] size_t getShardCapacity() const {
		return shardCapacity_;
	}

	/**
	 * Set a new capacity for shards (data partials). Setting this value does not affect already allocated shards
	 * and will be used only when a new shard needs to be allocated. If passed in capacity is set to 0, the setter
	 * will use a capacity value of 1, because that is the lowest allowed capacity value.
	 * @param shardCapacity 	Capacity of events for newly allocated shards.
	 */
	void setShardCapacity(const size_t shardCapacity) {
		shardCapacity_ = std::max<size_t>(1ULL, shardCapacity);
	}

	/**
	 * Get the amount of shards that are currently referenced by the event store.
	 * @return 		Number of referenced shards (data partials).
	 */
	[[nodiscard]] size_t getShardCount() const {
		return dataPartials_.size();
	}

	/**
	 * Get the event rate (events per second) for the events stored in this storage.
	 * @return 		Events per second within this storage.
	 */
	[[nodiscard]] double rate() const {
		const int64_t durationMicros = getHighestTime() - getLowestTime();
		if (durationMicros == 0) {
			return 0.;
		}
		return static_cast<double>(size()) / (static_cast<double>(durationMicros) * 1e-6);
	}

	/**
	 * Convert event store into a continuous memory packet. This performs a deep copy of underlying data.
	 * @return Event packet with a copy of all stored events in this event store.
	 */
	[[nodiscard]] EventPacketType toPacket() const {
		EventPacketType packet;
		packet.elements.reserve(size());
		for (const auto &event : *this) {
			packet.elements.push_back(event);
		}
		return packet;
	}

	friend std::ostream &operator<<(std::ostream &os, const AddressableEventStorage &storage) {
		os << fmt::format("EventStore containing {} events within {} duration; time range within [{}; {}]",
			storage.size(), storage.duration(), storage.getLowestTime(), storage.getHighestTime());
		return os;
	}
};

using EventStore          = AddressableEventStorage<dv::Event, dv::EventPacket>;
using DepthEventStore     = dv::AddressableEventStorage<dv::DepthEvent, dv::DepthEventPacket>;
using EventStreamSlicer   = StreamSlicer<EventStore>;
using FrameStreamSlicer   = StreamSlicer<dv::cvector<dv::Frame>>;
using IMUStreamSlicer     = StreamSlicer<dv::cvector<dv::IMU>>;
using TriggerStreamSlicer = StreamSlicer<dv::cvector<dv::Trigger>>;

static_assert(dv::concepts::EventStorage<EventStore>);
static_assert(dv::concepts::EventStorage<DepthEventStore>);

/**
 * TimeSurface class that builds the surface of the occurrences of the last
 * timestamps.
 */
template<class EventStoreType, typename ScalarType = int64_t>
class TimeSurfaceBase {
public:
	using Scalar = ScalarType;

	/**
	 * Dummy constructor
	 * Constructs a new, empty TimeSurface without any data allocated to it.
	 */
	TimeSurfaceBase() = default;

	/**
	 * Creates a new TimeSurface with the given size. The Mat is zero initialized
	 * @param rows The number of rows of the TimeSurface
	 * @param cols The number of cols of the TimeSurface
	 */
	explicit TimeSurfaceBase(const uint32_t rows, const uint32_t cols) :
		mData(Eigen::Matrix<ScalarType, Eigen::Dynamic, Eigen::Dynamic>::Zero(rows, cols)) {
	}

	/**
	 * Creates a new TimeSurface of the given size. The Mat is zero initialized.
	 * @param size The opencv size to be used to initialize
	 */
	explicit TimeSurfaceBase(const cv::Size &size) :
		TimeSurfaceBase(static_cast<uint32_t>(size.height), static_cast<uint32_t>(size.width)) {
	}

	/**
	 * Copy constructor, constructs a new time surface with shared ownership of the data.
	 * @param other The time surface to be copied. The data is not copied but takes shared ownership.
	 */
	TimeSurfaceBase(const TimeSurfaceBase &other) = default;

	/**
	 * Destructor
	 */
	virtual ~TimeSurfaceBase() = default;

	/**
	 * Inserts the event store into the time surface.
	 * @param store The event store to be added
	 * @return A reference to this TimeSurfaceBase.
	 */
	virtual TimeSurfaceBase &operator<<(const EventStoreType &store) {
		accept(store);
		return *this;
	}

	/**
	 * Inserts the event into the time surface.
	 * @param event The event to be added
	 * @return A reference to this TimeSurfaceBase.
	 */
	virtual TimeSurfaceBase &operator<<(const typename EventStoreType::iterator::value_type &event) {
		accept(event);
		return *this;
	}

	/**
	 * Generates a frame from the data contained in the event store
	 * @param mat The storage where the frame should be generated
	 * @return A reference to the generated frame.
	 */
	dv::Frame &operator>>(dv::Frame &mat) const {
		mat = generateFrame();
		return mat;
	}

	/**
	 * Inserts the event store into the time surface.
	 * @param store The event store to be added
	 */
	virtual void accept(const EventStoreType &store) {
		for (const auto &event : store) {
			accept(event);
		}
	}

	/**
	 * Inserts the event into the time surface.
	 * @param event The event to be added
	 */
	virtual void accept(const typename EventStoreType::iterator::value_type &event) {
		at(event.y(), event.x()) = event.timestamp();
	}

	/**
	 * Returns a const reference to the element at the given coordinates.
	 * The element can only be read from
	 * @param y The y coordinate of the element to be accessed.
	 * @param x The x coordinate of the element to be accessed.
	 * @return A const reference to the element at the requested coordinates.
	 */
	[[nodiscard]] const ScalarType &at(const int16_t y, const int16_t x) const {
		if (!(y >= 0 && y < rows() && x >= 0 && x < cols())) {
			throw std::range_error("Attempted to access out-of-range value in TimeSurfaceBase.");
		}

		return mData(y, x);
	}

	/**
	 * Returns a reference to the element at the given coordinates.
	 * The element can both be read from as well as written to.
	 * @param y The y coordinate of the element to be accessed.
	 * @param x The x coordinate of the element to be accessed.
	 * @return A reference to the element at the requested coordinates.
	 */
	[[nodiscard]] ScalarType &at(const int16_t y, const int16_t x) {
		if (!(y >= 0 && y < rows() && x >= 0 && x < cols())) {
			throw std::range_error("Attempted to access out-of-range value in TimeSurfaceBase.");
		}

		return mData(y, x);
	}

	/**
	 * Returns a const reference to the element at the given coordinates.
	 * The element can only be read from
	 * @param y The y coordinate of the element to be accessed.
	 * @param x The x coordinate of the element to be accessed.
	 * @return A const reference to the element at the requested coordinates.
	 */
	[[nodiscard]] const ScalarType &operator()(const int16_t y, const int16_t x) const noexcept {
		dv::runtime_assert(y >= 0 && y < rows() && x >= 0 && x < cols(), "address out of range");
		return mData(y, x);
	}

	/**
	 * Returns a reference to the element at the given coordinates.
	 * The element can both be read from as well as written to.
	 * @param y The y coordinate of the element to be accessed.
	 * @param x The x coordinate of the element to be accessed.
	 * @return A reference to the element at the requested coordinates.
	 */
	[[nodiscard]] ScalarType &operator()(const int16_t y, const int16_t x) noexcept {
		dv::runtime_assert(y >= 0 && y < rows() && x >= 0 && x < cols(), "address out of range");
		return mData(y, x);
	}

	/**
	 * Returns a block of the time surface
	 * @param topRow the row coordinate at the top of the block
	 * @param leftCol the column coordinate at the left of the block
	 * @param height the height of the block
	 * @param width the width of the block
	 * @return the block
	 */
	[[nodiscard]] auto block(
		const int16_t topRow, const int16_t leftCol, const int16_t height, const int16_t width) const {
		return mData.block(topRow, leftCol, height, width);
	}

	/**
	 * Returns a block of the time surface
	 * @param topRow the row coordinate at the top of the block
	 * @param leftCol the column coordinate at the left of the block
	 * @param height the height of the block
	 * @param width the width of the block
	 * @return the block
	 */
	[[nodiscard]] auto block(const int16_t topRow, const int16_t leftCol, const int16_t height, const int16_t width) {
		return mData.block(topRow, leftCol, height, width);
	}

	/**
	 * Generates a frame from the data contained in the event store
	 * @return The generated frame.
	 */
	[[nodiscard]] dv::Frame generateFrame() const {
		dv::Frame frame(mData.maxCoeff(), getOCVMatScaled<uint8_t>());
		frame.source = dv::FrameSource::ACCUMULATION;
		return frame;
	}

	/**
	 * Creates a new OpenCV matrix of the type given and copies the time data into this OpenCV matrix.
	 * This version does only subtracts an offset from the values for them to fit into the value range of the requested
	 * frame type. Therefore this method preserves the units of the timestamps that are contained in the
	 * time surface.
	 *
	 * The data in the time surface is of signed 64bit integer type. There is no OpenCV type that can hold the full
	 * range of these values. Therefore, the returned data is a pair of an OpenCV Mat, of a type that can be chosen by
	 * the user, and an offset of signed 64bit integer, which contains the offset that can be added to each pixel value
	 * so that their values are in units of microseconds.
	 * @tparam T The type of the OpenCV Mat to be generated.
	 * @return An OpenCV Mat of the requested type, as well as an offset which can be added to the matrix in order for
	 * the data  to be in microseconds.
	 */
	template<class T = uint8_t>
	requires std::integral<T>
	[[nodiscard]] std::pair<cv::Mat, int64_t> getOCVMat() const {
		static_assert(sizeof(T) <= 4, "This method is intended only for integral types of <=32bit size");

		const auto [minTimeStamp, maxTimestamp] = std::accumulate(mData.data(), mData.data() + mData.size(),
			std::make_pair(std::numeric_limits<int64_t>::max(), std::numeric_limits<int64_t>::min()),
			[](const std::pair<int64_t, int64_t> &prev, const int64_t &next) {
				return std::make_pair(next != 0 && next < prev.first ? next : prev.first,
					next != 0 && next > prev.second ? next : prev.second);
			});

		const int64_t offset = maxTimestamp - static_cast<int64_t>(std::numeric_limits<T>::max());

		const Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic> tsDiff
			= (mData.array() - offset).max(std::numeric_limits<T>::min()).matrix().template cast<T>();

		cv::Mat mat(tsDiff.rows(), tsDiff.cols(), cv::DataType<T>::type);
		cv::eigen2cv(tsDiff, mat);

		return std::make_pair(mat, offset);
	}

	/**
	 * Creates a new OpenCV matrix of the type given and copies the time data into this OpenCV matrix.
	 * This version scales the values for them to fit into the value range of the requested frame type. Therefore the
	 * units of the timestamps are not preserved.
	 *
	 * The data in the time surface is of signed 64bit integer type. There is no OpenCV type that can hold the full
	 * range of these values. Therefore, the returned data is a pair of an OpenCV Mat, of a type that can be chosen by
	 * the user, and an offset of signed 64bit integer, which contains the offset that can be added to each pixel value
	 * so that their values are in units of microseconds.
	 * @tparam T The type of the OpenCV Mat to be generated.
	 * @param lookBackOverride override the amount of time to look back into the past. Defaults to the complete range
	 * contained in the time surface. The unit of the parameter is the unit of time contained in the TimeSurface.
	 * @return An OpenCV Mat of the requested type, as well as an offset which can be added to the matrix in order for
	 * the data  to be in microseconds.
	 */
	template<typename T = uint8_t>
	requires std::integral<T>
	[[nodiscard]] cv::Mat getOCVMatScaled(const std::optional<int64_t> lookBackOverride = std::nullopt) const {
		static_assert(sizeof(T) <= 4, "This method is intended only for integral types of <=32bit size");

		const auto [minTimeStamp, maxTimestamp] = std::accumulate(mData.data(), mData.data() + mData.size(),
			std::make_pair(std::numeric_limits<int64_t>::max(), std::numeric_limits<int64_t>::min()),
			[](const std::pair<int64_t, int64_t> &prev, const int64_t &next) {
				return std::make_pair(next != 0 && next < prev.first ? next : prev.first,
					next != 0 && next > prev.second ? next : prev.second);
			});

		const auto lookback = lookBackOverride.has_value() ? *lookBackOverride : maxTimestamp - minTimeStamp;
		const auto minTimestampOverride
			= lookBackOverride.has_value() ? maxTimestamp - *lookBackOverride : minTimeStamp;

		const Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic> tsDiffScaled
			= ((mData.template cast<double>().array() - minTimestampOverride)
					* ((static_cast<double>(std::numeric_limits<T>::max())
						   - static_cast<double>(std::numeric_limits<T>::min()))
						/ static_cast<double>(lookback))
				+ static_cast<double>(std::numeric_limits<T>::min()))
				  .max(std::numeric_limits<T>::min())
				  .min(std::numeric_limits<T>::max())
				  .matrix()
				  .template cast<T>();

		cv::Mat mat(
			static_cast<int>(tsDiffScaled.rows()), static_cast<int>(tsDiffScaled.cols()), cv::DataType<T>::type);
		cv::eigen2cv(tsDiffScaled, mat);

		return mat;
	}

	/**
	 * Sets all values in the time surface to zero
	 */
	void reset() {
		mData.setZero();
	}

	/**
	 * Adds a constant to the time surface.
	 * Values are bounds checked to 0. If the new time would become negative,
	 * it is set to 0.
	 * @tparam T The type of the constant. Accepts any numeric type.
	 * @param s The constant to be added
	 * @return A new TimeSurfaceBase with the changed times
	 */
	template<typename T>
	TimeSurfaceBase operator+(const T &s) const {
		TimeSurfaceBase tm(mData.rows(), mData.cols());
		addImpl(static_cast<const ScalarType>(s), tm);
		return tm;
	}

	/**
	 * Adds a constant to the TimeSurface.
	 * Values are bounds checked to 0. If the new time would become negative,
	 * it is set to 0.
	 * @tparam T The type of the constant. Accepts any numeric type.
	 * @param s The constant to be added
	 * @return A reference to the TimeSurfaceBase
	 */
	template<typename T>
	TimeSurfaceBase &operator+=(const T &s) {
		addImpl(static_cast<const ScalarType>(s), *this);
		return *this;
	}

	/**
	 * Subtracts a constant from the TimeSurface.
	 * Values are bounds checked to 0. If the new time would become negative,
	 * it is set to 0.
	 * @tparam T The type of the constant. Accepts any numeric type.
	 * @param s The constant to be subtracted
	 * @return A reference to the TimeSurfaceBase
	 */
	template<typename T>
	TimeSurfaceBase operator-(const T &s) const {
		TimeSurfaceBase tm(mData.rows(), mData.cols());
		addImpl(static_cast<const ScalarType>(-s), tm);
		return tm;
	}

	/**
	 * Subtracts a constant from the TimeSurface.
	 * Values are bounds checked to 0. If the new time would become negative,
	 * it is set to 0.
	 * @tparam T The type of the constant. Accepts any numeric type.
	 * @param s The constant to be subtracted
	 * @return A reference to the TimeSurfaceBase
	 */
	template<typename T>
	TimeSurfaceBase &operator-=(const T &s) {
		addImpl(static_cast<const ScalarType>(-s), *this);
		return *this;
	}

	/**
	 * Assigns constant to the TimeSurface.
	 * Values are bounds checked to 0. If the new time would become negative,
	 * it is set to 0.
	 * @tparam T The type of the constant. Accepts any numeric type.
	 * @param s The constant to be subtracted
	 * @return A reference to the TimeSurfaceBase
	 */
	template<typename T>
	TimeSurfaceBase &operator=(const T &s) {
		mData.setConstant(s);
		return *this;
	}

	/**
	 * The size of the TimeSurface.
	 * @return Returns the size of this time matrix as an opencv size
	 */
	[[nodiscard]] cv::Size size() const noexcept {
		return {static_cast<int>(mData.cols()), static_cast<int>(mData.rows())};
	}

	/**
	 * Returns the number of rows of the TimeSurface
	 * @return the number of rows
	 */
	[[nodiscard]] int16_t rows() const noexcept {
		return static_cast<int16_t>(mData.rows());
	}

	/**
	 * Returns the number of columns of the TimeSurface
	 * @return the number of columns
	 */
	[[nodiscard]] int16_t cols() const noexcept {
		return static_cast<int16_t>(mData.cols());
	}

	/**
	 * Returns true if the TimeSurface has zero size. In this case, it was not
	 * allocated with a size.
	 * @return true if the TimeSurface does not have a size > 0
	 * @deprecated Use isEmpty() instead.
	 * @sa TimeSurfaceBase::isEmpty()
	 */
	[[deprecated("Use isEmpty() instead.")]] [[nodiscard]] bool empty() const noexcept {
		return isEmpty();
	}

	/**
	 * Returns true if the TimeSurface has zero size. In this case, it was not
	 * allocated with a size.
	 * @return true if the TimeSurface does not have a size > 0
	 */
	[[nodiscard]] bool isEmpty() const noexcept {
		return (mData.rows() == 0) || (mData.cols() == 0);
	}

protected:
	Eigen::Matrix<ScalarType, Eigen::Dynamic, Eigen::Dynamic> mData;

	void addImpl(const ScalarType a, TimeSurfaceBase &target) const {
		target.mData = (mData.array() + a).max(0).matrix();
	}
};

using TimeSurface = TimeSurfaceBase<EventStore>;

static_assert(dv::concepts::EventToFrameConverter<TimeSurface, EventStore>);
static_assert(dv::concepts::BlockAccessible<TimeSurface>);
static_assert(dv::concepts::TimeSurface<TimeSurface, EventStore>);

/**
 * A speed invariant time surface, as described by https://arxiv.org/abs/1903.11332
 * @tparam EventStoreType 	Type of underlying event store
 * @tparam patchDiameter	Diameter of the patch to apply the speed invariant update. The paper defines parameter `r`
 * 							which is half of the diameter value, so for an `r = 5`, use diameter = `2 * r` or 10 in
 * 							this case. The update is performed using eigen optimized routines, so the value has limits:
 * 							it has to be in range (0; 16) and divisible by 2. By default set to 8 which gives the best
 * 							performance.
 */
template<class EventStoreType, uint32_t patchDiameter = 8, typename ScalarType = uint8_t>
class SpeedInvariantTimeSurfaceBase : public TimeSurfaceBase<EventStoreType, ScalarType> {
protected:
	using BaseClassType = TimeSurfaceBase<EventStoreType, ScalarType>;

public:
	/**
	 * Create a speed invariant time surface with known image dimensions.
	 * @param shape Dimensions of the expected event data.
	 */
	explicit SpeedInvariantTimeSurfaceBase(const cv::Size &shape) : BaseClassType(shape) {
		static_assert(patchDiameter > 0 && patchDiameter < 16 && (patchDiameter % 2) == 0);
		mLatestPixelValue = (patchDiameter + 1) * (patchDiameter + 1);
	}

	/**
	 * Inserts the event store into the speed invariant time surface.
	 * @param store The event store to be added
	 * @return A reference to this TimeSurface.
	 */
	SpeedInvariantTimeSurfaceBase &operator<<(const EventStoreType &store) override {
		accept(store);
		return *this;
	}

	/**
	 * Inserts the event into the speed invariant time surface.
	 * @param event The event  to be added
	 * @return A reference to this TimeSurface.
	 */
	SpeedInvariantTimeSurfaceBase &operator<<(const typename EventStoreType::iterator::value_type &event) override {
		accept(event);
		return *this;
	}

	/**
	 * Inserts the event store into the speed invariant time surface.
	 * @param store The event store to be added
	 */
	void accept(const EventStoreType &store) override {
		for (const Event &event : store) {
			accept(event);
		}
	}

	/**
	 * Inserts the event into the speed invariant time surface.
	 * @param event The event  to be added
	 */
	void accept(const typename EventStoreType::iterator::value_type &event) override {
		constexpr int mHalfPatchWidth = patchDiameter / 2;
		auto &currentPixel            = BaseClassType::at(event.y(), event.x());

		if (event.x() > mHalfPatchWidth && event.x() < BaseClassType::cols() - mHalfPatchWidth
			&& event.y() > mHalfPatchWidth && event.y() < BaseClassType::cols() - mHalfPatchWidth) {
			// Safe to apply batch update
			auto area = BaseClassType::mData.template block<patchDiameter, patchDiameter>(
				event.y() - mHalfPatchWidth, event.x() - mHalfPatchWidth);
			Eigen::Matrix<ScalarType, patchDiameter, patchDiameter> largerValues
				= (area.array() > currentPixel).template cast<ScalarType>();
			area -= largerValues;
		}
		else {
			// Close to an edge, resort to a safe, dynamically indexed loop
			const auto rowStart = std::max(0, event.y() - mHalfPatchWidth);
			const auto rowEnd   = std::min(BaseClassType::rows() - 1, event.y() + mHalfPatchWidth);
			const auto colStart = std::max(0, event.x() - mHalfPatchWidth);
			const auto colEnd   = std::min(BaseClassType::cols() - 1, event.x() + mHalfPatchWidth);

			for (int16_t row = rowStart; row <= rowEnd; row++) {
				for (int16_t col = colStart; col <= colEnd; col++) {
					auto &value = BaseClassType::operator()(row, col);
					value       -= value > currentPixel;
				}
			}
		}

		currentPixel = mLatestPixelValue;
	}

private:
	int64_t mLatestPixelValue;
};

using SpeedInvariantTimeSurface = SpeedInvariantTimeSurfaceBase<EventStore>;

static_assert(concepts::EventToFrameConverter<SpeedInvariantTimeSurface, EventStore>);
static_assert(concepts::BlockAccessible<SpeedInvariantTimeSurface>);

} // namespace dv

// fmt compatibility for ostream class printing.
template<>
struct fmt::formatter<dv::EventStore> : fmt::ostream_formatter {};
