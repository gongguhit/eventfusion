#ifndef DV_SDK_MODULE_HPP
#define DV_SDK_MODULE_HPP

#include "module_base.hpp"

#include <boost/core/demangle.hpp>
#include <boost/tti/has_static_member_function.hpp>

#include <typeinfo>

/**
 * Macro that expands into the global `dvModuleGetInfo150` function, exposed to the API for DV.
 * The function instantiates the `ModuleStaticDefinition` class with the given Module (A subclass)
 * of `dv::ModuleBase` and returns the static info section.
 * @param MODULE
 */
#define registerModuleClass(MODULE) \
	dvModuleInfo dvModuleGetInfo150() { return &(dv::ModuleStatics<MODULE>::info); }

namespace dv {

/**
 * Trait for the existence of a static initDescription method with const char* return value
 * @tparam T The class to be tested
 */
BOOST_TTI_HAS_STATIC_MEMBER_FUNCTION(initDescription)
template<typename T>
inline constexpr bool has_initDescription = has_static_member_function_initDescription<T, const char *>::value;

/**
 * Trait for the existence of a static initConfigOptions method with map argument
 * @tparam T The class to be tested
 */
BOOST_TTI_HAS_STATIC_MEMBER_FUNCTION(initConfigOptions)
template<typename T>
inline constexpr bool has_initConfigOptions
	= has_static_member_function_initConfigOptions<T, void(RuntimeConfig &)>::value;

BOOST_TTI_HAS_STATIC_MEMBER_FUNCTION(advancedStaticInit)
template<typename T>
inline constexpr bool has_advancedStaticInit
	= has_static_member_function_advancedStaticInit<T, void(dvModuleData)>::value;

BOOST_TTI_HAS_STATIC_MEMBER_FUNCTION(initTypes)
template<typename T>
inline constexpr bool has_initTypes
	= has_static_member_function_initTypes<T, void(std::vector<dv::Types::Type> &)>::value;

BOOST_TTI_HAS_STATIC_MEMBER_FUNCTION(initInputs)
template<typename T>
inline constexpr bool has_initInputs = has_static_member_function_initInputs<T, void(dv::InputDefinitionList &)>::value;

BOOST_TTI_HAS_STATIC_MEMBER_FUNCTION(initOutputs)
template<typename T>
inline constexpr bool has_initOutputs
	= has_static_member_function_initOutputs<T, void(dv::OutputDefinitionList &)>::value;

/**
 * Pure static template class that provides the static C interface functions
 * for the module class `T` to be exposed to DV. It essentially wraps the
 * the functions of the given C++ module class to the stateless
 * functions and external state of the DV interface. Template parameter T must be
 * a valid DV module, e.g. it has to extend `dv::ModuleBase` and provide
 * certain static functions. Upon (static) instantiation, the `ModuleStatics`
 * class performs static (compile time) validations to check if `T` is a valid
 * module.
 * @tparam T A valid DV Module class that extends `dv::ModuleBase`
 */
template<class T>
class ModuleStatics {
	/* Static assertions. Checks the existence of all required static members. */
	static_assert(std::is_base_of_v<ModuleBase, T>, "Your module does not inherit from dv::ModuleBase.");
	static_assert(has_initDescription<T>,
		"Your module does not specify a `static const char* initDescription()` function."
		"This function should return a string with a description of the module.");
	static_assert(has_initConfigOptions<T>,
		"Your module does not specify a `static void initConfigOptions(dv::RuntimeConfig &config)` "
		"function. This function should insert desired config options into the map.");

public:
	/**
	 * Wrapper for the `staticInit` DV function. Performs a static call to the
	 * `configInit<T>` function of `ModuleBase`, which in turn gets the config from
	 * the user defined module `T`. The config then gets parsed and injected as DvConfig
	 * nodes.
	 * @param moduleData The DV provided moduleData.
	 * @return true if static initialization succeeded, false if it failed.
	 */
	static bool staticInit(dvModuleData moduleData) {
		if constexpr (has_initTypes<T>) {
			try {
				std::vector<dv::Types::Type> types;

				T::initTypes(types);

				for (const auto &t : types) {
					dvModuleRegisterType(moduleData, t);
				}
			}
			catch (const std::exception &ex) {
				dv::Log(dv::logLevel::ERROR, "moduleStaticInit(): failed adding types, exception '{:s} :: {:s}'.",
					boost::core::demangle(typeid(ex).name()), ex.what());
				return (false);
			}
		}

		if constexpr (has_initInputs<T>) {
			try {
				dv::InputDefinitionList inputs;

				T::initInputs(inputs);

				for (const auto &in : inputs.getInputs()) {
					dvModuleRegisterInput(moduleData, in.name.c_str(), in.typeName.c_str(), in.optional);
				}
			}
			catch (const std::exception &ex) {
				dv::Log(dv::logLevel::ERROR, "moduleStaticInit(): failed adding inputs, exception '{:s} :: {:s}'.",
					boost::core::demangle(typeid(ex).name()), ex.what());
				return (false);
			}
		}

		if constexpr (has_initOutputs<T>) {
			try {
				dv::OutputDefinitionList outputs;

				T::initOutputs(outputs);

				for (const auto &out : outputs.getOutputs()) {
					dvModuleRegisterOutput(moduleData, out.name.c_str(), out.typeName.c_str());
				}
			}
			catch (const std::exception &ex) {
				dv::Log(dv::logLevel::ERROR, "moduleStaticInit(): failed adding outputs, exception '{:s} :: {:s}'.",
					boost::core::demangle(typeid(ex).name()), ex.what());
				return (false);
			}
		}

		try {
			ModuleBase::__setStaticGetDefaultConfig(&T::initConfigOptions);

			ModuleBase::staticConfigInit(moduleData->moduleNode);
		}
		catch (const std::exception &ex) {
			dv::Log(dv::logLevel::ERROR, "moduleStaticInit(): failed static config init, exception '{:s} :: {:s}'.",
				boost::core::demangle(typeid(ex).name()), ex.what());
			return (false);
		}

		if constexpr (has_advancedStaticInit<T>) {
			try {
				T::advancedStaticInit(moduleData);
			}
			catch (const std::exception &ex) {
				dv::Log(dv::logLevel::ERROR,
					"moduleStaticInit(): failed advanced static init, exception '{:s} :: {:s}'.",
					boost::core::demangle(typeid(ex).name()), ex.what());
				return (false);
			}
		}

		return (true);
	}

	/**
	 * Wrapper for the `init` DV function. Constructs the user defined `T` module
	 * into the module state. Configuration is updated by the ModuleBase constructor.
	 * @param moduleData The DV provided moduleData.
	 * @return true if runtime initialization succeeded, false if it failed.
	 */
	static bool init(dvModuleData moduleData) {
		try {
			// Set the static fields prior to construction.
			ModuleBase::__setStaticGetDefaultConfig(&T::initConfigOptions);
			ModuleBase::__setStaticModuleData(moduleData);

			// Construct T, will call ModuleBase() and then T() constructors.
			new (moduleData->moduleState) T();
		}
		catch (const std::exception &ex) {
			dv::Log(dv::logLevel::ERROR, "moduleInit(): '{:s} :: {:s}', disabling module.",
				boost::core::demangle(typeid(ex).name()), ex.what());
			return (false);
		}

		return (true);
	}

	/**
	 * Wrapper for the stateless `run` DV function. Relays the call to the user
	 * defined `run` function of the user defined `T` module, which exists as the
	 * module state.
	 * @param moduleData The DV provided moduleData. Used to extract the state.
	 * @param in The input data to be processed by the module.
	 * @param out Pointer to the output data.
	 */
	static void run(dvModuleData moduleData) {
		try {
			static_cast<T *>(moduleData->moduleState)->runInternal();
		}
		catch (const std::exception &ex) {
			dv::Log(dv::logLevel::ERROR, "moduleRun(): '{:s} :: {:s}', disabling module.",
				boost::core::demangle(typeid(ex).name()), ex.what());

			dv::Cfg::Node(moduleData->moduleNode).put<dv::CfgType::BOOL>("running", false);
		}
	}

	/**
	 * Deconstructs the user defined `T` module in the state by calling
	 * its destructor.
	 * @param moduleData The DV provided moduleData.
	 */
	static void exit(dvModuleData moduleData) {
		try {
			std::destroy_at(static_cast<T *>(moduleData->moduleState));
		}
		catch (const std::exception &ex) {
			dv::Log(dv::logLevel::ERROR, "moduleExit(): '{:s} :: {:s}', disabling module.",
				boost::core::demangle(typeid(ex).name()), ex.what());

			dv::Cfg::Node(moduleData->moduleNode).put<dv::CfgType::BOOL>("running", false);
		}
	}

	/**
	 * Wrapper for the DV config function. Relays the call to the stateful
	 * `configUpdate` function of the `T` module. If not overloaded by a the user,
	 * the `configUpdate` function of `ModuleBase` is called which reads out all
	 * config from the DvConfig node and updates a runtime dict of configs.
	 * @param moduleData The moduleData provided by DV.
	 */
	static void config(dvModuleData moduleData) {
		try {
			static_cast<T *>(moduleData->moduleState)->configInternal();
		}
		catch (const std::exception &ex) {
			dv::Log(dv::logLevel::ERROR, "moduleConfig(): '{:s} :: {:s}', disabling module.",
				boost::core::demangle(typeid(ex).name()), ex.what());

			dv::Cfg::Node(moduleData->moduleNode).put<dv::CfgType::BOOL>("running", false);
		}
	}

	/**
	 * Static definition of the dvModuleFunctionsS struct. This struct
	 * gets filles with the static wrapper functions provided in this class
	 * at compile time.
	 */
	static const struct dvModuleFunctionsS functions;

	/**
	 * Static definition of the dvModuleInfoS struct. This struct
	 * gets filled with the static information from the user provided `T` module.
	 */
	static const struct dvModuleInfoS info;
};

/**
 * Static definition of the `ModuleStatics::functions` struct. This struct
 * contains the addresses to all the wrapper functions instantiated to the template
 * module `T`. This struct is then passed to DV to allow it to access the
 * functionalities inside the module.
 * @tparam T The user defined module. Must inherit from `dv::ModuleBase`
 */
template<class T>
const dvModuleFunctionsS ModuleStatics<T>::functions = {&ModuleStatics<T>::staticInit, &ModuleStatics<T>::init,
	&ModuleStatics<T>::run, &ModuleStatics<T>::config, &ModuleStatics<T>::exit};

/**
 * Static definition of the info struct, which gets passed to DV.
 * DV reads this and uses the information to call into the module.
 * It gets instantiated with the template parameter T, which has to
 * be a valid module and inherit from `dv::ModuleBase`.
 * @tparam T The user defined module. Must inherit from `dv::ModuleBase`
 */
template<class T>
const dvModuleInfoS ModuleStatics<T>::info = {1, T::initDescription(), sizeof(T), &functions};

} // namespace dv

#endif // DV_SDK_MODULE_HPP
