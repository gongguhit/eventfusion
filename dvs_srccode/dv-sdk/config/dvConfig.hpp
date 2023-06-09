#ifndef DVCONFIG_HPP_
#define DVCONFIG_HPP_

#include "dvConfig.h"

#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace dv::Config {

enum class AttributeType {
	UNKNOWN = DVCFG_TYPE_UNKNOWN,
	BOOL    = DVCFG_TYPE_BOOL,
	INT     = DVCFG_TYPE_INT,
	LONG    = DVCFG_TYPE_LONG,
	FLOAT   = DVCFG_TYPE_FLOAT,
	DOUBLE  = DVCFG_TYPE_DOUBLE,
	STRING  = DVCFG_TYPE_STRING,
};

template<typename T>
struct AttributeTypeConverter {
	static_assert(true, "Conversion to dv::Config::AttributeType not supported for this type.");
};

template<>
struct AttributeTypeConverter<bool> {
	static constexpr AttributeType type = AttributeType::BOOL;
};

template<>
struct AttributeTypeConverter<int32_t> {
	static constexpr AttributeType type = AttributeType::INT;
};

template<>
struct AttributeTypeConverter<int64_t> {
	static constexpr AttributeType type = AttributeType::LONG;
};

template<>
struct AttributeTypeConverter<float> {
	static constexpr AttributeType type = AttributeType::FLOAT;
};

template<>
struct AttributeTypeConverter<double> {
	static constexpr AttributeType type = AttributeType::DOUBLE;
};

template<>
struct AttributeTypeConverter<std::string> {
	static constexpr AttributeType type = AttributeType::STRING;
};

template<AttributeType T>
struct AttributeTypeGenerator {
	static_assert(true, "Only BOOL, INT, LONG, FLOAT, DOUBLE and STRING are supported.");
};

template<>
struct AttributeTypeGenerator<AttributeType::BOOL> {
	using type                                             = bool;
	static const enum dvConfigAttributeType underlyingType = DVCFG_TYPE_BOOL;
};

template<>
struct AttributeTypeGenerator<AttributeType::INT> {
	using type                                             = int32_t;
	static const enum dvConfigAttributeType underlyingType = DVCFG_TYPE_INT;
};

template<>
struct AttributeTypeGenerator<AttributeType::LONG> {
	using type                                             = int64_t;
	static const enum dvConfigAttributeType underlyingType = DVCFG_TYPE_LONG;
};

template<>
struct AttributeTypeGenerator<AttributeType::FLOAT> {
	using type                                             = float;
	static const enum dvConfigAttributeType underlyingType = DVCFG_TYPE_FLOAT;
};

template<>
struct AttributeTypeGenerator<AttributeType::DOUBLE> {
	using type                                             = double;
	static const enum dvConfigAttributeType underlyingType = DVCFG_TYPE_DOUBLE;
};

template<>
struct AttributeTypeGenerator<AttributeType::STRING> {
	using type                                             = std::string;
	static const enum dvConfigAttributeType underlyingType = DVCFG_TYPE_STRING;
};

template<AttributeType T>
struct AttributeRangeGenerator {
	using rangeType = typename AttributeTypeGenerator<T>::type;
};

template<>
struct AttributeRangeGenerator<AttributeType::BOOL> {
	using rangeType = int32_t;
};

template<>
struct AttributeRangeGenerator<AttributeType::STRING> {
	using rangeType = int32_t;
};

template<AttributeType T>
struct AttributeValue {
public:
	using valueType = typename AttributeTypeGenerator<T>::type;

	valueType value;

	AttributeValue(valueType v) : value(v) {
	}

	AttributeValue(const dvConfigAttributeValue v) {
		switch (T) {
			case AttributeType::BOOL:
				value = v.boolean;
				break;

			case AttributeType::INT:
				value = static_cast<valueType>(v.iint);
				break;

			case AttributeType::LONG:
				value = static_cast<valueType>(v.ilong);
				break;

			case AttributeType::FLOAT:
				value = static_cast<valueType>(v.ffloat);
				break;

			case AttributeType::DOUBLE:
				value = static_cast<valueType>(v.ddouble);
				break;

			case AttributeType::STRING:
				static_assert(true, "AttributeValue<STRING> is specialized.");
				break;

			case AttributeType::UNKNOWN:
			default:
				static_assert(true, "UNKNOWN for internal use only.");
				break;
		}
	}

	dvConfigAttributeValue getCUnion() const {
		dvConfigAttributeValue cUnion;

		switch (T) {
			case AttributeType::BOOL:
				cUnion.boolean = value;
				break;

			case AttributeType::INT:
				cUnion.iint = static_cast<int32_t>(value);
				break;

			case AttributeType::LONG:
				cUnion.ilong = static_cast<int64_t>(value);
				break;

			case AttributeType::FLOAT:
				cUnion.ffloat = static_cast<float>(value);
				break;

			case AttributeType::DOUBLE:
				cUnion.ddouble = static_cast<double>(value);
				break;

			case AttributeType::STRING:
				static_assert(true, "AttributeValue<STRING> is specialized.");
				break;

			case AttributeType::UNKNOWN:
			default:
				static_assert(true, "UNKNOWN for internal use only.");
				break;
		}

		return (cUnion);
	}
};

template<>
struct AttributeValue<AttributeType::STRING> {
public:
	typename AttributeTypeGenerator<AttributeType::STRING>::type value;

	AttributeValue(const std::string_view v) : value(v) {
	}

	AttributeValue(const dvConfigAttributeValue v) : value(v.string) {
	}

	dvConfigAttributeValue getCUnion() const {
		dvConfigAttributeValue cUnion;

		cUnion.string = const_cast<char *>(value.c_str());

		return (cUnion);
	}
};

template<AttributeType T>
struct AttributeRanges {
public:
	using rangeType = typename AttributeRangeGenerator<T>::rangeType;

	rangeType min;
	rangeType max;

	AttributeRanges(rangeType minVal, rangeType maxVal) : min(minVal), max(maxVal) {
	}

	AttributeRanges(const dvConfigAttributeRanges ranges) {
		switch (T) {
			case AttributeType::INT:
				min = static_cast<rangeType>(ranges.min.intRange);
				max = static_cast<rangeType>(ranges.max.intRange);
				break;

			case AttributeType::LONG:
				min = static_cast<rangeType>(ranges.min.longRange);
				max = static_cast<rangeType>(ranges.max.longRange);
				break;

			case AttributeType::FLOAT:
				min = static_cast<rangeType>(ranges.min.floatRange);
				max = static_cast<rangeType>(ranges.max.floatRange);
				break;

			case AttributeType::DOUBLE:
				min = static_cast<rangeType>(ranges.min.doubleRange);
				max = static_cast<rangeType>(ranges.max.doubleRange);
				break;

			case AttributeType::STRING:
				min = static_cast<rangeType>(ranges.min.stringRange);
				max = static_cast<rangeType>(ranges.max.stringRange);
				break;

			case AttributeType::BOOL:
				static_assert(true, "AttributeRanges<BOOL> is specialized.");
				break;

			case AttributeType::UNKNOWN:
			default:
				static_assert(true, "UNKNOWN for internal use only.");
				break;
		}
	}

	dvConfigAttributeRanges getCStruct() const {
		dvConfigAttributeRanges cStruct;

		switch (T) {
			case AttributeType::INT:
				cStruct.min.intRange = static_cast<int32_t>(min);
				cStruct.max.intRange = static_cast<int32_t>(max);
				break;

			case AttributeType::LONG:
				cStruct.min.longRange = static_cast<int64_t>(min);
				cStruct.max.longRange = static_cast<int64_t>(max);
				break;

			case AttributeType::FLOAT:
				cStruct.min.floatRange = static_cast<float>(min);
				cStruct.max.floatRange = static_cast<float>(max);
				break;

			case AttributeType::DOUBLE:
				cStruct.min.doubleRange = static_cast<double>(min);
				cStruct.max.doubleRange = static_cast<double>(max);
				break;

			case AttributeType::STRING:
				cStruct.min.stringRange = static_cast<int32_t>(min);
				cStruct.max.stringRange = static_cast<int32_t>(max);
				break;

			case AttributeType::BOOL:
				static_assert(true, "AttributeRanges<BOOL> is specialized.");
				break;

			case AttributeType::UNKNOWN:
			default:
				static_assert(true, "UNKNOWN for internal use only.");
				break;
		}

		return (cStruct);
	}
};

template<>
struct AttributeRanges<AttributeType::BOOL> {
public:
	using rangeType = typename AttributeRangeGenerator<AttributeType::BOOL>::rangeType;

	rangeType min;
	rangeType max;

	AttributeRanges() : min(0), max(1) {
	}

	AttributeRanges(const dvConfigAttributeRanges ranges) : min(0), max(1) {
		(void) (ranges); // Ignore ranges for bool, always zero.
	}

	dvConfigAttributeRanges getCStruct() const {
		dvConfigAttributeRanges cStruct;

		cStruct.min.intRange = 0;
		cStruct.max.intRange = 1;

		return (cStruct);
	}
};

enum class AttributeFlags : uint32_t {
	NORMAL    = DVCFG_FLAGS_NORMAL,
	READ_ONLY = DVCFG_FLAGS_READ_ONLY,
	NO_EXPORT = DVCFG_FLAGS_NO_EXPORT,
	IMPORTED  = DVCFG_FLAGS_IMPORTED,
};

inline AttributeFlags operator|(AttributeFlags lhs, AttributeFlags rhs) {
	using T = std::underlying_type_t<AttributeFlags>;
	return (static_cast<AttributeFlags>(static_cast<T>(lhs) | static_cast<T>(rhs)));
}

inline AttributeFlags &operator|=(AttributeFlags &lhs, AttributeFlags rhs) {
	lhs = lhs | rhs; // Call | overload above.
	return (lhs);
}

inline int getCFlags(AttributeFlags f) {
	return (static_cast<std::underlying_type_t<AttributeFlags>>(f));
}

class Node {
private:
	dvConfigNode node;

public:
	Node(dvConfigNode n) : node(n) {
	}

	explicit operator dvConfigNode() {
		return (node);
	}

	explicit operator dvConfigNodeConst() const {
		return (node);
	}

	explicit operator bool() const {
		return (node != nullptr);
	}

	std::string getName() const {
		return (dvConfigNodeGetName(node));
	}

	std::string getPath() const {
		return (dvConfigNodeGetPath(node));
	}

	/**
	 * This returns a reference to a node, and as such must be carefully mediated with
	 * any dvConfigNodeRemoveNode() calls.
	 *
	 * @throws std::out_of_range Node is root and has no parent.
	 */
	Node getParent() const {
		dvConfigNode parent = dvConfigNodeGetParent(node);

		if (parent == nullptr) {
			throw std::out_of_range("Node is root and has no parent.");
		}

		return (parent);
	}

	/**
	 * This returns a reference to a node, and as such must be carefully mediated with
	 * any dvConfigNodeRemoveNode() calls.
	 */
	std::vector<Node> getChildren() const {
		std::vector<Node> children;

		size_t numChildren          = 0;
		dvConfigNode *childrenArray = dvConfigNodeGetChildren(node, &numChildren);

		if (numChildren > 0) {
			children.reserve(numChildren);

			for (size_t i = 0; i < numChildren; i++) {
				children.emplace_back(childrenArray[i]);
			}

			free(childrenArray);
		}

		return (children);
	}

	void addNodeListener(void *userData, dvConfigNodeChangeListener node_changed) {
		dvConfigNodeAddNodeListener(node, userData, node_changed);
	}

	void removeNodeListener(void *userData, dvConfigNodeChangeListener node_changed) {
		dvConfigNodeRemoveNodeListener(node, userData, node_changed);
	}

	void removeAllNodeListeners() {
		dvConfigNodeRemoveAllNodeListeners(node);
	}

	void addAttributeListener(void *userData, dvConfigAttributeChangeListener attribute_changed) {
		dvConfigNodeAddAttributeListener(node, userData, attribute_changed);
	}

	void removeAttributeListener(void *userData, dvConfigAttributeChangeListener attribute_changed) {
		dvConfigNodeRemoveAttributeListener(node, userData, attribute_changed);
	}

	void removeAllAttributeListeners() {
		dvConfigNodeRemoveAllAttributeListeners(node);
	}

	/**
	 * Careful, only use if no references exist to this node and all its children.
	 * References are created by dv::Config::Tree::getNode(), dv::Config::Node::getRelativeNode(),
	 * dv::Config::Node::getParent() and dv::Config::Node::getChildren().
	 */
	void removeNode() {
		dvConfigNodeRemoveNode(node);
	}

	/**
	 * Careful, only use if no references exist to this node's children.
	 * References are created by dvConfigTreeGetNode(), dvConfigNodeGetRelativeNode(),
	 * dvConfigNodeGetParent() and dvConfigNodeGetChildren().
	 */
	void removeSubTree() {
		dvConfigNodeRemoveSubTree(node);
	}

	void clearSubTree(bool clearThisNode) {
		dvConfigNodeClearSubTree(node, clearThisNode);
	}

	void copyTo(dv::Config::Node destination) const {
		dvConfigNodeCopy(node, static_cast<dvConfigNode>(destination));
	}

	template<AttributeType T>
	void createAttribute(const std::string &key, const AttributeValue<T> &defaultValue,
		const AttributeRanges<T> &ranges, AttributeFlags flags, const std::string &description) {
		dvConfigNodeCreateAttribute(node, key.c_str(), AttributeTypeGenerator<T>::underlyingType,
			defaultValue.getCUnion(), ranges.getCStruct(), getCFlags(flags), description.c_str());
	}

	// Non-templated version for dynamic runtime types.
	void createAttribute(const std::string &key, AttributeType type, const dvConfigAttributeValue defaultValue,
		const dvConfigAttributeRanges &ranges, AttributeFlags flags, const std::string &description) {
		dvConfigNodeCreateAttribute(node, key.c_str(), static_cast<enum dvConfigAttributeType>(type), defaultValue,
			ranges, getCFlags(flags), description.c_str());
	}

	template<AttributeType T>
	void removeAttribute(const std::string &key) {
		dvConfigNodeRemoveAttribute(node, key.c_str(), AttributeTypeGenerator<T>::underlyingType);
	}

	// Non-templated version for dynamic runtime types.
	void removeAttribute(const std::string &key, AttributeType type) {
		dvConfigNodeRemoveAttribute(node, key.c_str(), static_cast<enum dvConfigAttributeType>(type));
	}

	void removeAllAttributes() {
		dvConfigNodeRemoveAllAttributes(node);
	}

	template<AttributeType T>
	bool existsAttribute(const std::string &key) const {
		return (dvConfigNodeExistsAttribute(node, key.c_str(), AttributeTypeGenerator<T>::underlyingType));
	}

	// Non-templated version for dynamic runtime types.
	bool existsAttribute(const std::string &key, AttributeType type) const {
		return (dvConfigNodeExistsAttribute(node, key.c_str(), static_cast<enum dvConfigAttributeType>(type)));
	}

	template<AttributeType T>
	bool putAttribute(const std::string &key, const AttributeValue<T> &value) {
		return (
			dvConfigNodePutAttribute(node, key.c_str(), AttributeTypeGenerator<T>::underlyingType, value.getCUnion()));
	}

	// Non-templated version for dynamic runtime types.
	bool putAttribute(const std::string &key, AttributeType type, const dvConfigAttributeValue value) {
		return (dvConfigNodePutAttribute(node, key.c_str(), static_cast<enum dvConfigAttributeType>(type), value));
	}

	template<AttributeType T>
	AttributeValue<T> getAttribute(const std::string &key) const {
		dvConfigAttributeValue cVal
			= dvConfigNodeGetAttribute(node, key.c_str(), AttributeTypeGenerator<T>::underlyingType);

		AttributeValue<T> retVal(cVal);

		if (T == AttributeType::STRING) {
			free(cVal.string);
		}

		return (retVal);
	}

	// Non-templated version for dynamic runtime types. Remember to free retVal.string if type == STRING.
	dvConfigAttributeValue getAttribute(const std::string &key, AttributeType type) const {
		return (dvConfigNodeGetAttribute(node, key.c_str(), static_cast<enum dvConfigAttributeType>(type)));
	}

	template<AttributeType T>
	bool updateReadOnlyAttribute(const std::string &key, const AttributeValue<T> &value) {
		return (dvConfigNodeUpdateReadOnlyAttribute(
			node, key.c_str(), AttributeTypeGenerator<T>::underlyingType, value.getCUnion()));
	}

	// Non-templated version for dynamic runtime types.
	bool updateReadOnlyAttribute(const std::string &key, AttributeType type, const dvConfigAttributeValue value) {
		return (dvConfigNodeUpdateReadOnlyAttribute(
			node, key.c_str(), static_cast<enum dvConfigAttributeType>(type), value));
	}

	template<AttributeType T>
	void create(const std::string &key, typename AttributeTypeGenerator<T>::type defaultValue,
		const AttributeRanges<T> &ranges, AttributeFlags flags, const std::string &description) {
		const AttributeValue<T> defVal(defaultValue);
		createAttribute(key, defVal, ranges, flags, description);
	}

	template<AttributeType T>
	void remove(const std::string &key) {
		removeAttribute<T>(key);
	}

	template<AttributeType T>
	bool exists(const std::string &key) const {
		return (existsAttribute<T>(key));
	}

	template<AttributeType T>
	bool put(const std::string &key, typename AttributeTypeGenerator<T>::type value) {
		const AttributeValue<T> val(value);
		return (putAttribute<T>(key, val));
	}

	template<AttributeType T>
	bool updateReadOnly(const std::string &key, typename AttributeTypeGenerator<T>::type value) {
		const AttributeValue<T> val(value);
		return (updateReadOnlyAttribute<T>(key, val));
	}

	template<AttributeType T>
	typename AttributeTypeGenerator<T>::type get(const std::string &key) const {
		return (getAttribute<T>(key).value);
	}

	// Convenience setter methods
	inline bool putInt(const std::string &key, int32_t val) {
		return put<dv::Config::AttributeType::INT>(key, val);
	}

	inline bool putLong(const std::string &key, int64_t val) {
		return put<dv::Config::AttributeType::LONG>(key, val);
	}

	inline bool putFloat(const std::string &key, float val) {
		return put<dv::Config::AttributeType::FLOAT>(key, val);
	}

	inline bool putDouble(const std::string &key, double val) {
		return put<dv::Config::AttributeType::DOUBLE>(key, val);
	}

	inline bool putString(const std::string &key, const std::string &val) {
		return put<dv::Config::AttributeType::STRING>(key, val);
	}

	inline bool putBool(const std::string &key, bool val) {
		return put<dv::Config::AttributeType::BOOL>(key, val);
	}

	// Convenience getter methods
	inline int32_t getInt(const std::string &key) const {
		return get<dv::Config::AttributeType::INT>(key);
	}

	inline int64_t getLong(const std::string &key) const {
		return get<dv::Config::AttributeType::LONG>(key);
	}

	inline float getFloat(const std::string &key) const {
		return get<dv::Config::AttributeType::FLOAT>(key);
	}

	inline double getDouble(const std::string &key) const {
		return get<dv::Config::AttributeType::DOUBLE>(key);
	}

	inline std::string getString(const std::string &key) const {
		return get<dv::Config::AttributeType::STRING>(key);
	}

	inline bool getBool(const std::string &key) const {
		return get<dv::Config::AttributeType::BOOL>(key);
	}

	bool exportNodeToXML(const std::string &filePath, bool exportAll = false) const {
		return (dvConfigNodeExportNodeToXML(node, filePath.c_str(), exportAll));
	}

	bool exportSubTreeToXML(const std::string &filePath, bool exportAll = false) const {
		return (dvConfigNodeExportSubTreeToXML(node, filePath.c_str(), exportAll));
	}

	bool importNodeFromXML(const std::string &filePath, bool strict = true) {
		return (dvConfigNodeImportNodeFromXML(node, filePath.c_str(), strict));
	}

	bool importSubTreeFromXML(const std::string &filePath, bool strict = true) {
		return (dvConfigNodeImportSubTreeFromXML(node, filePath.c_str(), strict));
	}

	std::string exportNodeToXMLString(bool exportAll = false) const {
		auto xmlStr = dvConfigNodeExportNodeToXMLString(node, exportAll);
		auto cppStr = std::string(xmlStr);
		free(xmlStr);
		return (cppStr);
	}

	std::string exportSubTreeToXMLString(bool exportAll = false) const {
		auto xmlStr = dvConfigNodeExportSubTreeToXMLString(node, exportAll);
		auto cppStr = std::string(xmlStr);
		free(xmlStr);
		return (cppStr);
	}

	bool importNodeFromXMLString(const std::string &xmlString, bool strict = true) {
		return (dvConfigNodeImportNodeFromXMLString(node, xmlString.c_str(), strict));
	}

	bool importSubTreeFromXMLString(const std::string &xmlString, bool strict = true) {
		return (dvConfigNodeImportSubTreeFromXMLString(node, xmlString.c_str(), strict));
	}

	bool stringToAttributeConverter(
		const std::string &key, const std::string &type, const std::string &value, bool overrideReadOnly = false) {
		return (
			dvConfigNodeStringToAttributeConverter(node, key.c_str(), type.c_str(), value.c_str(), overrideReadOnly));
	}

	std::vector<std::string> getChildNames() const {
		std::vector<std::string> childNames;

		size_t numChildNames         = 0;
		const char **childNamesArray = dvConfigNodeGetChildNames(node, &numChildNames);

		if (numChildNames > 0) {
			childNames.reserve(numChildNames);

			for (size_t i = 0; i < numChildNames; i++) {
				childNames.emplace_back(childNamesArray[i]);
			}

			free(childNamesArray);
		}

		return (childNames);
	}

	std::vector<std::string> getAttributeKeys() const {
		std::vector<std::string> attributeKeys;

		size_t numAttributeKeys         = 0;
		const char **attributeKeysArray = dvConfigNodeGetAttributeKeys(node, &numAttributeKeys);

		if (numAttributeKeys > 0) {
			attributeKeys.reserve(numAttributeKeys);

			for (size_t i = 0; i < numAttributeKeys; i++) {
				attributeKeys.emplace_back(attributeKeysArray[i]);
			}

			free(attributeKeysArray);
		}

		return (attributeKeys);
	}

	AttributeType getAttributeType(const std::string &key) const {
		return (static_cast<AttributeType>(dvConfigNodeGetAttributeType(node, key.c_str())));
	}

	template<AttributeType T>
	AttributeRanges<T> getAttributeRanges(const std::string &key) const {
		return (dvConfigNodeGetAttributeRanges(node, key.c_str(), AttributeTypeGenerator<T>::underlyingType));
	}

	// Non-templated version for dynamic runtime types.
	struct dvConfigAttributeRanges getAttributeRanges(const std::string &key, AttributeType type) const {
		return (dvConfigNodeGetAttributeRanges(node, key.c_str(), static_cast<enum dvConfigAttributeType>(type)));
	}

	template<AttributeType T>
	int getAttributeFlags(const std::string &key) const {
		return (dvConfigNodeGetAttributeFlags(node, key.c_str(), AttributeTypeGenerator<T>::underlyingType));
	}

	// Non-templated version for dynamic runtime types.
	int getAttributeFlags(const std::string &key, AttributeType type) const {
		return (dvConfigNodeGetAttributeFlags(node, key.c_str(), static_cast<enum dvConfigAttributeType>(type)));
	}

	template<AttributeType T>
	std::string getAttributeDescription(const std::string &key) const {
		char *desc = dvConfigNodeGetAttributeDescription(node, key.c_str(), AttributeTypeGenerator<T>::underlyingType);

		std::string retVal(desc);

		free(desc);

		return (retVal);
	}

	// Non-templated version for dynamic runtime types.
	std::string getAttributeDescription(const std::string &key, AttributeType type) const {
		char *desc
			= dvConfigNodeGetAttributeDescription(node, key.c_str(), static_cast<enum dvConfigAttributeType>(type));

		std::string retVal(desc);

		free(desc);

		return (retVal);
	}

	void attributeModifierButton(const std::string &key, const std::string &buttonLabel) {
		dvConfigNodeAttributeModifierButton(node, key.c_str(), buttonLabel.c_str());
	}

	void attributeModifierListOptions(
		const std::string &key, const std::string &listOptions, bool allowMultipleSelections) {
		dvConfigNodeAttributeModifierListOptions(node, key.c_str(), listOptions.c_str(), allowMultipleSelections);
	}

	void attributeModifierFileChooser(const std::string &key, const std::string &typeAndExtensions) {
		dvConfigNodeAttributeModifierFileChooser(node, key.c_str(), typeAndExtensions.c_str());
	}

	void attributeModifierUnit(const std::string &key, const std::string &unitInformation) {
		dvConfigNodeAttributeModifierUnit(node, key.c_str(), unitInformation.c_str());
	}

	void attributeModifierPriorityAttributes(const std::string &priorityAttributes) {
		dvConfigNodeAttributeModifierPriorityAttributes(node, priorityAttributes.c_str());
	}

	void attributeModifierGUISupport() {
		dvConfigNodeAttributeModifierGUISupport(node);
	}

	void attributeBooleanReset(const std::string &key) {
		dvConfigNodeAttributeBooleanReset(node, key.c_str());
	}

	bool existsRelativeNode(const std::string &nodePath) const {
		return (dvConfigNodeExistsRelativeNode(node, nodePath.c_str()));
	}

	/**
	 * This returns a reference to a node, and as such must be carefully mediated with
	 * any dvConfigNodeRemoveNode() calls.
	 *
	 * @throws std::out_of_range Invalid relative node path.
	 */
	Node getRelativeNode(const std::string &relativeNodePath) const {
		dvConfigNode relativeNode = dvConfigNodeGetRelativeNode(node, relativeNodePath.c_str());

		if (relativeNode == nullptr) {
			throw std::out_of_range("Invalid relative node path.");
		}

		return (relativeNode);
	}

	void attributeUpdaterAdd(const std::string &key, AttributeType type, dvConfigAttributeUpdater updater,
		void *updaterUserData, bool runOnce = false) {
		dvConfigNodeAttributeUpdaterAdd(
			node, key.c_str(), static_cast<enum dvConfigAttributeType>(type), updater, updaterUserData, runOnce);
	}

	void attributeUpdaterRemove(
		const std::string &key, AttributeType type, dvConfigAttributeUpdater updater, void *updaterUserData) {
		dvConfigNodeAttributeUpdaterRemove(
			node, key.c_str(), static_cast<enum dvConfigAttributeType>(type), updater, updaterUserData);
	}

	void attributeUpdaterRemoveAll() {
		dvConfigNodeAttributeUpdaterRemoveAll(node);
	}
};

class Helper {
public:
	static std::string typeToStringConverter(AttributeType type) {
		return (dvConfigHelperTypeToStringConverter(static_cast<enum dvConfigAttributeType>(type)));
	}

	static AttributeType stringToTypeConverter(const std::string &typeString) {
		return (static_cast<AttributeType>(dvConfigHelperStringToTypeConverter(typeString.c_str())));
	}

	static std::string valueToStringConverter(AttributeType type, union dvConfigAttributeValue value) {
		char *val = dvConfigHelperValueToStringConverter(static_cast<enum dvConfigAttributeType>(type), value);

		std::string retVal(val);

		free(val);

		return (retVal);
	}

	static union dvConfigAttributeValue stringToValueConverter(AttributeType type, const std::string &valueString) {
		return (
			dvConfigHelperStringToValueConverter(static_cast<enum dvConfigAttributeType>(type), valueString.c_str()));
	}

	static std::string flagsToStringConverter(int flags) {
		char *val = dvConfigHelperFlagsToStringConverter(flags);

		std::string retVal(val);

		free(val);

		return (retVal);
	}

	static int stringToFlagsConverter(const std::string &flagsString) {
		return (dvConfigHelperStringToFlagsConverter(flagsString.c_str()));
	}

	static std::string rangesToStringConverter(AttributeType type, struct dvConfigAttributeRanges ranges) {
		char *val = dvConfigHelperRangesToStringConverter(static_cast<enum dvConfigAttributeType>(type), ranges);

		std::string retVal(val);

		free(val);

		return (retVal);
	}

	static struct dvConfigAttributeRanges stringToRangesConverter(AttributeType type, const std::string &rangesString) {
		return (
			dvConfigHelperStringToRangesConverter(static_cast<enum dvConfigAttributeType>(type), rangesString.c_str()));
	}
};

class Tree {
private:
	dvConfigTree tree;

public:
	Tree(dvConfigTree t) : tree(t) {
	}

	explicit operator dvConfigTree() const {
		return (tree);
	}

	static Tree globalTree() {
		return (dvConfigTreeGlobal());
	}

	static Tree newTree() {
		return (dvConfigTreeNew());
	}

	void deleteTree() {
		dvConfigTreeDelete(tree);
		tree = nullptr;
	}

	static void errorLogCallbackSet(dvConfigTreeErrorLogCallback error_log_cb) {
		dvConfigTreeErrorLogCallbackSet(error_log_cb);
	}

	static dvConfigTreeErrorLogCallback errorLogCallbackGet() {
		return (dvConfigTreeErrorLogCallbackGet());
	}

	bool existsNode(const std::string &nodePath) const {
		return (dvConfigTreeExistsNode(tree, nodePath.c_str()));
	}

	Node getRootNode() const {
		return (dvConfigTreeGetNode(tree, "/"));
	}

	/**
	 * This returns a reference to a node, and as such must be carefully mediated with
	 * any dvConfigNodeRemoveNode() calls.
	 *
	 * @throws std::out_of_range Invalid absolute node path.
	 */
	Node getNode(const std::string &nodePath) const {
		dvConfigNode node = dvConfigTreeGetNode(tree, nodePath.c_str());

		if (node == nullptr) {
			throw std::out_of_range("Invalid absolute node path.");
		}

		return (node);
	}

	void attributeUpdaterRemoveAll() {
		dvConfigTreeAttributeUpdaterRemoveAll(tree);
	}

	bool attributeUpdaterRun() {
		return (dvConfigTreeAttributeUpdaterRun(tree));
	}

	/**
	 * Listener must be able to deal with userData being NULL at any moment.
	 * This can happen due to concurrent changes from this setter.
	 */
	void globalNodeListenerSet(dvConfigNodeChangeListener node_changed, void *userData) {
		dvConfigTreeGlobalNodeListenerSet(tree, node_changed, userData);
	}

	/**
	 * Listener must be able to deal with userData being NULL at any moment.
	 * This can happen due to concurrent changes from this setter.
	 */
	void globalAttributeListenerSet(dvConfigAttributeChangeListener attribute_changed, void *userData) {
		dvConfigTreeGlobalAttributeListenerSet(tree, attribute_changed, userData);
	}
};

static Tree GLOBAL = Tree::globalTree();

} // namespace dv::Config

#endif /* DVCONFIG_HPP_ */
