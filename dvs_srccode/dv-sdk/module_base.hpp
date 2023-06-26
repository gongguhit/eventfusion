#ifndef DV_SDK_MODULE_BASE_HPP
#define DV_SDK_MODULE_BASE_HPP

#include "config.hpp"
#include "log.hpp"
#include "module.h"
#include "module_io.hpp"
#include "utils.h"

#include <utility>

// Allow modules to have multiple threads by making a module's global objects
// (log, inputs, ...) safe to be used concurrently by multiple threads.
// This has a performance impact and is disabled by default.
#ifndef DV_MODULE_MULTI_THREAD_SAFE
#	define DV_MODULE_MULTI_THREAD_SAFE 0
#endif

namespace dv {

/**
 * The dv ModuleBase. Every module shall inherit from this module.
 * The base Module provides the following:
 * - Abstraction of the configuration
 * - Input / output management
 */
class ModuleBase {
private:
	/**
	 * moduleData and getDefaultConfig are thread-local to avoid race-conditions between
	 * them being initialized here as nullptr and being set in module.hpp.
	 * moduleData is different at runtime for each module and must be thread-local.
	 * getDefaultConfig is non-atomic and is thread-local for extra safety.
	 */
	static inline thread_local dvModuleData __moduleData{nullptr};
	static inline thread_local void (*__getDefaultConfig)(RuntimeConfig &){nullptr};

public:
	/**
	 * Static config init function. Calles the user provided `initConfigOptions` function
	 * which exists in this class as a static called `__getDefaultConfig`.
	 * It generates the default config and creates the elements for the default
	 * config in the DV config tree.
	 *
	 * @param moduleNode The dvConfig node for which the config should be generated.
	 */
	static void staticConfigInit(dv::Config::Node moduleNode) {
		// read config options from static user provided function
		RuntimeConfig defaultConfig{moduleNode};
		(*__getDefaultConfig)(defaultConfig);
	}

	/**
	 * __INTERNAL USE ONLY__
	 * Sets the static, thread local module data to be used by
	 * a subsequent constructor. This shall only be used prior to
	 * @param _moduleData The moduleData param to be used for
	 * ModuleBase member initialization upon constructor
	 */
	static void __setStaticModuleData(dvModuleData _moduleData) {
		__moduleData = _moduleData;
	}

	/**
	 * __INTERNAL USE ONLY__
	 * Sets the `__getDefaultConfig` static function to the user provided
	 * static function that generates the default config map.
	 * The reference to this function is used since there is no access
	 * to the child - subclass static functions possible from this class.
	 * The default config is both generated before instantiation in
	 * a call to `staticConfigInit` as well as in the constructor
	 * at runtime.
	 * @param _getDefaultConfig
	 */
	static void __setStaticGetDefaultConfig(void (*_getDefaultConfig)(RuntimeConfig &)) {
		__getDefaultConfig = _getDefaultConfig;
	}

	/**
	 * DV low-level module data. Use it to access the low-level DV API.
	 */
	dvModuleData moduleData;

	/**
	 * Loggers for the module. Each module has their own to avoid interference.
	 */
#if defined(DV_MODULE_MULTI_THREAD_SAFE) && DV_MODULE_MULTI_THREAD_SAFE == 1
	static inline thread_local Logger log{};
#else
	Logger log;
#endif

	/**
	 * The module configuration node. Use it to access configuration at
	 * a lower level than the RuntimeConfigMap.
	 */
	dv::Config::Node moduleNode;

	/**
	 * Allows easy access to configuration data and is automatically
	 * updated with new values on changes in the configuration tree.
	 */
	RuntimeConfig config;

	/**
	 * Access data inputs and related actions in a type-safe manner.
	 */
	RuntimeInputs inputs;

	/**
	 * Access data outputs and related actions in a type-safe manner.
	 */
	RuntimeOutputs outputs;

	/**
	 * Base module constructor. The base module constructor initializes
	 * the logger and config members of the class, by utilizing the
	 * `static_thread` local pointer to the DV moduleData pointer
	 * provided prior to constructrion. This makes sure, that logger
	 * and config are available at the time the subclass constructor is
	 * called.
	 */
	ModuleBase() :
		moduleData(__moduleData),
		moduleNode(moduleData->moduleNode),
		config(moduleNode),
		inputs(moduleData),
		outputs(moduleData) {
		dv::runtime_assert(__moduleData != nullptr, "module-data cannot be NULL");

		// Initialize the config map with the default config.
		(*__getDefaultConfig)(config);

		// Add standard config.
		config.add("logLevel",
			dv::ConfigOption::stringOption(
				moduleNode.getAttributeDescription<dv::CfgType::STRING>("logLevel"), moduleNode.getString("logLevel")));
		config.add("running",
			dv::ConfigOption::boolOption(moduleNode.getAttributeDescription<dv::CfgType::BOOL>("running"), true));
	}

	// Virtual default constructor for polymorphism.
	virtual ~ModuleBase() = default;

	// Don't allow move/copy of modules, or assigning them around.
	ModuleBase(const ModuleBase &m) = delete;
	ModuleBase(ModuleBase &&m)      = delete;

	ModuleBase &operator=(const ModuleBase &rhs) = delete;
	ModuleBase &operator=(ModuleBase &&rhs)      = delete;

	/**
	 * Internal method that gets called whenever a config gets changed.
	 * It first merges the new config from the config tree into a runtime
	 * dictionary (map). After that, a user-provided function is called
	 * with which the user has the possibility to copy out config values.
	 */
	void configInternal() {
		// merges the changes from the config tree into the runtime dict
		config.update();

		// calls a user overloadable function to handle config changes
		configUpdate();
	}

	/**
	 * Virtual function to be implemented by the user. Can be left empty.
	 * Called on configuration update, allows more advanced control of how
	 * configuration values are updated.
	 */
	virtual void configUpdate() {
	}

	/**
	 * Internal method that gets called to run the main user code.
	 * Will call the user-provided run() function and then
	 * advance all input data objects by one.
	 */
	void runInternal() {
		// advance all inputs
		dvModuleInputAdvance(moduleData, "");

		// call user run
		run();
	}

	/**
	 * Virtual function to be implemented by the user.
	 * Main function that runs the module and handles data.
	 */
	virtual void run() = 0;
};

} // namespace dv

#endif // DV_SDK_MODULE_BASE_HPP
