#ifndef DV_SDK_TYPES_HPP
#define DV_SDK_TYPES_HPP

#include "../utils.h"

#include <cassert>

#ifdef __cplusplus
constexpr static uint32_t dvTypeIdentifierToId(const char *x) {
	uint32_t ret = static_cast<uint32_t>(x[3]);
	ret          |= (static_cast<uint32_t>(x[2]) << 8);
	ret          |= (static_cast<uint32_t>(x[1]) << 16);
	ret          |= (static_cast<uint32_t>(x[0]) << 24);
	return (ret);
}

constexpr static void dvTypeIdToIdentifier(const uint32_t id, char *x) {
	x[0] = static_cast<char>((id >> 24) & 0x00FF);
	x[1] = static_cast<char>((id >> 16) & 0x00FF);
	x[2] = static_cast<char>((id >> 8) & 0x00FF);
	x[3] = static_cast<char>((id) &0x00FF);
}

extern "C" {
#endif

	struct dvTypeTimeElementExtractor {
		int64_t startTimestamp;
		int64_t endTimestamp;
		int64_t numElements;
	};

	typedef uint32_t (*dvTypePackFuncPtr)(void *toFlatBufferBuilder, const void *fromObject);
	typedef void (*dvTypeUnpackFuncPtr)(void *toObject, const void *fromFlatBuffer);
	typedef void *(*dvTypeConstructPtr)(size_t sizeOfObject);
	typedef void (*dvTypeDestructPtr)(void *object);
	typedef struct dvTypeTimeElementExtractor (*dvTypeTimeElementExtractorPtr)(const void *object);
	typedef bool (*dvTypeUnpackTimeElementRangeFuncPtr)(
		void *toObject, const void *fromFlatBuffer, struct dvTypeTimeElementExtractor range);

	struct dvType {
		uint32_t id;
		const char *identifier;
		const char *description;
		size_t sizeOfType;
		dvTypePackFuncPtr pack;
		dvTypeUnpackFuncPtr unpack;
		dvTypeConstructPtr construct;
		dvTypeDestructPtr destruct;
		dvTypeTimeElementExtractorPtr timeElementExtractor;
		dvTypeUnpackTimeElementRangeFuncPtr unpackTimeElementRange;

#ifdef __cplusplus
		constexpr dvType() :
			id(dvTypeIdentifierToId("NULL")),
			identifier("NULL"),
			description("Placeholder for errors."),
			sizeOfType(0),
			pack(nullptr),
			unpack(nullptr),
			construct(nullptr),
			destruct(nullptr),
			timeElementExtractor(nullptr),
			unpackTimeElementRange(nullptr) {
		}

		constexpr dvType(const char *_identifier, const char *_description, size_t _sizeOfType, dvTypePackFuncPtr _pack,
			dvTypeUnpackFuncPtr _unpack, dvTypeConstructPtr _construct, dvTypeDestructPtr _destruct,
			dvTypeTimeElementExtractorPtr _timeElementExtractor,
			dvTypeUnpackTimeElementRangeFuncPtr _unpackTimeElementRange) :
			id(dvTypeIdentifierToId(_identifier)),
			identifier(_identifier),
			description(_description),
			sizeOfType(_sizeOfType),
			pack(_pack),
			unpack(_unpack),
			construct(_construct),
			destruct(_destruct),
			timeElementExtractor(_timeElementExtractor),
			unpackTimeElementRange(_unpackTimeElementRange) {
			if (identifier == nullptr) {
				throw std::invalid_argument("Type identifier must be defined.");
			}

			if ((identifier[0] == 0) || (identifier[1] == 0) || (identifier[2] == 0) || (identifier[3] == 0)
				|| (identifier[4] != 0)) {
				throw std::invalid_argument("Type identifier must be exactly four characters long.");
			}

			if ((description == nullptr) || (description[0] == 0)) {
				throw std::invalid_argument("Type description must be defined.");
			}
		}

		~dvType() = default;

		// Types should be copyable and movable.
		dvType(const dvType &t)              = default;
		dvType &operator=(const dvType &rhs) = default;
		dvType(dvType &&t)                   = default;
		dvType &operator=(dvType &&rhs)      = default;

		bool operator==(const dvType &rhs) const noexcept {
			return ((id == rhs.id) && (sizeOfType == rhs.sizeOfType) && (pack == rhs.pack) && (unpack == rhs.unpack)
					&& (construct == rhs.construct) && (destruct == rhs.destruct)
					&& (timeElementExtractor == rhs.timeElementExtractor)
					&& (unpackTimeElementRange == rhs.unpackTimeElementRange));
		}

		bool operator!=(const dvType &rhs) const noexcept {
			return (!operator==(rhs));
		}
#endif
	};

	DVSDK_EXPORT struct dvType dvTypeSystemGetInfoByIdentifier(const char *tIdentifier);
	DVSDK_EXPORT struct dvType dvTypeSystemGetInfoByID(uint32_t tId);

	struct dvTypedObject {
		uint32_t typeId;
		size_t objSize;
		void *obj;

#ifdef __cplusplus
		dvTypedObject(const dvType &t) {
			typeId  = t.id;
			objSize = t.sizeOfType;
			obj     = (*t.construct)(objSize);

			if (obj == nullptr) {
				throw std::bad_alloc();
			}
		}

		~dvTypedObject() noexcept {
			if (obj != nullptr) {
				const auto t = dvTypeSystemGetInfoByID(typeId);
				(*t.destruct)(obj);
			}
		}

		// Typed objects should not be copyable, but movable.
		dvTypedObject(const dvTypedObject &t)              = delete;
		dvTypedObject &operator=(const dvTypedObject &rhs) = delete;

		dvTypedObject(dvTypedObject &&t) {
			typeId  = t.typeId;
			objSize = t.objSize;
			obj     = t.obj;

			t.typeId  = dvTypeIdentifierToId("NULL");
			t.objSize = 0;
			t.obj     = nullptr;
		}

		dvTypedObject &operator=(dvTypedObject &&rhs) {
			dv::runtime_assert(this != &rhs, "cannot move-assign into self");

			if (obj != nullptr) {
				const auto t = dvTypeSystemGetInfoByID(typeId);
				(*t.destruct)(obj);
			}

			typeId  = rhs.typeId;
			objSize = rhs.objSize;
			obj     = rhs.obj;

			rhs.typeId  = dvTypeIdentifierToId("NULL");
			rhs.objSize = 0;
			rhs.obj     = nullptr;

			return *this;
		}

		bool operator==(const dvTypedObject &rhs) const noexcept {
			return ((typeId == rhs.typeId) && (objSize == rhs.objSize) && (obj == rhs.obj));
		}

		bool operator!=(const dvTypedObject &rhs) const noexcept {
			return (!operator==(rhs));
		}
#endif
	};

#ifdef __cplusplus
}

static_assert(std::is_standard_layout_v<dvType>, "dvType is not standard layout");
static_assert(std::is_standard_layout_v<dvTypedObject>, "dvTypedObject is not standard layout");

#	include <dv-processing/external/flatbuffers/flatbuffers.h>
#	include <dv-processing/data/cvector.hpp>
#	include <boost/tti/has_member_data.hpp>
#	include <boost/tti/has_member_function.hpp>

namespace dv::Types {

constexpr static const char *nullIdentifier = "NULL";
constexpr static const uint32_t nullId      = dvTypeIdentifierToId(nullIdentifier);

constexpr static const char *anyIdentifier = "ANYT";
constexpr static const uint32_t anyId      = dvTypeIdentifierToId(anyIdentifier);

using Type        = dvType;
using TypedObject = dvTypedObject;

using PackFuncPtr                   = dvTypePackFuncPtr;
using UnpackFuncPtr                 = dvTypeUnpackFuncPtr;
using ConstructPtr                  = dvTypeConstructPtr;
using DestructPtr                   = dvTypeDestructPtr;
using TimeElementExtractorPtr       = dvTypeTimeElementExtractorPtr;
using UnpackTimeElementRangeFuncPtr = dvTypeUnpackTimeElementRangeFuncPtr;

template<typename ObjectAPIType>
constexpr static uint32_t Packer(void *toFlatBufferBuilder, const void *fromObject) {
	using FBType = typename ObjectAPIType::TableType;

	dv::runtime_assert(toFlatBufferBuilder != nullptr, "toFlatBufferBuilder cannot be NULL");
	dv::runtime_assert(fromObject != nullptr, "fromObject cannot be NULL");

	return (FBType::Pack(*(static_cast<flatbuffers::FlatBufferBuilder *>(toFlatBufferBuilder)),
		static_cast<const ObjectAPIType *>(fromObject), nullptr)
				.o);
}

template<typename ObjectAPIType>
constexpr static void Unpacker(void *toObject, const void *fromFlatBuffer) {
	using FBType = typename ObjectAPIType::TableType;

	dv::runtime_assert(toObject != nullptr, "toObject cannot be NULL");
	dv::runtime_assert(fromFlatBuffer != nullptr, "fromFlatBuffer cannot be NULL");

	FBType::UnPackToFrom(static_cast<ObjectAPIType *>(toObject), static_cast<const FBType *>(fromFlatBuffer), nullptr);
}

template<typename ObjectAPIType>
constexpr static void *Constructor(size_t sizeOfObject) {
	auto obj = static_cast<ObjectAPIType *>(malloc(sizeOfObject));
	if (obj == nullptr) {
		throw std::bad_alloc();
	}

	new (obj) ObjectAPIType{};

	return (obj);
}

template<typename ObjectAPIType>
constexpr static void Destructor(void *object) {
	dv::runtime_assert(object != nullptr, "object cannot be NULL");

	auto obj = static_cast<ObjectAPIType *>(object);

	std::destroy_at(obj);

	free(obj);
}

BOOST_TTI_HAS_MEMBER_DATA(elements)
template<typename T, typename U>
inline constexpr bool object_has_elements = has_member_data_elements<T, dv::cvector<U>>::value;

BOOST_TTI_HAS_MEMBER_FUNCTION(elements)
template<typename T, typename U>
inline constexpr bool flatbuffer_has_elements_with_struct
	= has_member_function_elements<const flatbuffers::Vector<const U *> *(T::*) () const>::value;
template<typename T, typename U>
inline constexpr bool flatbuffer_has_elements_with_table
	= has_member_function_elements<const flatbuffers::Vector<flatbuffers::Offset<U>> *(T::*) () const>::value;

// Flatbuffer tables.
BOOST_TTI_HAS_MEMBER_DATA(timestamp)
template<typename T>
inline constexpr bool has_timestamp_as_data = has_member_data_timestamp<T, int64_t>::value;

// Flatbuffer structs.
BOOST_TTI_HAS_MEMBER_FUNCTION(timestamp)
template<typename T>
inline constexpr bool has_timestamp_as_function = has_member_function_timestamp<int64_t (T::*)() const>::value;

template<typename ObjectAPIType, typename SubObjectAPIType>
constexpr static struct dvTypeTimeElementExtractor TimeElementExtractor(const void *object) {
	dv::runtime_assert(object != nullptr, "object cannot be NULL");

	auto obj = static_cast<const ObjectAPIType *>(object);

	// Information extraction mode: get number of contained elements
	// and timestamp related information.
	struct dvTypeTimeElementExtractor result {
		- 1, -1, -1
	};

	if constexpr (object_has_elements<ObjectAPIType, SubObjectAPIType>) {
		result.numElements = obj->elements.size();

		if (result.numElements > 0) {
			if constexpr (has_timestamp_as_data<SubObjectAPIType>) {
				result.startTimestamp = obj->elements.front().timestamp;
				result.endTimestamp   = obj->elements.back().timestamp;
			}
			else if constexpr (has_timestamp_as_function<SubObjectAPIType>) {
				result.startTimestamp = obj->elements.front().timestamp();
				result.endTimestamp   = obj->elements.back().timestamp();
			}
			else {
				result.startTimestamp = result.endTimestamp = -1;
			}
		}
	}
	else {
		result.numElements = 1;

		if constexpr (has_timestamp_as_data<ObjectAPIType>) {
			result.startTimestamp = result.endTimestamp = obj->timestamp;
		}
		else if constexpr (has_timestamp_as_function<ObjectAPIType>) {
			result.startTimestamp = result.endTimestamp = obj->timestamp();
		}
		else {
			result.startTimestamp = result.endTimestamp = -1;
		}
	}

	return (result);
}

template<typename ObjectAPIType, typename SubObjectAPIType>
constexpr static bool UnpackTimeElementRange(
	void *toObject, const void *fromFlatBuffer, struct dvTypeTimeElementExtractor range) {
	using FBType          = typename ObjectAPIType::TableType;
	using SubObjectFBType = typename SubObjectAPIType::TableType;

	dv::runtime_assert(toObject != nullptr, "toObject cannot be NULL");
	dv::runtime_assert(fromFlatBuffer != nullptr, "fromFlatBuffer cannot be NULL");

	auto toObj  = static_cast<ObjectAPIType *>(toObject);
	auto fromFB = static_cast<const FBType *>(fromFlatBuffer);

	if (range.numElements == -1) {
		// Do time-based range extraction, if timestamp exists.
		// Operate directly on Flatbuffers.
		if constexpr (flatbuffer_has_elements_with_table<FBType, SubObjectFBType>
					  || flatbuffer_has_elements_with_struct<FBType, SubObjectFBType>) {
			if constexpr (has_timestamp_as_function<SubObjectFBType>) {
				auto fbElements = fromFB->elements();

				if ((fbElements != nullptr) && (fbElements->size() > 0)) {
					auto lower = std::lower_bound(fbElements->cbegin(), fbElements->cend(), range.startTimestamp,
						[](const auto &elem, const auto &val) {
							return (elem->timestamp() < val);
						});

					if (lower == fbElements->cend()) {
						// No starting point found anywhere, no commit.
						return (false);
					}

					auto upper = std::upper_bound(
						lower, fbElements->cend(), range.endTimestamp, [](const auto &val, const auto &elem) {
							return (elem->timestamp() > val);
						});

					auto objSize = toObj->elements.size();

					toObj->elements.resize(objSize + (std::distance(lower, upper)));

					std::for_each(lower, upper, [toObj, objSize](const auto &elem) mutable {
						if constexpr (flatbuffer_has_elements_with_table<FBType, SubObjectFBType>) {
							elem->UnPackTo(&toObj->elements[objSize++], nullptr);
						}
						else {
							toObj->elements[objSize++] = *elem;
						}
					});
				}
			}
		}
		else {
			if constexpr (has_timestamp_as_function<FBType>) {
				if (fromFB->timestamp() >= range.startTimestamp && fromFB->timestamp() <= range.endTimestamp) {
					Unpacker<ObjectAPIType>(toObj, fromFB);

					// Early commit, we extracted one element already.
					return (true);
				}
			}
		}
	}
	else {
		// Element count-based extraction.
		// TODO: implement later.
	}

	// No commit.
	return (false);
}

template<typename ObjectAPIType, typename SubObjectAPIType>
constexpr static Type makeTypeDefinition(const char *description) {
	static_assert(std::is_standard_layout_v<ObjectAPIType>, "ObjectAPIType is not standard layout");
	static_assert(std::is_standard_layout_v<SubObjectAPIType>, "SubObjectAPIType is not standard layout");

	return (Type{ObjectAPIType::TableType::identifier, description, sizeof(ObjectAPIType), &Packer<ObjectAPIType>,
		&Unpacker<ObjectAPIType>, &Constructor<ObjectAPIType>, &Destructor<ObjectAPIType>,
		&TimeElementExtractor<ObjectAPIType, SubObjectAPIType>,
		&UnpackTimeElementRange<ObjectAPIType, SubObjectAPIType>});
}

} // namespace dv::Types

#endif

#endif // DV_SDK_TYPES_HPP
