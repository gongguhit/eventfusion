#ifndef DV_SDK_CONFIG_HPP
#define DV_SDK_CONFIG_HPP

#include "cross/portable_io.h"
#include "utils.h"

#include <boost/algorithm/string/join.hpp>

#include <chrono>
#include <cmath>
#include <string>
#include <unordered_map>
#include <vector>

// Allow modules to have multiple threads by making a module's global objects
// (log, inputs, ...) safe to be used concurrently by multiple threads.
// This has a performance impact and is disabled by default.
#ifndef DV_MODULE_MULTI_THREAD_SAFE
#	define DV_MODULE_MULTI_THREAD_SAFE 0
#endif

#if defined(DV_MODULE_MULTI_THREAD_SAFE) && DV_MODULE_MULTI_THREAD_SAFE == 1
#	include <shared_mutex>
#endif

namespace dv {

/**
 * Returns the sign of the given number as -1 or 1. Returns 1 for 0.
 * @tparam T The data type of the number and return value
 * @param x the data to be checked
 * @return -1 iff x < 0, 1 otherwise
 */
template<class T>
inline T sgn(T x) {
	return (x < static_cast<T>(0)) ? static_cast<T>(-1) : static_cast<T>(1);
}

/**
 * Different opening modes for a File Dialog config option.
 */
enum class FileDialogMode {
	OPEN,
	SAVE,
	DIRECTORY
};

/**
 * INTERNAL: select between different string types.
 */
enum class _StringAttributeType {
	NORMAL,
	LIST,
	FILE
};

/**
 * INTERNAL: select between different boolean types
 */
enum class _BooleanAttributeType {
	CHECKBOX,
	BUTTON
};

/**
 * Maps the selected config type to a struct with additional config params
 * that have to be set for the selected config type. Defaults to an empty struct.
 */
template<dv::Config::AttributeType>
struct _ConfigAttributes {};

template<>
struct _ConfigAttributes<dv::Config::AttributeType::BOOL> {
	dv::_BooleanAttributeType attributeType;
	const std::string buttonLabel;

	_ConfigAttributes(dv::_BooleanAttributeType attributeType_, const std::string &buttonLabel_ = std::string()) :
		attributeType(attributeType_),
		buttonLabel(buttonLabel_) {
	}
};

template<>
struct _ConfigAttributes<dv::Config::AttributeType::INT> {
	dv::Config::AttributeRanges<dv::Config::AttributeType::INT> range;
	std::string unit;

	_ConfigAttributes(int32_t minValue, int32_t maxValue) : range(minValue, maxValue) {
	}
};

template<>
struct _ConfigAttributes<dv::Config::AttributeType::LONG> {
	dv::Config::AttributeRanges<dv::Config::AttributeType::LONG> range;
	std::string unit;

	_ConfigAttributes(int64_t minValue, int64_t maxValue) : range(minValue, maxValue) {
	}
};

template<>
struct _ConfigAttributes<dv::Config::AttributeType::FLOAT> {
	dv::Config::AttributeRanges<dv::Config::AttributeType::FLOAT> range;
	std::string unit;

	_ConfigAttributes(float minValue, float maxValue) : range(minValue, maxValue) {
	}
};

template<>
struct _ConfigAttributes<dv::Config::AttributeType::DOUBLE> {
	dv::Config::AttributeRanges<dv::Config::AttributeType::DOUBLE> range;
	std::string unit;

	_ConfigAttributes(double minValue, double maxValue) : range(minValue, maxValue) {
	}
};

template<>
struct _ConfigAttributes<dv::Config::AttributeType::STRING> {
	dv::Config::AttributeRanges<dv::Config::AttributeType::STRING> length;
	_StringAttributeType type;
	// List of strings related options.
	std::vector<std::string> listOptions;
	bool listAllowMultipleSelections;
	// File chooser (path string) related options.
	FileDialogMode fileMode;
	std::string fileAllowedExtensions;

	_ConfigAttributes(int32_t minLength, int32_t maxLength, _StringAttributeType t) :
		length(minLength, maxLength),
		type(t) {
	}
};

class _RateLimiter {
private:
	float rate;                                                    // unit: messages / milliseconds
	float allowanceLimit;                                          // unit: messages
	float allowance;                                               // unit: messages
	std::chrono::time_point<std::chrono::steady_clock> last_check; // unit: milliseconds (cast)

public:
	_RateLimiter(int32_t messageRate, int32_t perMilliseconds) :
		rate(static_cast<float>(messageRate) / static_cast<float>(perMilliseconds)),
		allowanceLimit(static_cast<float>(messageRate)),
		allowance(1.0f), // Always allow first message through.
		last_check(std::chrono::steady_clock::now()) {
	}

	bool pass() {
		std::chrono::time_point<std::chrono::steady_clock> current = std::chrono::steady_clock::now();
		const auto time_passed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(current - last_check);
		last_check                = current;

		allowance += static_cast<float>(time_passed_ms.count()) * rate;

		if (allowance > allowanceLimit) {
			allowance = allowanceLimit; // throttle
		}

		if (allowance < 1.0f) {
			return (false);
		}
		else {
			allowance -= 1.0f;
			return (true);
		}
	}
};

/**
 * Templated implementation class of a ConfigOption. Stores extra attributes according to the selected config type.
 */
template<dv::Config::AttributeType T>
class _ConfigOption {
	using _AttrType = typename dv::Config::AttributeTypeGenerator<T>::type;

public:
	const std::string description;
	const _AttrType initValue;
	const _ConfigAttributes<T> attributes;
	const dv::Config::AttributeFlags flags;
	const bool updateReadOnly;
	_AttrType currentValue;

	_ConfigOption(const std::string &description_, _AttrType initValue_, const _ConfigAttributes<T> &attributes_,
		dv::Config::AttributeFlags flags_, bool updateReadOnly_ = false) :
		description(description_),
		initValue(initValue_),
		attributes(attributes_),
		flags(flags_),
		updateReadOnly(updateReadOnly_),
		currentValue(initValue_) {
	}
};

/**
 * Non template class for a config option. Has a type independent unique pointer
 * that points to the actual (templated) config object.
 * Private constructor. Should only be used with static factory function.
 */
class ConfigOption {
private:
	dv::unique_ptr_void configOption;
	dv::Config::AttributeType type;
	dv::Config::Node node;
	std::string key;
	std::unique_ptr<_RateLimiter> rateLimit;

	/**
	 * __Private constructor__
	 * Takes shared_ptr to templated `_ConfigOption` as well as the variant.
	 * @param configOption_ A unique_ptr to an instantiated `_ConfigOption`
	 * @param type_ The config variant of the passed option
	 */
	ConfigOption(dv::unique_ptr_void configOption_, dv::Config::AttributeType type_) :
		configOption(std::move(configOption_)),
		type(type_),
		node(nullptr) {
	}

	/**
	 * Set link to actual node and attribute for configuration tree operations.
	 * Must be set for tree operations (create, update etc.) to work.
	 * @param moduleNode the module's configuration node.
	 * @param fullKey the key under which the attribute is to be stored.
	 */
	void setNodeAttrLink(dv::Config::Node moduleNode, const std::string &fullKey) {
		size_t pos = fullKey.find_last_of("/");

		if (pos != std::string::npos) {
			node = moduleNode.getRelativeNode(fullKey.substr(0, pos + 1));
			key  = fullKey.substr(pos + 1);
		}
		else {
			node = moduleNode;
			key  = fullKey;
		}
	}

	/**
	 * Updates the current value of the ConfigOption based on the value
	 * that is present in the dv config tree.
	 */
	template<dv::Config::AttributeType T>
	void _updateValue() {
		auto &config = getConfigObject<T>();

		auto newValue = node.get<T>(key);

		if (newValue != config.currentValue) {
			config.currentValue = newValue;
		}
	}

	/**
	 * Private base factory method.
	 * Used as a base for all the config factories defined
	 * Creates a new ConfigOption of the requested type. Works by first instantiating
	 * a `_ConfigOption` with the right templated variant, then creating a `unique_ptr`
	 * and creating a `ConfigOption` to return.
	 * @return
	 */
	template<dv::Config::AttributeType T>
	static ConfigOption getOption(const std::string &description,
		typename dv::Config::AttributeTypeGenerator<T>::type defaultValue, const _ConfigAttributes<T> &attributes,
		dv::Config::AttributeFlags flags, bool updateReadOnly = false) {
		return (ConfigOption(
			dv::make_unique_void(new _ConfigOption<T>(description, defaultValue, attributes, flags, updateReadOnly)),
			T));
	}

public:
	void setRateLimit(int32_t messageRate, int32_t perMilliseconds) {
		if (messageRate <= 0 || perMilliseconds <= 0) {
			// Disable rate limiting.
			rateLimit = nullptr;
		}
		else {
			rateLimit = std::make_unique<_RateLimiter>(messageRate, perMilliseconds);
		}
	}

	/**
	 * Returns the type of this `ConfigOption`.
	 * @return the configuration's type.
	 */
	dv::Config::AttributeType getType() const {
		return (type);
	}

	/**
	 * Returns the underlying config object, casted to the specified configVariant.
	 * The config variant can be read out with the `getVariant` method.
	 * @tparam T The config variant to be casted to.
	 * @return The underlying _ConfigObject with the configuration data
	 */
	template<dv::Config::AttributeType T>
	_ConfigOption<T> &getConfigObject() {
		return (*(static_cast<_ConfigOption<T> *>(configOption.get())));
	}

	/**
	 * Returns the underlying config object, casted to the specified configVariant.
	 * The config variant can be read out with the `getVariant` method.
	 * @tparam T The config variant to be casted to.
	 * @return The underlying _ConfigObject with the configuration data
	 */
	template<dv::Config::AttributeType T>
	const _ConfigOption<T> &getConfigObject() const {
		return (*(static_cast<const _ConfigOption<T> *>(configOption.get())));
	}

	/**
	 * Returns the current value of this config option. Needs a template paramenter
	 * of the type `dv::ConfigVariant::*` to determine what type of config parameter
	 * to return.
	 * @tparam T The config variant type
	 * @return A simple value (long, string etc) that is the current value of the config option
	 */
	template<dv::Config::AttributeType T>
	const typename dv::Config::AttributeTypeGenerator<T>::type &get() const {
		return (static_cast<const typename dv::Config::AttributeTypeGenerator<T>::type &>(
			getConfigObject<T>().currentValue));
	}

	/**
	 * Updates the current value of this config option. Needs a template paramenter
	 * of the type `dv::ConfigVariant::*` to determine what type of config parameter
	 * to return. The change is propagated to the configuration tree.
	 * @tparam T The config variant type
	 * @param value A simple value (long, string etc) to update the config option with
	 * @param force Force an update of the variable (useful for statistics variables)
	 */
	template<dv::Config::AttributeType T>
	void set(const typename dv::Config::AttributeTypeGenerator<T>::type &value, bool force) {
		auto &config = getConfigObject<T>();

		// No change, no update.
		if (value == config.currentValue) {
			return;
		}

		// Update current value right away, so subsequent get()s see this.
		config.currentValue = value;

		// Update configuration tree. This will also execute all attribute listeners,
		// including the config-change one, which will force a second full update on
		// the next run. That is not optimal, but usually negligible. Rate limiting
		// can be used to ameliorate the situation for often-updated variables.
		if (rateLimit && !rateLimit->pass() && !force) {
			return;
		}

		if (config.updateReadOnly) {
			node.updateReadOnly<T>(key, value);
		}
		else {
			node.put<T>(key, value);
		}
	}

	/**
	 * Creates a dvConfig Attribute in the dv config tree for the object.
	 *
	 * @param moduleNode the module's configuration node.
	 * @param fullKey the key under which the attribute is to be stored. Forward
	 * slashes (/) can be used to get sub-nodes.
	 */
	void createAttribute(dv::Config::Node moduleNode, const std::string &fullKey) {
		setNodeAttrLink(moduleNode, fullKey);

		switch (type) {
			case dv::Config::AttributeType::BOOL: {
				auto &config = getConfigObject<dv::Config::AttributeType::BOOL>();

				node.createAttribute<dv::Config::AttributeType::BOOL>(
					key, config.initValue, {}, config.flags, config.description);

				if (config.attributes.attributeType == dv::_BooleanAttributeType::BUTTON) {
					node.attributeModifierButton(key, config.attributes.buttonLabel);
				}

				break;
			}

			case dv::Config::AttributeType::INT: {
				auto &config = getConfigObject<dv::Config::AttributeType::INT>();

				node.createAttribute<dv::Config::AttributeType::INT>(
					key, config.initValue, config.attributes.range, config.flags, config.description);

				if (!config.attributes.unit.empty()) {
					node.attributeModifierUnit(key, config.attributes.unit);
				}

				break;
			}

			case dv::Config::AttributeType::LONG: {
				auto &config = getConfigObject<dv::Config::AttributeType::LONG>();

				node.createAttribute<dv::Config::AttributeType::LONG>(
					key, config.initValue, config.attributes.range, config.flags, config.description);

				if (!config.attributes.unit.empty()) {
					node.attributeModifierUnit(key, config.attributes.unit);
				}

				break;
			}

			case dv::Config::AttributeType::FLOAT: {
				auto &config = getConfigObject<dv::Config::AttributeType::FLOAT>();

				node.createAttribute<dv::Config::AttributeType::FLOAT>(
					key, config.initValue, config.attributes.range, config.flags, config.description);

				if (!config.attributes.unit.empty()) {
					node.attributeModifierUnit(key, config.attributes.unit);
				}

				break;
			}

			case dv::Config::AttributeType::DOUBLE: {
				auto &config = getConfigObject<dv::Config::AttributeType::DOUBLE>();

				node.createAttribute<dv::Config::AttributeType::DOUBLE>(
					key, config.initValue, config.attributes.range, config.flags, config.description);

				if (!config.attributes.unit.empty()) {
					node.attributeModifierUnit(key, config.attributes.unit);
				}

				break;
			}

			case dv::Config::AttributeType::STRING: {
				auto &config = getConfigObject<dv::Config::AttributeType::STRING>();

				node.createAttribute<dv::Config::AttributeType::STRING>(key,
					static_cast<const std::string_view>(config.initValue), config.attributes.length, config.flags,
					config.description);

				if (config.attributes.type == _StringAttributeType::LIST) {
					std::string listAttribute = boost::algorithm::join(config.attributes.listOptions, ",");

					node.attributeModifierListOptions(
						key, listAttribute, config.attributes.listAllowMultipleSelections);
				}

				if (config.attributes.type == _StringAttributeType::FILE) {
					std::string fileChooserAttribute;

					if (config.attributes.fileMode == FileDialogMode::OPEN) {
						fileChooserAttribute += "OPEN";
					}
					else if (config.attributes.fileMode == FileDialogMode::SAVE) {
						fileChooserAttribute += "SAVE";
					}
					else {
						fileChooserAttribute += "DIRECTORY";
					}

					if (!config.attributes.fileAllowedExtensions.empty()) {
						fileChooserAttribute.push_back(':');
						fileChooserAttribute += config.attributes.fileAllowedExtensions;
					}

					node.attributeModifierFileChooser(key, fileChooserAttribute);
				}

				break;
			}

			case dv::Config::AttributeType::UNKNOWN: {
				break;
			}
		}
	}

	/**
	 * Updates the current value of the ConfigOption based on the value
	 * that is present in the dv config tree.
	 */
	void updateValue() {
		using AT = dv::Config::AttributeType;

		switch (type) {
			case AT::BOOL:
				return _updateValue<AT::BOOL>();
			case AT::INT:
				return _updateValue<AT::INT>();
			case AT::LONG:
				return _updateValue<AT::LONG>();
			case AT::FLOAT:
				return _updateValue<AT::FLOAT>();
			case AT::DOUBLE:
				return _updateValue<AT::DOUBLE>();
			case AT::STRING:
				return _updateValue<AT::STRING>();
			case AT::UNKNOWN:
				break;
		}
	}

	// Static convenience factory methods -------------------------------------------------------
	/**
	 * Factory function. Creates boolean option (checkbox).
	 * @param description A description that describes the purpose of this option
	 * @param defaultValue The default value of the option, defaults to false
	 * @param readOnly If the option is read-only, defaults to false
	 * @return A ConfigOption Object
	 */
	static ConfigOption boolOption(const std::string &description, bool defaultValue = false, bool readOnly = false) {
		return getOption<dv::Config::AttributeType::BOOL>(description, defaultValue,
			{dv::_BooleanAttributeType::CHECKBOX},
			(readOnly) ? (dv::Config::AttributeFlags::READ_ONLY | dv::Config::AttributeFlags::NO_EXPORT)
					   : (dv::Config::AttributeFlags::NORMAL),
			readOnly);
	}

	/**
	 * Factory function. Creates a button option. A button is a boolean that gets
	 * set when clicked. The button gets disabled as long as the boolean value is not reset
	 * @param description A description that describes the purpose of this option
	 * @param defaultValue The default value of the option
	 * @return A ConfigOption Object
	 */
	static ConfigOption buttonOption(const std::string &description, const std::string &buttonLabel) {
		return getOption<dv::Config::AttributeType::BOOL>(
			description, false, {dv::_BooleanAttributeType::BUTTON, buttonLabel}, dv::Config::AttributeFlags::NORMAL);
	}

	/**
	 * Factory function. Creates a integer config option (32 bit).
	 * @param description A description that describes the purpose of this option
	 * @param defaultValue The default value that this option shall have
	 * @param minValue The min value a user can choose for this option
	 * @param maxValue The max value a user can choose for this option
	 * @return A ConfigOption Object
	 */
	static ConfigOption intOption(
		const std::string &description, int32_t defaultValue, int32_t minValue, int32_t maxValue) {
		return getOption<dv::Config::AttributeType::INT>(
			description, defaultValue, {minValue, maxValue}, dv::Config::AttributeFlags::NORMAL);
	}

	/**
	 * Factory function. Creates a integer config option (32 bit).
	 * @param description A description that describes the purpose of this option
	 * @param defaultValue The default value that this option shall have
	 * @return A ConfigOption Object
	 */
	static ConfigOption intOption(const std::string &description, int32_t defaultValue) {
		int32_t sensibleRangeValue = ((std::abs(defaultValue) > 0) ? static_cast<int32_t>(std::pow(10,
										  std::floor(std::log10(std::abs(static_cast<double>(defaultValue))) + 1.0)))
																   : 10);

		int32_t sensibleUpperRange = defaultValue >= 0 ? sensibleRangeValue : 0;
		int32_t sensibleLowerRange = defaultValue >= 0 ? 0 : -sensibleRangeValue;

		return getOption<dv::Config::AttributeType::INT>(
			description, defaultValue, {sensibleLowerRange, sensibleUpperRange}, dv::Config::AttributeFlags::NORMAL);
	}

	/**
	 * Factory function. Creates a long integer config option (64 bit).
	 * @param description A description that describes the purpose of this option
	 * @param defaultValue The default value that this option shall have
	 * @param minValue The min value a user can choose for this option
	 * @param maxValue The max value a user can choose for this option
	 * @return A ConfigOption Object
	 */
	static ConfigOption longOption(
		const std::string &description, int64_t defaultValue, int64_t minValue, int64_t maxValue) {
		return getOption<dv::Config::AttributeType::LONG>(
			description, defaultValue, {minValue, maxValue}, dv::Config::AttributeFlags::NORMAL);
	}

	/**
	 * Factory function. Creates a long integer config option (64 bit).
	 * @param description A description that describes the purpose of this option
	 * @param defaultValue The default value that this option shall have
	 * @return A ConfigOption Object
	 */
	static ConfigOption longOption(const std::string &description, int64_t defaultValue) {
		int64_t sensibleRangeValue = ((std::abs(defaultValue) > 0) ? static_cast<int64_t>(std::pow(10,
										  std::floor(std::log10(std::abs(static_cast<double>(defaultValue))) + 1.0)))
																   : 10);

		int64_t sensibleUpperRange = defaultValue >= 0 ? sensibleRangeValue : 0;
		int64_t sensibleLowerRange = defaultValue >= 0 ? 0 : -sensibleRangeValue;

		return getOption<dv::Config::AttributeType::LONG>(
			description, defaultValue, {sensibleLowerRange, sensibleUpperRange}, dv::Config::AttributeFlags::NORMAL);
	}

	/**
	 * Factory function. Creates a single-precision floating point config option.
	 * @param description A description that describes the purpose of this option
	 * @param defaultValue The default value that this option shall have
	 * @param minValue The min value a user can choose for this option
	 * @param maxValue The max value a user can choose for this option
	 * @return A ConfigOption Object
	 */
	static ConfigOption floatOption(
		const std::string &description, float defaultValue, float minValue, float maxValue) {
		return getOption<dv::Config::AttributeType::FLOAT>(
			description, defaultValue, {minValue, maxValue}, dv::Config::AttributeFlags::NORMAL);
	}

	/**
	 * Factory function. Creates a single-precision floating point config option.
	 * @param description A description that describes the purpose of this option
	 * @param defaultValue The default value that this option shall have
	 * @return A ConfigOption Object
	 */
	static ConfigOption floatOption(const std::string &description, float defaultValue) {
		float sensibleRangeValue = std::max(
			((std::abs(defaultValue) > 0.0f) ? std::pow(10.0f, std::floor(std::log10(std::abs(defaultValue)) + 1.0f))
											 : 1.0f),
			1.0f);

		float sensibleUpperRange = defaultValue >= 0.0f ? sensibleRangeValue : 0.0f;
		float sensibleLowerRange = defaultValue >= 0.0f ? 0.0f : -sensibleRangeValue;

		return getOption<dv::Config::AttributeType::FLOAT>(
			description, defaultValue, {sensibleLowerRange, sensibleUpperRange}, dv::Config::AttributeFlags::NORMAL);
	}

	/**
	 * Factory function. Creates a double-precision floating point config option.
	 * @param description A description that describes the purpose of this option
	 * @param defaultValue The default value that this option shall have
	 * @param minValue The min value a user can choose for this option
	 * @param maxValue The max value a user can choose for this option
	 * @return A ConfigOption Object
	 */
	static ConfigOption doubleOption(
		const std::string &description, double defaultValue, double minValue, double maxValue) {
		return getOption<dv::Config::AttributeType::DOUBLE>(
			description, defaultValue, {minValue, maxValue}, dv::Config::AttributeFlags::NORMAL);
	}

	/**
	 * Factory function. Creates a double-precision floating point config option.
	 * @param description A description that describes the purpose of this option
	 * @param defaultValue The default value that this option shall have
	 * @return A ConfigOption Object
	 */
	static ConfigOption doubleOption(const std::string &description, double defaultValue) {
		double sensibleRangeValue = std::max(
			((std::abs(defaultValue) > 0.0) ? std::pow(10.0, std::floor(std::log10(std::abs(defaultValue)) + 1.0))
											: 1.0),
			1.0);

		double sensibleUpperRange = defaultValue >= 0.0 ? sensibleRangeValue : 0.0;
		double sensibleLowerRange = defaultValue >= 0.0 ? 0.0 : -sensibleRangeValue;

		return getOption<dv::Config::AttributeType::DOUBLE>(
			description, defaultValue, {sensibleLowerRange, sensibleUpperRange}, dv::Config::AttributeFlags::NORMAL);
	}

	/**
	 * Factory function. Creates a string config option.
	 * @param description A description that describes the purpose of this option
	 * @param defaultValue The default value that this option shall have
	 * @return A ConfigOption Object
	 */
	static ConfigOption stringOption(const std::string &description, const std::string &defaultValue) {
		return getOption<dv::Config::AttributeType::STRING>(description, defaultValue,
			{0, INT32_MAX, _StringAttributeType::NORMAL}, dv::Config::AttributeFlags::NORMAL);
	}

	/**
	 * Factory function. Creates a string config option.
	 * @param description A description that describes the purpose of this option
	 * @param defaultValue The default value that this option shall have
	 * @param minValue The min string length a user can input
	 * @param maxValue The max string length a user can input
	 * @return A ConfigOption Object
	 */
	static ConfigOption stringOption(
		const std::string &description, const std::string &defaultValue, int32_t minLength, int32_t maxLength) {
		return getOption<dv::Config::AttributeType::STRING>(description, defaultValue,
			{minLength, maxLength, _StringAttributeType::NORMAL}, dv::Config::AttributeFlags::NORMAL);
	}

	/**
	 * Factory function. Creates a list config option (list of strings).
	 *
	 * @param description A description that describes the purpose of this option
	 * @param defaultChoice The index of the default choice, taken from the subsequent vector
	 * @param choices Vector of possible choices
	 * @param allowMultipleSelection wether to allow selecting multiple options at the same time
	 *
	 * @return A ConfigOption Object
	 */
	static ConfigOption listOption(const std::string &description, size_t defaultChoice,
		const std::vector<std::string> &choices, bool allowMultipleSelection = false) {
		_ConfigAttributes<dv::Config::AttributeType::STRING> attr{0, INT32_MAX, _StringAttributeType::LIST};
		attr.listOptions                 = choices;
		attr.listAllowMultipleSelections = allowMultipleSelection;

		return getOption<dv::Config::AttributeType::STRING>(
			description, choices.at(defaultChoice), attr, dv::Config::AttributeFlags::NORMAL);
	}

	/**
	 * Factory function. Creates a list config option (list of strings).
	 *
	 * @param description A description that describes the purpose of this option
	 * @param defaultChoice The the string of the default choice
	 * @param choices Vector of possible choices
	 * @param allowMultipleSelection wether to allow selecting multiple options at the same time
	 *
	 * @return A ConfigOption Object
	 */
	static ConfigOption listOption(const std::string &description, const std::string &defaultChoice,
		const std::vector<std::string> &choices, bool allowMultipleSelection = false) {
		_ConfigAttributes<dv::Config::AttributeType::STRING> attr{0, INT32_MAX, _StringAttributeType::LIST};
		attr.listOptions                 = choices;
		attr.listAllowMultipleSelections = allowMultipleSelection;

		if (!dv::vectorContains(choices, defaultChoice)) {
			throw std::out_of_range("List config option default choice not in list of choices.");
		}

		return getOption<dv::Config::AttributeType::STRING>(
			description, defaultChoice, attr, dv::Config::AttributeFlags::NORMAL);
	}

	/**
	 * Factory function. Creates a file open config option.
	 * @param description A description that describes the purpose of this option
	 * @return A ConfigOption Object
	 */
	static ConfigOption fileOpenOption(const std::string &description) {
		_ConfigAttributes<dv::Config::AttributeType::STRING> attr{0, PATH_MAX, _StringAttributeType::FILE};
		attr.fileMode              = FileDialogMode::OPEN;
		attr.fileAllowedExtensions = "";

		return getOption<dv::Config::AttributeType::STRING>(description, "", attr, dv::Config::AttributeFlags::NORMAL);
	}

	/**
	 * Factory function. Creates a file open config option.
	 * @param description A description that describes the purpose of this option
	 * @param allowedExtensions The allowed extensions of the file to be opened
	 * @return A ConfigOption Object
	 */
	static ConfigOption fileOpenOption(const std::string &description, const std::string &allowedExtensions) {
		_ConfigAttributes<dv::Config::AttributeType::STRING> attr{0, PATH_MAX, _StringAttributeType::FILE};
		attr.fileMode              = FileDialogMode::OPEN;
		attr.fileAllowedExtensions = allowedExtensions;

		return getOption<dv::Config::AttributeType::STRING>(description, "", attr, dv::Config::AttributeFlags::NORMAL);
	}

	/**
	 * Factory function. Creates a file open config option.
	 * @param description A description that describes the purpose of this option
	 * @param defaultValue The default path of the file
	 * @param allowedExtensions The allowed extensions of the file to be opened
	 * @return A ConfigOption Object
	 */
	static ConfigOption fileOpenOption(
		const std::string &description, const std::string &defaultValue, const std::string &allowedExtensions) {
		_ConfigAttributes<dv::Config::AttributeType::STRING> attr{0, PATH_MAX, _StringAttributeType::FILE};
		attr.fileMode              = FileDialogMode::OPEN;
		attr.fileAllowedExtensions = allowedExtensions;

		return getOption<dv::Config::AttributeType::STRING>(
			description, defaultValue, attr, dv::Config::AttributeFlags::NORMAL);
	}

	/**
	 * Factory function. Creates a file save config option.
	 * @param description A description that describes the purpose of this option
	 * @return A ConfigOption Object
	 */
	static ConfigOption fileSaveOption(const std::string &description) {
		_ConfigAttributes<dv::Config::AttributeType::STRING> attr{0, PATH_MAX, _StringAttributeType::FILE};
		attr.fileMode              = FileDialogMode::SAVE;
		attr.fileAllowedExtensions = "";

		return getOption<dv::Config::AttributeType::STRING>(description, "", attr, dv::Config::AttributeFlags::NORMAL);
	}

	/**
	 * Factory function. Creates a file save config option.
	 * @param description A description that describes the purpose of this option
	 * @param allowedExtensions The allowed extensions of the file to be saved
	 * @return A ConfigOption Object
	 */
	static ConfigOption fileSaveOption(const std::string &description, const std::string &allowedExtensions) {
		_ConfigAttributes<dv::Config::AttributeType::STRING> attr{0, PATH_MAX, _StringAttributeType::FILE};
		attr.fileMode              = FileDialogMode::SAVE;
		attr.fileAllowedExtensions = allowedExtensions;

		return getOption<dv::Config::AttributeType::STRING>(description, "", attr, dv::Config::AttributeFlags::NORMAL);
	}

	/**
	 * Factory function. Creates a file save config option.
	 * @param description A description that describes the purpose of this option
	 * @param defaultValue The default path of the file
	 * @param allowedExtensions The allowed extensions of the file to be saved
	 * @return A ConfigOption Object
	 */
	static ConfigOption fileSaveOption(
		const std::string &description, const std::string &defaultValue, const std::string &allowedExtensions) {
		_ConfigAttributes<dv::Config::AttributeType::STRING> attr{0, PATH_MAX, _StringAttributeType::FILE};
		attr.fileMode              = FileDialogMode::SAVE;
		attr.fileAllowedExtensions = allowedExtensions;

		return getOption<dv::Config::AttributeType::STRING>(
			description, defaultValue, attr, dv::Config::AttributeFlags::NORMAL);
	}

	/**
	 * Factory function. Creates directory choose option
	 * @param description A description that describes the purpose of this option
	 * @return A ConfigOption Object
	 */
	static ConfigOption directoryOption(const std::string &description) {
		_ConfigAttributes<dv::Config::AttributeType::STRING> attr{0, PATH_MAX, _StringAttributeType::FILE};
		attr.fileMode = FileDialogMode::DIRECTORY;

		return getOption<dv::Config::AttributeType::STRING>(description, "", attr, dv::Config::AttributeFlags::NORMAL);
	}

	/**
	 * Factory function. Creates directory choose option
	 * @param description A description that describes the purpose of this option
	 * @param defaultValue The default path of the directory
	 * @return A ConfigOption Object
	 */
	static ConfigOption directoryOption(const std::string &description, const std::string &defaultValue) {
		_ConfigAttributes<dv::Config::AttributeType::STRING> attr{0, PATH_MAX, _StringAttributeType::FILE};
		attr.fileMode = FileDialogMode::DIRECTORY;

		return getOption<dv::Config::AttributeType::STRING>(
			description, defaultValue, attr, dv::Config::AttributeFlags::NORMAL);
	}

	/**
	 * Factory function. Creates read-only statistics option.
	 * @param description A description that describes the purpose of this option
	 * @return A ConfigOption Object
	 */
	static ConfigOption statisticOption(const std::string &description) {
		auto opt = getOption<dv::Config::AttributeType::LONG>(description, 0, {0, INT64_MAX},
			dv::Config::AttributeFlags::READ_ONLY | dv::Config::AttributeFlags::NO_EXPORT, true);

		// Enforce rate limiting for statistics going to the config tree of 1 per second.
		opt.setRateLimit(1, 1000);

		return (opt);
	}
};

class RuntimeConfig {
private:
	std::unordered_map<std::string, ConfigOption> configMap;
	dv::Config::Node moduleNode;

#if defined(DV_MODULE_MULTI_THREAD_SAFE) && DV_MODULE_MULTI_THREAD_SAFE == 1
	mutable std::shared_mutex lock;
#endif

public:
	RuntimeConfig(dv::Config::Node mn) : moduleNode(mn) {
	}

	/**
	 * Adds a new configuration option to the set of config options. Configuration options can be generated
	 * using the `dv::ConfigOption:: ...` factory functions. The key is the name of the config option and has
	 * to be a name that complies with the specifications of a C variable name.
	 * @param key The key or name of this config option. Camel case C compliant name
	 * @param config The config object. Use `dv::ConfigOption:: ...` factory functions to generate them
	 */
	void add(const std::string &key, ConfigOption config) {
#if defined(DV_MODULE_MULTI_THREAD_SAFE) && DV_MODULE_MULTI_THREAD_SAFE == 1
		std::unique_lock l{lock};
#endif

		configMap.insert_or_assign(key, std::move(config));

		auto &cfg = configMap.at(key);
		cfg.createAttribute(moduleNode, key);

		// Ensure value is up-to-date, for example if it already exists
		// because it was loaded from a file.
		cfg.updateValue();
	}

	/**
	 * Sets the priority options for this module. The priority options for the module are the options
	 * that are shown to the user in the gui in the sidebar, without having to click on the plus sign.
	 * Priority options should be the list of the most important options.
	 * @param priorityAttributes The list of the keys of the most important configuration options
	 */
	void setPriorityOptions(const std::vector<std::string> &priorityAttributes) {
		// If empty, just ensure a modifier is present.
		if (priorityAttributes.empty()) {
			moduleNode.attributeModifierPriorityAttributes("");
			return;
		}

		std::unordered_map<std::string, std::vector<std::string>> keys;

		for (const auto &attr : priorityAttributes) {
			size_t pos = attr.find_last_of("/");

			if (pos != std::string::npos) {
				const auto &n = attr.substr(0, pos + 1);
				const auto &k = attr.substr(pos + 1);

				keys[n].push_back(k);
			}
			else {
				keys["./"].push_back(attr);
			}
		}

		for (auto &entry : keys) {
			auto &node = entry.first;
			auto &mods = entry.second;

			dv::vectorSortUnique(mods);

			if (mods.size() > 1) {
				// Multiple modifiers, ensure no empty one is present.
				dv::vectorRemoveIf(mods, [](const auto &str) {
					return str.empty();
				});
			}

			const auto &modifier = boost::algorithm::join(mods, ",");

			moduleNode.getRelativeNode(node).attributeModifierPriorityAttributes(modifier);
		}
	}

	/**
	 * Returns the current value of this configuration option that is stored under the given key.
	 * This is the templated generic function, there are overloaded helper functions available for all
	 * supported types
	 * @tparam T The AttributeType type of the attribute
	 * @param key The name under which this configuration option is stored
	 * @return The currently stored value of this configuration option
	 */
	template<dv::Config::AttributeType T>
	const typename dv::Config::AttributeTypeGenerator<T>::type &get(const std::string &key) const {
#if defined(DV_MODULE_MULTI_THREAD_SAFE) && DV_MODULE_MULTI_THREAD_SAFE == 1
		std::shared_lock l{lock};
#endif

		if (configMap.count(key) == 0) {
			throw std::out_of_range("RuntimeConfig.get(\"" + key + "\"): key doesn't exist.");
		}

		const auto &cfg = configMap.at(key);

		if constexpr (dv::DEBUG_ENABLED) {
			if (cfg.getType() != T) {
				throw std::runtime_error(
					"RuntimeConfig.get(\"" + key + "\"): key type and given template type are not the same.");
			}
		}

		return (cfg.get<T>());
	}

	/**
	 * Sets the configuration option under the given name to the provided value. Requires the attribute type
	 * as a template argument. There are non-templated convenience functions available for all supported
	 * attribute types. Note that this function is only guaranteed when called within `configUpdate`, calling
	 * it outside of configUpdate, the behaviour is undefined.
	 * @tparam T The type of the attribute
	 * @param key The name under which this configuration option is stored
	 * @param value The value to be stored under the config options
	 * @param force Force an update of the variable (useful for statistics variables)
	 */
	template<dv::Config::AttributeType T>
	void set(
		const std::string &key, const typename dv::Config::AttributeTypeGenerator<T>::type &value, bool force = false) {
#if defined(DV_MODULE_MULTI_THREAD_SAFE) && DV_MODULE_MULTI_THREAD_SAFE == 1
		std::unique_lock l{lock};
#endif

		if (configMap.count(key) == 0) {
			throw std::out_of_range("RuntimeConfig.set(\"" + key + "\"): key doesn't exist.");
		}

		auto &cfg = configMap.at(key);

		if constexpr (dv::DEBUG_ENABLED) {
			if (cfg.getType() != T) {
				throw std::runtime_error(
					"RuntimeConfig.set(\"" + key + "\"): key type and given template type are not the same.");
			}
		}

		cfg.set<T>(value, force);
	}

	void update() {
#if defined(DV_MODULE_MULTI_THREAD_SAFE) && DV_MODULE_MULTI_THREAD_SAFE == 1
		std::unique_lock l{lock};
#endif

		for (auto &entry : configMap) {
			entry.second.updateValue();
		}
	}

	/**
	 * Sets the value of a bool config option. Not that this function is only guaranteed when called
	 * in the `configUpdate` function. Calling it outside of it can result in undefined behaviour
	 * @param key The key of the config option
	 * @param value The value to be set
	 */
	inline void setBool(const std::string &key, bool value) {
		set<dv::Config::AttributeType::BOOL>(key, value);
	}

	/**
	 * Sets the value of an int config option. Not that this function is only guaranteed when called
	 * in the `configUpdate` function. Calling it outside of it can result in undefined behaviour
	 * @param key The key of the config option
	 * @param value The value to be set
	 */
	inline void setInt(const std::string &key, int32_t value) {
		set<dv::Config::AttributeType::INT>(key, value);
	}

	/**
	 * Sets the value of a long config option. Not that this function is only guaranteed when called
	 * in the `configUpdate` function. Calling it outside of it can result in undefined behaviour
	 * @param key The key of the config option
	 * @param value The value to be set
	 */
	inline void setLong(const std::string &key, int64_t value) {
		set<dv::Config::AttributeType::LONG>(key, value);
	}

	/**
	 * Sets the value of a float config option. Not that this function is only guaranteed when called
	 * in the `configUpdate` function. Calling it outside of it can result in undefined behaviour
	 * @param key The key of the config option
	 * @param value The value to be set
	 */
	inline void setFloat(const std::string &key, float value) {
		set<dv::Config::AttributeType::FLOAT>(key, value);
	}

	/**
	 * Sets the value of a double config option. Not that this function is only guaranteed when called
	 * in the `configUpdate` function. Calling it outside of it can result in undefined behaviour
	 * @param key The key of the config option
	 * @param value The value to be set
	 */
	inline void setDouble(const std::string &key, double value) {
		set<dv::Config::AttributeType::DOUBLE>(key, value);
	}

	/**
	 * Sets the value of an int config option. Not that this function is only guaranteed when called
	 * in the `configUpdate` function. Calling it outside of it can result in undefined behaviour
	 * @param key The key of the config option
	 * @param value The value to be set
	 */
	inline void setString(const std::string &key, const std::string &value) {
		set<dv::Config::AttributeType::STRING>(key, value);
	}

	/**
	 * Returns the value of the config option with the given key.
	 * Note that this function will only work if the type of the option is actually bool
	 * @param key the key of the option to look up
	 * @return the value of the option
	 */
	inline bool getBool(const std::string &key) const {
		return get<dv::CfgType::BOOL>(key);
	}

	/**
	 * Returns the value of the config option with the given key.
	 * Note that this function will only work if the type of the option is actually int
	 * @param key the key of the option to look up
	 * @return the value of the option
	 */
	inline int32_t getInt(const std::string &key) const {
		return get<dv::CfgType::INT>(key);
	}

	/**
	 * Returns the value of the config option with the given key.
	 * Note that this function will only work if the type of the option is actually long
	 * @param key the key of the option to look up
	 * @return the value of the option
	 */
	inline int64_t getLong(const std::string &key) const {
		return get<dv::CfgType::LONG>(key);
	}

	/**
	 * 	Returns the value of the config option with the given key.
	 * 	Note that this function will only work if the type of the option is actually float
	 * 	@param key the key of the option to look up
	 * 	@return the value of the option
	 */
	inline float getFloat(const std::string &key) const {
		return get<dv::CfgType::FLOAT>(key);
	}

	/**
	 * Returns the value of the config option with the given key.
	 * Note that this function will only work if the type of the option is actually double
	 * @param key the key of the option to look up
	 * @return the value of the option
	 */
	inline double getDouble(const std::string &key) const {
		return get<dv::CfgType::DOUBLE>(key);
	}

	/**
	 * Returns the value of the config option with the given key.
	 * Note that this function will only work if the type of the option is actually string
	 * @param key the key of the option to look up
	 * @return the value of the option
	 */
	inline const std::string &getString(const std::string &key) const {
		return get<dv::CfgType::STRING>(key);
	}

	inline void set(const std::string &key, bool value) {
#if defined(DV_MODULE_MULTI_THREAD_SAFE) && DV_MODULE_MULTI_THREAD_SAFE == 1
		std::unique_lock l{lock};
#endif

		if (configMap.count(key) == 0) {
			throw std::out_of_range("RuntimeConfig.set(\"" + key + "\"): key doesn't exist.");
		}

		auto &cfg = configMap.at(key);

		if (cfg.getType() == dv::CfgType::BOOL) {
			cfg.set<dv::CfgType::BOOL>(value, false);
		}
		else {
			throw std::out_of_range("RuntimeConfig.set(\"" + key + "\"): given key is not BOOL.");
		}
	}

	inline void set(const std::string &key, int32_t value) {
#if defined(DV_MODULE_MULTI_THREAD_SAFE) && DV_MODULE_MULTI_THREAD_SAFE == 1
		std::unique_lock l{lock};
#endif

		if (configMap.count(key) == 0) {
			throw std::out_of_range("RuntimeConfig.set(\"" + key + "\"): key doesn't exist.");
		}

		auto &cfg = configMap.at(key);

		if (cfg.getType() == dv::CfgType::INT) {
			cfg.set<dv::CfgType::INT>(value, false);
		}
		else if (cfg.getType() == dv::CfgType::LONG) {
			cfg.set<dv::CfgType::LONG>(value, false);
		}
		else {
			throw std::out_of_range("RuntimeConfig.set(\"" + key + "\"): given key is not INT or LONG.");
		}
	}

	inline void set(const std::string &key, uint32_t value) {
#if defined(DV_MODULE_MULTI_THREAD_SAFE) && DV_MODULE_MULTI_THREAD_SAFE == 1
		std::unique_lock l{lock};
#endif

		if (configMap.count(key) == 0) {
			throw std::out_of_range("RuntimeConfig.set(\"" + key + "\"): key doesn't exist.");
		}

		auto &cfg = configMap.at(key);

		if (cfg.getType() == dv::CfgType::INT) {
			if (value <= INT32_MAX) {
				cfg.set<dv::CfgType::INT>(static_cast<int32_t>(value), false);
			}
			else {
				throw std::out_of_range("RuntimeConfig.set(\"" + key + "\"): UINT value exceeds INT range.");
			}
		}
		else if (cfg.getType() == dv::CfgType::LONG) {
			cfg.set<dv::CfgType::LONG>(value, false);
		}
		else {
			throw std::out_of_range("RuntimeConfig.set(\"" + key + "\"): given key is not INT or LONG.");
		}
	}

	inline void set(const std::string &key, int64_t value) {
#if defined(DV_MODULE_MULTI_THREAD_SAFE) && DV_MODULE_MULTI_THREAD_SAFE == 1
		std::unique_lock l{lock};
#endif

		if (configMap.count(key) == 0) {
			throw std::out_of_range("RuntimeConfig.set(\"" + key + "\"): key doesn't exist.");
		}

		auto &cfg = configMap.at(key);

		if (cfg.getType() == dv::CfgType::LONG) {
			cfg.set<dv::CfgType::LONG>(value, false);
		}
		else {
			throw std::out_of_range("RuntimeConfig.set(\"" + key + "\"): given key is not LONG.");
		}
	}

	inline void set(const std::string &key, uint64_t value) {
#if defined(DV_MODULE_MULTI_THREAD_SAFE) && DV_MODULE_MULTI_THREAD_SAFE == 1
		std::unique_lock l{lock};
#endif

		if (configMap.count(key) == 0) {
			throw std::out_of_range("RuntimeConfig.set(\"" + key + "\"): key doesn't exist.");
		}

		auto &cfg = configMap.at(key);

		if (cfg.getType() == dv::CfgType::LONG) {
			if (value <= INT64_MAX) {
				cfg.set<dv::CfgType::LONG>(static_cast<int64_t>(value), false);
			}
			else {
				throw std::out_of_range("RuntimeConfig.set(\"" + key + "\"): ULONG value exceeds LONG range.");
			}
		}
		else {
			throw std::out_of_range("RuntimeConfig.set(\"" + key + "\"): given key is not LONG.");
		}
	}

	inline void set(const std::string &key, float value) {
#if defined(DV_MODULE_MULTI_THREAD_SAFE) && DV_MODULE_MULTI_THREAD_SAFE == 1
		std::unique_lock l{lock};
#endif

		if (configMap.count(key) == 0) {
			throw std::out_of_range("RuntimeConfig.set(\"" + key + "\"): key doesn't exist.");
		}

		auto &cfg = configMap.at(key);

		if (cfg.getType() == dv::CfgType::FLOAT) {
			cfg.set<dv::CfgType::FLOAT>(value, false);
		}
		else if (cfg.getType() == dv::CfgType::DOUBLE) {
			cfg.set<dv::CfgType::DOUBLE>(static_cast<double>(value), false);
		}
		else {
			throw std::out_of_range("RuntimeConfig.set(\"" + key + "\"): given key is not FLOAT or DOUBLE.");
		}
	}

	inline void set(const std::string &key, double value) {
#if defined(DV_MODULE_MULTI_THREAD_SAFE) && DV_MODULE_MULTI_THREAD_SAFE == 1
		std::unique_lock l{lock};
#endif

		if (configMap.count(key) == 0) {
			throw std::out_of_range("RuntimeConfig.set(\"" + key + "\"): key doesn't exist.");
		}

		auto &cfg = configMap.at(key);

		if (cfg.getType() == dv::CfgType::DOUBLE) {
			cfg.set<dv::CfgType::DOUBLE>(value, false);
		}
		else {
			throw std::out_of_range("RuntimeConfig.set(\"" + key + "\"): given key is not DOUBLE.");
		}
	}

	inline void set(const std::string &key, const std::string &value) {
#if defined(DV_MODULE_MULTI_THREAD_SAFE) && DV_MODULE_MULTI_THREAD_SAFE == 1
		std::unique_lock l{lock};
#endif

		if (configMap.count(key) == 0) {
			throw std::out_of_range("RuntimeConfig.set(\"" + key + "\"): key doesn't exist.");
		}

		auto &cfg = configMap.at(key);

		if (cfg.getType() == dv::CfgType::STRING) {
			cfg.set<dv::CfgType::STRING>(value, false);
		}
		else {
			throw std::out_of_range("RuntimeConfig.set(\"" + key + "\"): given key is not STRING.");
		}
	}

	inline void get(const std::string &key, bool &value) const {
#if defined(DV_MODULE_MULTI_THREAD_SAFE) && DV_MODULE_MULTI_THREAD_SAFE == 1
		std::shared_lock l{lock};
#endif

		if (configMap.count(key) == 0) {
			throw std::out_of_range("RuntimeConfig.get(\"" + key + "\"): key doesn't exist.");
		}

		auto &cfg = configMap.at(key);

		if (cfg.getType() == dv::CfgType::BOOL) {
			value = cfg.get<dv::CfgType::BOOL>();
		}
		else {
			throw std::out_of_range("RuntimeConfig.get(\"" + key + "\"): given key is not BOOL.");
		}
	}

	inline void get(const std::string &key, int32_t &value) const {
#if defined(DV_MODULE_MULTI_THREAD_SAFE) && DV_MODULE_MULTI_THREAD_SAFE == 1
		std::shared_lock l{lock};
#endif

		if (configMap.count(key) == 0) {
			throw std::out_of_range("RuntimeConfig.get(\"" + key + "\"): key doesn't exist.");
		}

		auto &cfg = configMap.at(key);

		if (cfg.getType() == dv::CfgType::INT) {
			value = cfg.get<dv::CfgType::INT>();
		}
		else {
			throw std::out_of_range("RuntimeConfig.get(\"" + key + "\"): given key is not INT.");
		}
	}

	inline void get(const std::string &key, uint32_t &value) const {
#if defined(DV_MODULE_MULTI_THREAD_SAFE) && DV_MODULE_MULTI_THREAD_SAFE == 1
		std::shared_lock l{lock};
#endif

		if (configMap.count(key) == 0) {
			throw std::out_of_range("RuntimeConfig.get(\"" + key + "\"): key doesn't exist.");
		}

		auto &cfg = configMap.at(key);

		if (cfg.getType() == dv::CfgType::INT) {
			auto v = cfg.get<dv::CfgType::INT>();

			if (v >= 0) {
				value = static_cast<uint32_t>(v);
			}
			else {
				throw std::out_of_range("RuntimeConfig.get(\"" + key + "\"): INT value exceeds UINT range.");
			}
		}
		else {
			throw std::out_of_range("RuntimeConfig.get(\"" + key + "\"): given key is not INT.");
		}
	}

	inline void get(const std::string &key, int64_t &value) const {
#if defined(DV_MODULE_MULTI_THREAD_SAFE) && DV_MODULE_MULTI_THREAD_SAFE == 1
		std::shared_lock l{lock};
#endif

		if (configMap.count(key) == 0) {
			throw std::out_of_range("RuntimeConfig.get(\"" + key + "\"): key doesn't exist.");
		}

		auto &cfg = configMap.at(key);

		if (cfg.getType() == dv::CfgType::INT) {
			value = cfg.get<dv::CfgType::INT>();
		}
		else if (cfg.getType() == dv::CfgType::LONG) {
			value = cfg.get<dv::CfgType::LONG>();
		}
		else {
			throw std::out_of_range("RuntimeConfig.get(\"" + key + "\"): given key is not INT or LONG.");
		}
	}

	inline void get(const std::string &key, uint64_t &value) const {
#if defined(DV_MODULE_MULTI_THREAD_SAFE) && DV_MODULE_MULTI_THREAD_SAFE == 1
		std::shared_lock l{lock};
#endif

		if (configMap.count(key) == 0) {
			throw std::out_of_range("RuntimeConfig.get(\"" + key + "\"): key doesn't exist.");
		}

		auto &cfg = configMap.at(key);

		if (cfg.getType() == dv::CfgType::INT) {
			auto v = cfg.get<dv::CfgType::INT>();

			if (v >= 0) {
				value = static_cast<uint64_t>(v);
			}
			else {
				throw std::out_of_range("RuntimeConfig.get(\"" + key + "\"): INT value exceeds ULONG range.");
			}
		}
		else if (cfg.getType() == dv::CfgType::LONG) {
			auto v = cfg.get<dv::CfgType::LONG>();

			if (v >= 0) {
				value = static_cast<uint64_t>(v);
			}
			else {
				throw std::out_of_range("RuntimeConfig.get(\"" + key + "\"): LONG value exceeds ULONG range.");
			}
		}
		else {
			throw std::out_of_range("RuntimeConfig.get(\"" + key + "\"): given key is not INT or LONG.");
		}
	}

	inline void get(const std::string &key, float &value) const {
#if defined(DV_MODULE_MULTI_THREAD_SAFE) && DV_MODULE_MULTI_THREAD_SAFE == 1
		std::shared_lock l{lock};
#endif

		if (configMap.count(key) == 0) {
			throw std::out_of_range("RuntimeConfig.get(\"" + key + "\"): key doesn't exist.");
		}

		auto &cfg = configMap.at(key);

		if (cfg.getType() == dv::CfgType::FLOAT) {
			value = cfg.get<dv::CfgType::FLOAT>();
		}
		else {
			throw std::out_of_range("RuntimeConfig.get(\"" + key + "\"): given key is not FLOAT.");
		}
	}

	inline void get(const std::string &key, double &value) const {
#if defined(DV_MODULE_MULTI_THREAD_SAFE) && DV_MODULE_MULTI_THREAD_SAFE == 1
		std::shared_lock l{lock};
#endif

		if (configMap.count(key) == 0) {
			throw std::out_of_range("RuntimeConfig.get(\"" + key + "\"): key doesn't exist.");
		}

		auto &cfg = configMap.at(key);

		if (cfg.getType() == dv::CfgType::FLOAT) {
			value = static_cast<double>(cfg.get<dv::CfgType::FLOAT>());
		}
		else if (cfg.getType() == dv::CfgType::DOUBLE) {
			value = cfg.get<dv::CfgType::DOUBLE>();
		}
		else {
			throw std::out_of_range("RuntimeConfig.get(\"" + key + "\"): given key is not FLOAT or DOUBLE.");
		}
	}

	inline void get(const std::string &key, std::string &value) const {
#if defined(DV_MODULE_MULTI_THREAD_SAFE) && DV_MODULE_MULTI_THREAD_SAFE == 1
		std::shared_lock l{lock};
#endif

		if (configMap.count(key) == 0) {
			throw std::out_of_range("RuntimeConfig.get(\"" + key + "\"): key doesn't exist.");
		}

		auto &cfg = configMap.at(key);

		if (cfg.getType() == dv::CfgType::STRING) {
			value = cfg.get<dv::CfgType::STRING>();
		}
		else {
			throw std::out_of_range("RuntimeConfig.get(\"" + key + "\"): given key is not STRING.");
		}
	}
};

} // namespace dv

#endif // DV_SDK_CONFIG_HPP
