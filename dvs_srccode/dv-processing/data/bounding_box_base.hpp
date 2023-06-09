// automatically generated by the FlatBuffers compiler, do not modify

#ifndef FLATBUFFERS_GENERATED_BOUNDINGBOX_DV_H_
#define FLATBUFFERS_GENERATED_BOUNDINGBOX_DV_H_

#pragma GCC system_header
#pragma clang system_header

#include "../external/flatbuffers/flatbuffers.h"

#include "cstring.hpp"
#include "cvector.hpp"

namespace dv {

struct BoundingBoxFlatbuffer;
struct BoundingBox;

struct BoundingBoxPacketFlatbuffer;
struct BoundingBoxPacket;

bool operator==(const BoundingBox &lhs, const BoundingBox &rhs);
bool operator==(const BoundingBoxPacket &lhs, const BoundingBoxPacket &rhs);

inline const flatbuffers::TypeTable *BoundingBoxTypeTable();

inline const flatbuffers::TypeTable *BoundingBoxPacketTypeTable();

struct BoundingBox : public flatbuffers::NativeTable {
	typedef BoundingBoxFlatbuffer TableType;

	static FLATBUFFERS_CONSTEXPR const char *GetFullyQualifiedName() {
		return "dv.BoundingBox";
	}

	int64_t timestamp;
	float topLeftX;
	float topLeftY;
	float bottomRightX;
	float bottomRightY;
	float confidence;
	dv::cstring label;

	BoundingBox() :
		timestamp(0),
		topLeftX(0.0f),
		topLeftY(0.0f),
		bottomRightX(0.0f),
		bottomRightY(0.0f),
		confidence(0.0f) {
	}

	// Generated Constructor
	BoundingBox(int64_t _timestamp, float _topLeftX, float _topLeftY, float _bottomRightX, float _bottomRightY,
		float _confidence, const dv::cstring &_label) :
		timestamp{_timestamp},
		topLeftX{_topLeftX},
		topLeftY{_topLeftY},
		bottomRightX{_bottomRightX},
		bottomRightY{_bottomRightY},
		confidence{_confidence},
		label{_label} {
	}
};

inline bool operator==(const BoundingBox &lhs, const BoundingBox &rhs) {
	return (lhs.timestamp == rhs.timestamp) && (lhs.topLeftX == rhs.topLeftX) && (lhs.topLeftY == rhs.topLeftY)
		&& (lhs.bottomRightX == rhs.bottomRightX) && (lhs.bottomRightY == rhs.bottomRightY)
		&& (lhs.confidence == rhs.confidence) && (lhs.label == rhs.label);
}

struct BoundingBoxFlatbuffer FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
	typedef BoundingBox NativeTableType;

	static const flatbuffers::TypeTable *MiniReflectTypeTable() {
		return BoundingBoxTypeTable();
	}

	static FLATBUFFERS_CONSTEXPR const char *GetFullyQualifiedName() {
		return "dv.BoundingBox";
	}

	enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
		VT_TIMESTAMP    = 4,
		VT_TOPLEFTX     = 6,
		VT_TOPLEFTY     = 8,
		VT_BOTTOMRIGHTX = 10,
		VT_BOTTOMRIGHTY = 12,
		VT_CONFIDENCE   = 14,
		VT_LABEL        = 16
	};

	/// Timestamp (µs).
	int64_t timestamp() const {
		return GetField<int64_t>(VT_TIMESTAMP, 0);
	}

	/// top left corner of bounding box x-coordinate.
	float topLeftX() const {
		return GetField<float>(VT_TOPLEFTX, 0.0f);
	}

	/// top left corner of bounding box y-coordinate.
	float topLeftY() const {
		return GetField<float>(VT_TOPLEFTY, 0.0f);
	}

	/// bottom right corner of bounding box x-coordinate.
	float bottomRightX() const {
		return GetField<float>(VT_BOTTOMRIGHTX, 0.0f);
	}

	/// bottom right corner of bounding box y-coordinate.
	float bottomRightY() const {
		return GetField<float>(VT_BOTTOMRIGHTY, 0.0f);
	}

	/// confidence of the given bounding box.
	float confidence() const {
		return GetField<float>(VT_CONFIDENCE, 0.0f);
	}

	/// Label for the given bounding box
	const flatbuffers::String *label() const {
		return GetPointer<const flatbuffers::String *>(VT_LABEL);
	}

	bool Verify(flatbuffers::Verifier &verifier) const {
		return VerifyTableStart(verifier) && VerifyField<int64_t>(verifier, VT_TIMESTAMP)
			&& VerifyField<float>(verifier, VT_TOPLEFTX) && VerifyField<float>(verifier, VT_TOPLEFTY)
			&& VerifyField<float>(verifier, VT_BOTTOMRIGHTX) && VerifyField<float>(verifier, VT_BOTTOMRIGHTY)
			&& VerifyField<float>(verifier, VT_CONFIDENCE) && VerifyOffset(verifier, VT_LABEL)
			&& verifier.VerifyString(label()) && verifier.EndTable();
	}

	BoundingBox *UnPack(const flatbuffers::resolver_function_t *_resolver = nullptr) const;
	void UnPackTo(BoundingBox *_o, const flatbuffers::resolver_function_t *_resolver = nullptr) const;
	static void UnPackToFrom(
		BoundingBox *_o, const BoundingBoxFlatbuffer *_fb, const flatbuffers::resolver_function_t *_resolver = nullptr);
	static flatbuffers::Offset<BoundingBoxFlatbuffer> Pack(flatbuffers::FlatBufferBuilder &_fbb, const BoundingBox *_o,
		const flatbuffers::rehasher_function_t *_rehasher = nullptr);
};

struct BoundingBoxBuilder {
	flatbuffers::FlatBufferBuilder &fbb_;
	flatbuffers::uoffset_t start_;

	void add_timestamp(int64_t timestamp) {
		fbb_.AddElement<int64_t>(BoundingBoxFlatbuffer::VT_TIMESTAMP, timestamp, 0);
	}

	void add_topLeftX(float topLeftX) {
		fbb_.AddElement<float>(BoundingBoxFlatbuffer::VT_TOPLEFTX, topLeftX, 0.0f);
	}

	void add_topLeftY(float topLeftY) {
		fbb_.AddElement<float>(BoundingBoxFlatbuffer::VT_TOPLEFTY, topLeftY, 0.0f);
	}

	void add_bottomRightX(float bottomRightX) {
		fbb_.AddElement<float>(BoundingBoxFlatbuffer::VT_BOTTOMRIGHTX, bottomRightX, 0.0f);
	}

	void add_bottomRightY(float bottomRightY) {
		fbb_.AddElement<float>(BoundingBoxFlatbuffer::VT_BOTTOMRIGHTY, bottomRightY, 0.0f);
	}

	void add_confidence(float confidence) {
		fbb_.AddElement<float>(BoundingBoxFlatbuffer::VT_CONFIDENCE, confidence, 0.0f);
	}

	void add_label(flatbuffers::Offset<flatbuffers::String> label) {
		fbb_.AddOffset(BoundingBoxFlatbuffer::VT_LABEL, label);
	}

	explicit BoundingBoxBuilder(flatbuffers::FlatBufferBuilder &_fbb) : fbb_(_fbb) {
		start_ = fbb_.StartTable();
	}

	BoundingBoxBuilder &operator=(const BoundingBoxBuilder &);

	flatbuffers::Offset<BoundingBoxFlatbuffer> Finish() {
		const auto end = fbb_.EndTable(start_);
		auto o         = flatbuffers::Offset<BoundingBoxFlatbuffer>(end);
		return o;
	}
};

inline flatbuffers::Offset<BoundingBoxFlatbuffer> CreateBoundingBox(flatbuffers::FlatBufferBuilder &_fbb,
	int64_t timestamp = 0, float topLeftX = 0.0f, float topLeftY = 0.0f, float bottomRightX = 0.0f,
	float bottomRightY = 0.0f, float confidence = 0.0f, flatbuffers::Offset<flatbuffers::String> label = 0) {
	BoundingBoxBuilder builder_(_fbb);
	builder_.add_timestamp(timestamp);
	builder_.add_label(label);
	builder_.add_confidence(confidence);
	builder_.add_bottomRightY(bottomRightY);
	builder_.add_bottomRightX(bottomRightX);
	builder_.add_topLeftY(topLeftY);
	builder_.add_topLeftX(topLeftX);
	return builder_.Finish();
}

inline flatbuffers::Offset<BoundingBoxFlatbuffer> CreateBoundingBoxDirect(flatbuffers::FlatBufferBuilder &_fbb,
	int64_t timestamp = 0, float topLeftX = 0.0f, float topLeftY = 0.0f, float bottomRightX = 0.0f,
	float bottomRightY = 0.0f, float confidence = 0.0f, const char *label = nullptr) {
	auto label__ = label ? _fbb.CreateString(label) : 0;
	return dv::CreateBoundingBox(_fbb, timestamp, topLeftX, topLeftY, bottomRightX, bottomRightY, confidence, label__);
}

flatbuffers::Offset<BoundingBoxFlatbuffer> CreateBoundingBox(flatbuffers::FlatBufferBuilder &_fbb,
	const BoundingBox *_o, const flatbuffers::rehasher_function_t *_rehasher = nullptr);

struct BoundingBoxPacket : public flatbuffers::NativeTable {
	typedef BoundingBoxPacketFlatbuffer TableType;

	static FLATBUFFERS_CONSTEXPR const char *GetFullyQualifiedName() {
		return "dv.BoundingBoxPacket";
	}

	dv::cvector<BoundingBox> elements;

	BoundingBoxPacket() {
	}

	// Generated Constructor
	BoundingBoxPacket(const dv::cvector<BoundingBox> &_elements) : elements{_elements} {
	}

	friend std::ostream &operator<<(std::ostream &os, const BoundingBoxPacket &packet) {
		if (packet.elements.empty()) {
			os << fmt::format("BoundingBoxPacket containing 0 elements");
			return os;
		}

		const int64_t lowestTime  = packet.elements.front().timestamp;
		const int64_t highestTime = packet.elements.back().timestamp;

		os << fmt::format("BoundingBoxPacket containing {} elements within {}μs duration; time range within [{}; {}]",
			packet.elements.size(), highestTime - lowestTime, lowestTime, highestTime);
		return os;
	}
};

inline bool operator==(const BoundingBoxPacket &lhs, const BoundingBoxPacket &rhs) {
	return (lhs.elements == rhs.elements);
}

struct BoundingBoxPacketFlatbuffer FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
	typedef BoundingBoxPacket NativeTableType;
	static FLATBUFFERS_CONSTEXPR const char *identifier = "BBOX";

	static const flatbuffers::TypeTable *MiniReflectTypeTable() {
		return BoundingBoxPacketTypeTable();
	}

	static FLATBUFFERS_CONSTEXPR const char *GetFullyQualifiedName() {
		return "dv.BoundingBoxPacket";
	}

	enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
		VT_ELEMENTS = 4
	};

	const flatbuffers::Vector<flatbuffers::Offset<BoundingBoxFlatbuffer>> *elements() const {
		return GetPointer<const flatbuffers::Vector<flatbuffers::Offset<BoundingBoxFlatbuffer>> *>(VT_ELEMENTS);
	}

	bool Verify(flatbuffers::Verifier &verifier) const {
		return VerifyTableStart(verifier) && VerifyOffset(verifier, VT_ELEMENTS) && verifier.VerifyVector(elements())
			&& verifier.VerifyVectorOfTables(elements()) && verifier.EndTable();
	}

	BoundingBoxPacket *UnPack(const flatbuffers::resolver_function_t *_resolver = nullptr) const;
	void UnPackTo(BoundingBoxPacket *_o, const flatbuffers::resolver_function_t *_resolver = nullptr) const;
	static void UnPackToFrom(BoundingBoxPacket *_o, const BoundingBoxPacketFlatbuffer *_fb,
		const flatbuffers::resolver_function_t *_resolver = nullptr);
	static flatbuffers::Offset<BoundingBoxPacketFlatbuffer> Pack(flatbuffers::FlatBufferBuilder &_fbb,
		const BoundingBoxPacket *_o, const flatbuffers::rehasher_function_t *_rehasher = nullptr);
};

struct BoundingBoxPacketBuilder {
	flatbuffers::FlatBufferBuilder &fbb_;
	flatbuffers::uoffset_t start_;

	void add_elements(flatbuffers::Offset<flatbuffers::Vector<flatbuffers::Offset<BoundingBoxFlatbuffer>>> elements) {
		fbb_.AddOffset(BoundingBoxPacketFlatbuffer::VT_ELEMENTS, elements);
	}

	explicit BoundingBoxPacketBuilder(flatbuffers::FlatBufferBuilder &_fbb) : fbb_(_fbb) {
		start_ = fbb_.StartTable();
	}

	BoundingBoxPacketBuilder &operator=(const BoundingBoxPacketBuilder &);

	flatbuffers::Offset<BoundingBoxPacketFlatbuffer> Finish() {
		const auto end = fbb_.EndTable(start_);
		auto o         = flatbuffers::Offset<BoundingBoxPacketFlatbuffer>(end);
		return o;
	}
};

inline flatbuffers::Offset<BoundingBoxPacketFlatbuffer> CreateBoundingBoxPacket(flatbuffers::FlatBufferBuilder &_fbb,
	flatbuffers::Offset<flatbuffers::Vector<flatbuffers::Offset<BoundingBoxFlatbuffer>>> elements = 0) {
	BoundingBoxPacketBuilder builder_(_fbb);
	builder_.add_elements(elements);
	return builder_.Finish();
}

inline flatbuffers::Offset<BoundingBoxPacketFlatbuffer> CreateBoundingBoxPacketDirect(
	flatbuffers::FlatBufferBuilder &_fbb,
	const std::vector<flatbuffers::Offset<BoundingBoxFlatbuffer>> *elements = nullptr) {
	auto elements__ = elements ? _fbb.CreateVector<flatbuffers::Offset<BoundingBoxFlatbuffer>>(*elements) : 0;
	return dv::CreateBoundingBoxPacket(_fbb, elements__);
}

flatbuffers::Offset<BoundingBoxPacketFlatbuffer> CreateBoundingBoxPacket(flatbuffers::FlatBufferBuilder &_fbb,
	const BoundingBoxPacket *_o, const flatbuffers::rehasher_function_t *_rehasher = nullptr);

inline BoundingBox *BoundingBoxFlatbuffer::UnPack(const flatbuffers::resolver_function_t *_resolver) const {
	auto _o = new BoundingBox();
	UnPackTo(_o, _resolver);
	return _o;
}

inline void BoundingBoxFlatbuffer::UnPackTo(BoundingBox *_o, const flatbuffers::resolver_function_t *_resolver) const {
	(void) _o;
	(void) _resolver;
	UnPackToFrom(_o, this, _resolver);
}

inline void BoundingBoxFlatbuffer::UnPackToFrom(
	BoundingBox *_o, const BoundingBoxFlatbuffer *_fb, const flatbuffers::resolver_function_t *_resolver) {
	(void) _o;
	(void) _fb;
	(void) _resolver;
	{
		auto _e       = _fb->timestamp();
		_o->timestamp = _e;
	};
	{
		auto _e      = _fb->topLeftX();
		_o->topLeftX = _e;
	};
	{
		auto _e      = _fb->topLeftY();
		_o->topLeftY = _e;
	};
	{
		auto _e          = _fb->bottomRightX();
		_o->bottomRightX = _e;
	};
	{
		auto _e          = _fb->bottomRightY();
		_o->bottomRightY = _e;
	};
	{
		auto _e        = _fb->confidence();
		_o->confidence = _e;
	};
	{
		auto _e = _fb->label();
		if (_e)
			_o->label = dv::cstring(_e->c_str(), _e->size());
	};
}

inline flatbuffers::Offset<BoundingBoxFlatbuffer> BoundingBoxFlatbuffer::Pack(
	flatbuffers::FlatBufferBuilder &_fbb, const BoundingBox *_o, const flatbuffers::rehasher_function_t *_rehasher) {
	return CreateBoundingBox(_fbb, _o, _rehasher);
}

inline flatbuffers::Offset<BoundingBoxFlatbuffer> CreateBoundingBox(
	flatbuffers::FlatBufferBuilder &_fbb, const BoundingBox *_o, const flatbuffers::rehasher_function_t *_rehasher) {
	(void) _rehasher;
	(void) _o;

	struct _VectorArgs {
		flatbuffers::FlatBufferBuilder *__fbb;
		const BoundingBox *__o;
		const flatbuffers::rehasher_function_t *__rehasher;
	} _va = {&_fbb, _o, _rehasher};

	(void) _va;
	auto _timestamp    = _o->timestamp;
	auto _topLeftX     = _o->topLeftX;
	auto _topLeftY     = _o->topLeftY;
	auto _bottomRightX = _o->bottomRightX;
	auto _bottomRightY = _o->bottomRightY;
	auto _confidence   = _o->confidence;
	auto _label        = _o->label.empty() ? 0 : _fbb.CreateString(_o->label);
	return dv::CreateBoundingBox(
		_fbb, _timestamp, _topLeftX, _topLeftY, _bottomRightX, _bottomRightY, _confidence, _label);
}

inline BoundingBoxPacket *BoundingBoxPacketFlatbuffer::UnPack(const flatbuffers::resolver_function_t *_resolver) const {
	auto _o = new BoundingBoxPacket();
	UnPackTo(_o, _resolver);
	return _o;
}

inline void BoundingBoxPacketFlatbuffer::UnPackTo(
	BoundingBoxPacket *_o, const flatbuffers::resolver_function_t *_resolver) const {
	(void) _o;
	(void) _resolver;
	UnPackToFrom(_o, this, _resolver);
}

inline void BoundingBoxPacketFlatbuffer::UnPackToFrom(
	BoundingBoxPacket *_o, const BoundingBoxPacketFlatbuffer *_fb, const flatbuffers::resolver_function_t *_resolver) {
	(void) _o;
	(void) _fb;
	(void) _resolver;
	{
		auto _e = _fb->elements();
		if (_e) {
			_o->elements.resize(_e->size());
			for (flatbuffers::uoffset_t _i = 0; _i < _e->size(); _i++) {
				_e->Get(_i)->UnPackTo(&_o->elements[_i], _resolver);
			}
		}
	};
}

inline flatbuffers::Offset<BoundingBoxPacketFlatbuffer> BoundingBoxPacketFlatbuffer::Pack(
	flatbuffers::FlatBufferBuilder &_fbb, const BoundingBoxPacket *_o,
	const flatbuffers::rehasher_function_t *_rehasher) {
	return CreateBoundingBoxPacket(_fbb, _o, _rehasher);
}

inline flatbuffers::Offset<BoundingBoxPacketFlatbuffer> CreateBoundingBoxPacket(flatbuffers::FlatBufferBuilder &_fbb,
	const BoundingBoxPacket *_o, const flatbuffers::rehasher_function_t *_rehasher) {
	(void) _rehasher;
	(void) _o;

	struct _VectorArgs {
		flatbuffers::FlatBufferBuilder *__fbb;
		const BoundingBoxPacket *__o;
		const flatbuffers::rehasher_function_t *__rehasher;
	} _va = {&_fbb, _o, _rehasher};

	(void) _va;
	auto _elements = _o->elements.size() ? _fbb.CreateVector<flatbuffers::Offset<BoundingBoxFlatbuffer>>(
						 _o->elements.size(),
						 [](size_t i, _VectorArgs *__va) {
							 return CreateBoundingBox(*__va->__fbb, &__va->__o->elements[i], __va->__rehasher);
						 },
						 &_va)
										 : 0;
	return dv::CreateBoundingBoxPacket(_fbb, _elements);
}

inline const flatbuffers::TypeTable *BoundingBoxTypeTable() {
	static const flatbuffers::TypeCode type_codes[] = {
		{flatbuffers::ET_LONG,   0, -1},
        {flatbuffers::ET_FLOAT,  0, -1},
        {flatbuffers::ET_FLOAT,  0, -1},
		{flatbuffers::ET_FLOAT,  0, -1},
        {flatbuffers::ET_FLOAT,  0, -1},
        {flatbuffers::ET_FLOAT,  0, -1},
		{flatbuffers::ET_STRING, 0, -1}
    };
	static const char *const names[]
		= {"timestamp", "topLeftX", "topLeftY", "bottomRightX", "bottomRightY", "confidence", "label"};
	static const flatbuffers::TypeTable tt = {flatbuffers::ST_TABLE, 7, type_codes, nullptr, nullptr, names};
	return &tt;
}

inline const flatbuffers::TypeTable *BoundingBoxPacketTypeTable() {
	static const flatbuffers::TypeCode type_codes[] = {
		{flatbuffers::ET_SEQUENCE, 1, 0}
    };
	static const flatbuffers::TypeFunction type_refs[] = {BoundingBoxTypeTable};
	static const char *const names[]                   = {"elements"};
	static const flatbuffers::TypeTable tt = {flatbuffers::ST_TABLE, 1, type_codes, type_refs, nullptr, names};
	return &tt;
}

inline const dv::BoundingBoxPacketFlatbuffer *GetBoundingBoxPacket(const void *buf) {
	return flatbuffers::GetRoot<dv::BoundingBoxPacketFlatbuffer>(buf);
}

inline const dv::BoundingBoxPacketFlatbuffer *GetSizePrefixedBoundingBoxPacket(const void *buf) {
	return flatbuffers::GetSizePrefixedRoot<dv::BoundingBoxPacketFlatbuffer>(buf);
}

inline const char *BoundingBoxPacketIdentifier() {
	return "BBOX";
}

inline bool BoundingBoxPacketBufferHasIdentifier(const void *buf) {
	return flatbuffers::BufferHasIdentifier(buf, BoundingBoxPacketIdentifier());
}

inline bool VerifyBoundingBoxPacketBuffer(flatbuffers::Verifier &verifier) {
	return verifier.VerifyBuffer<dv::BoundingBoxPacketFlatbuffer>(BoundingBoxPacketIdentifier());
}

inline bool VerifySizePrefixedBoundingBoxPacketBuffer(flatbuffers::Verifier &verifier) {
	return verifier.VerifySizePrefixedBuffer<dv::BoundingBoxPacketFlatbuffer>(BoundingBoxPacketIdentifier());
}

inline void FinishBoundingBoxPacketBuffer(
	flatbuffers::FlatBufferBuilder &fbb, flatbuffers::Offset<dv::BoundingBoxPacketFlatbuffer> root) {
	fbb.Finish(root, BoundingBoxPacketIdentifier());
}

inline void FinishSizePrefixedBoundingBoxPacketBuffer(
	flatbuffers::FlatBufferBuilder &fbb, flatbuffers::Offset<dv::BoundingBoxPacketFlatbuffer> root) {
	fbb.FinishSizePrefixed(root, BoundingBoxPacketIdentifier());
}

inline std::unique_ptr<BoundingBoxPacket> UnPackBoundingBoxPacket(
	const void *buf, const flatbuffers::resolver_function_t *res = nullptr) {
	return std::unique_ptr<BoundingBoxPacket>(GetBoundingBoxPacket(buf)->UnPack(res));
}

} // namespace dv

// fmt compatibility for ostream class printing.
template<>
struct fmt::formatter<dv::BoundingBoxPacket> : fmt::ostream_formatter {};

#endif // FLATBUFFERS_GENERATED_BOUNDINGBOX_DV_H_
