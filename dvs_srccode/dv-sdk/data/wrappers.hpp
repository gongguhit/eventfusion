#ifndef DV_SDK_WRAPPERS_HPP
#define DV_SDK_WRAPPERS_HPP

#include "../module.h"
#include "../utils.h"
#include "cvector_proxy.hpp"

namespace dv {

struct commitType {};

constexpr commitType commit{};

/**
 * Common parts of the implementation of an input wrapper. All specific input data wrappers
 * for data types as well as the generic input data wrapper inherit from this.
 *
 * @tparam T The type of the input data
 */
template<typename T>
class _InputDataWrapperCommon {
protected:
	std::shared_ptr<const T> ptr;

	_InputDataWrapperCommon(std::shared_ptr<const T> p) : ptr(std::move(p)) {
	}

public:
	explicit operator bool() const noexcept {
		return (ptr.get() != nullptr);
	}

	std::shared_ptr<const T> getBasePointer() const noexcept {
		return (ptr);
	}

	operator std::shared_ptr<const T>() const noexcept {
		return (ptr);
	}
};

template<typename T, typename U>
class _InputVectorDataWrapperCommon : public _InputDataWrapperCommon<T>, public dv::cvectorConstProxy<U> {
protected:
	_InputVectorDataWrapperCommon(std::shared_ptr<const T> p) :
		_InputDataWrapperCommon<T>(p),
		dv::cvectorConstProxy<U>((p) ? (&p->elements) : (nullptr)) {
	}
};

/**
 * Generic case input data wrapper.
 * Used for all types that do not have a more specific input data wrapper.
 *
 * @tparam T The type of the input data
 */
template<typename T>
class InputDataWrapper : public _InputDataWrapperCommon<T> {
public:
	InputDataWrapper(std::shared_ptr<const T> p) : _InputDataWrapperCommon<T>(std::move(p)) {
	}

	const T &operator*() const noexcept {
		return (*(this->ptr.get()));
	}

	const T *operator->() const noexcept {
		return (this->ptr.get());
	}
};

/**
 * Generic case input data wrapper for vector-like data (must have a
 * dv::cvector<U> called 'elements').
 * Used for all vector types that do not have a more specific input data wrapper.
 *
 * @tparam T The type of the vector input data
 * @tparam U The type of the vector input data's elements
 */
template<typename T, typename U>
class InputVectorDataWrapper : public _InputVectorDataWrapperCommon<T, U> {
public:
	InputVectorDataWrapper(std::shared_ptr<const T> p) : _InputVectorDataWrapperCommon<T, U>(std::move(p)) {
	}

	const T &operator*() const noexcept {
		return (*(this->ptr.get()));
	}

	const T *operator->() const noexcept {
		return (this->ptr.get());
	}
};

/**
 * Common parts of the implementation of an output wrapper. All specific output data wrappers
 * for data types as well as the generic output data wrapper inherit from this.
 *
 * @tparam T The type of the output data
 */
template<typename T>
class _OutputDataWrapperCommon {
protected:
	T *ptr;
	dvModuleData moduleData;
	std::string name;

	_OutputDataWrapperCommon(T *p, dvModuleData m, const std::string &n) : ptr(p), moduleData(m), name(n) {
		// Low-level pointer for outputs cannot be null.
		if (p == nullptr) {
			throw std::runtime_error("OutputDataWrapper: null passed as data.");
		}
	}

public:
	_OutputDataWrapperCommon &operator=(const _InputDataWrapperCommon<T> &rhs) {
		if (!rhs) {
			throw std::runtime_error("Output: tried to copy-assign from empty input.");
		}

		// Copy assign content object.
		*ptr = *rhs.getBasePointer();
		return (*this);
	}

	explicit operator bool() const noexcept {
		return (ptr != nullptr);
	}

	T *getBasePointer() noexcept {
		return (ptr);
	}

	const T *getBasePointer() const noexcept {
		return (ptr);
	}

	void commit() {
		dvModuleOutputCommit(moduleData, name.c_str());

		// Update with next object, in case we continue to use this.
		auto typedObject = dvModuleOutputAllocate(moduleData, name.c_str());
		if (typedObject == nullptr) {
			ptr = nullptr;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wterminate"
			// Errors will write a log message and return null, due to being a
			// C function that cannot pass exceptions. Let's throw here and quit.
			throw std::bad_alloc();
#pragma GCC diagnostic pop
		}
		else {
			ptr = static_cast<T *>(typedObject->obj);
		}
	}

	_OutputDataWrapperCommon &operator<<(commitType) {
		commit();
		return *this;
	}
};

template<typename T, typename U>
class _OutputVectorDataWrapperCommon : public _OutputDataWrapperCommon<T>, public dv::cvectorProxy<U> {
protected:
	_OutputVectorDataWrapperCommon(T *p, dvModuleData m, const std::string &n) :
		_OutputDataWrapperCommon<T>(p, m, n),
		dv::cvectorProxy<U>(&p->elements) {
		// Low-level pointer for outputs cannot be null.
		if (p == nullptr) {
			throw std::runtime_error("OutputVectorDataWrapper: null passed as data.");
		}
	}

public:
	void commit() {
		// Ignore empty trigger packets.
		if (dv::cvectorProxy<U>::empty()) {
			return;
		}

		// commit the data
		_OutputDataWrapperCommon<T>::commit();

		// reassign the iterator
		dv::cvectorProxy<U>::reassign(&this->ptr->elements);
	}

	_OutputVectorDataWrapperCommon &operator<<(commitType) {
		commit();
		return *this;
	}

	_OutputVectorDataWrapperCommon &operator<<(const U &rhs) {
		dv::cvectorProxy<U>::push_back(rhs);
		return *this;
	}

	/**
	 * Appends all events the iterable container to the output buffer
	 * @param container The store with the events to be copied into the output buffer
	 * @return A reference to the output buffer
	 */
	template<typename K>
	_OutputVectorDataWrapperCommon &operator<<(const K &container) {
		for (const U &e : container) {
			dv::cvectorProxy<U>::push_back(e);
		}
		return *this;
	}
};

/**
 * Generic case output data wrapper.
 * Used for all types that do not have a more specific output data wrapper.
 *
 * @tparam T The type of the output data
 */
template<typename T>
class OutputDataWrapper : public _OutputDataWrapperCommon<T> {
public:
	OutputDataWrapper(T *p, dvModuleData m, const std::string &n) : _OutputDataWrapperCommon<T>(p, m, n) {
	}

	T &operator*() noexcept {
		return (*this->ptr);
	}

	const T &operator*() const noexcept {
		return (*this->ptr);
	}

	T *operator->() noexcept {
		return (this->ptr);
	}

	const T *operator->() const noexcept {
		return (this->ptr);
	}

	// Un-hide copy assignment.
	using _OutputDataWrapperCommon<T>::operator=;
};

/**
 * Generic case output vector data wrapper for vector-like data (must have a
 * dv::cvector<U> called 'elements').
 * Used for all vector types that do not have a more specific output data wrapper.
 *
 * @tparam T The type of the vector output data
 * @tparam U The type of the vector output data's elements
 */
template<typename T, typename U>
class OutputVectorDataWrapper : public _OutputVectorDataWrapperCommon<T, U> {
public:
	OutputVectorDataWrapper(T *p, dvModuleData m, const std::string &n) :
		_OutputVectorDataWrapperCommon<T, U>(p, m, n) {
	}

	T &operator*() noexcept {
		return (*this->ptr);
	}

	const T &operator*() const noexcept {
		return (*this->ptr);
	}

	T *operator->() noexcept {
		return (this->ptr);
	}

	const T *operator->() const noexcept {
		return (this->ptr);
	}

	// Un-hide copy assignment.
	using _OutputDataWrapperCommon<T>::operator=;
};

/**
 * Base class for a runtime input definition.
 * There are template-specialized subclasses of this, providing convenience function
 * interfaces for the most common, known types. There is also a generic,
 * templated subclass `RuntimeInput` which does not add any more convenience
 * functions over this common subclass, and can be used for the generic case
 */
template<typename T>
class _RuntimeInputCommon {
private:
	/* Runtime name of this module from config */
	std::string name_;
	/* Pointer to the dv moduleData struct */
	dvModuleData moduleData_;

protected:
	/**
	 * Fetches available data at the input and returns a shared_ptr to it.
	 * Also casts the shared_ptr to this particular input type.
	 * @return A shared_ptr of the input data type to the latest received data
	 */
	std::shared_ptr<const T> getUnwrapped() const {
		auto typedObject = dvModuleInputGet(moduleData_, name_.c_str());
		if (typedObject == nullptr) {
			// Actual errors will write a log message and return null.
			// No data just returns null, which is an expected case.
			// Just forward null, which will instantiate an empty shared_ptr.
			return (nullptr);
		}

		// Build shared_ptr with custom deleter first, so that in verification failure case
		// (debug mode), memory gets properly cleaned up.
		std::shared_ptr<const T> objPtr{
			static_cast<const T *>(typedObject->obj), [moduleData = moduleData_, name = name_, typedObject](const T *) {
				dvModuleInputDismiss(moduleData, name.c_str(), typedObject);
			}};

		if constexpr (dv::DEBUG_ENABLED) {
			if (typedObject->typeId != dvTypeIdentifierToId(T::TableType::identifier)) {
				throw std::runtime_error(
					"getUnwrapped(" + name_ + "): input type and given template type are not compatible.");
			}
		}

		return (objPtr);
	}

	/**
	 * This constructor is called by the child classes in their initialization
	 * @param name The name of this input
	 * @param moduleData Pointer to the dv moduleData struct
	 */
	_RuntimeInputCommon(const std::string &name, dvModuleData moduleData) : name_(name), moduleData_(moduleData) {
		if (!dv::Config::Node(moduleData->moduleNode).existsRelativeNode("inputs/" + name + "/")) {
			throw std::invalid_argument("Invalid input name '" + name + "'.");
		}
	}

public:
	/**
	 * Get data from an input
	 *
	 * @return An input wrapper of the desired type, allowing data access
	 */
	const InputDataWrapper<T> data() const {
		const InputDataWrapper<T> wrapper{getUnwrapped()};
		return (wrapper);
	}

	/**
	 * Returns true, if this optional input is actually connected to an output of another module
	 * @return true, if this input is connected
	 */
	bool isConnected() const {
		return (dvModuleInputIsConnected(moduleData_, name_.c_str()));
	}

	/**
	 * Returns an info node about the specified input. Can be used to determine dimensions of an
	 * input/output
	 * @return A node that contains the specified inputs information, such as "sizeX" or "sizeY"
	 */
	const dv::Config::Node infoNode() const {
		if (!isConnected()) {
			throw std::out_of_range("Unconnected input '" + name_ + "': cannot get info node.");
		}

		// const_cast and then re-add const manually. Needed for transition to C++ type.
		return (const_cast<dvConfigNode>(dvModuleInputGetInfoNode(moduleData_, name_.c_str())));
	}

	/**
	 * Returns the description of the origin of the data
	 * @return the description of the origin of the data
	 */
	const std::string getOriginDescription() const {
		return (infoNode().getString("source"));
	}
};

template<typename T, typename U>
class _RuntimeVectorInputCommon : public _RuntimeInputCommon<T> {
protected:
	_RuntimeVectorInputCommon(const std::string &name, dvModuleData moduleData) :
		_RuntimeInputCommon<T>(name, moduleData) {
	}

public:
	/**
	 * Get data from a vector input
	 *
	 * @return An input wrapper of the desired type, allowing data access
	 */
	const InputVectorDataWrapper<T, U> data() const {
		const InputVectorDataWrapper<T, U> wrapper{_RuntimeInputCommon<T>::getUnwrapped()};
		return (wrapper);
	}
};

/**
 * Describes a generic input at runtime. A generic input can be instantiated for any type.
 * This class basically just inherits from `_RuntimeInputCommon<T>` and does not add any
 * specializations.
 * @tparam T The type of the input data
 */
template<typename T>
class RuntimeInput : public _RuntimeInputCommon<T> {
public:
	RuntimeInput(const std::string &name, dvModuleData moduleData) : _RuntimeInputCommon<T>(name, moduleData) {
	}
};

/**
 * Describes a generic vector input at runtime. A generic vecotr input can be instantiated for
 * any vector type (a type with a member called 'elements' of type dv::cvector<U>).
 * This class basically just inherits from `_RuntimeInputCommon<T>` and does not add any
 * specializations.
 *
 * @tparam T The type of the vector input data
 * @tparam U The type of the vector input data's elements
 */
template<typename T, typename U>
class RuntimeVectorInput : public _RuntimeVectorInputCommon<T, U> {
public:
	RuntimeVectorInput(const std::string &name, dvModuleData moduleData) :
		_RuntimeVectorInputCommon<T, U>(name, moduleData) {
	}
};

/**
 * Base class for a runtime output. This class acts as the base for various template-specialized
 * sub classes which provide convenience functions for outputting data in their respective
 * data types. There is a templated generic subclass `RuntimeOutput<T>` that can be used
 * for the generic case
 * @tparam T The flatbuffers type of the output data
 */
template<typename T>
class _RuntimeOutputCommon {
protected:
	/* Configured name of the module at runtime */
	std::string name_;
	/* pointer to the dv moduleData struct at runtime */
	dvModuleData moduleData_;

	/**
	 * Allocates a new instance of the datatype of this output and returns a
	 * raw pointer to the allocated memory. If there was memory allocated before
	 * (This function has been called before) but the output never has been commited,
	 * a raw pointer to the previously allocated memory gets returned.
	 * @return A raw pointer to the allocated memory
	 */
	T *allocateUnwrapped() {
		auto typedObject = dvModuleOutputAllocate(moduleData_, name_.c_str());
		if (typedObject == nullptr) {
			// Errors will write a log message and return null, due to being a
			// C function that cannot pass exceptions. Let's throw here and quit.
			throw std::bad_alloc();
		}

		if constexpr (dv::DEBUG_ENABLED) {
			if (typedObject->typeId != dvTypeIdentifierToId(T::TableType::identifier)) {
				throw std::runtime_error(
					"allocateUnwrapped(" + name_ + "): output type and given template type are not compatible.");
			}
		}

		return (static_cast<T *>(typedObject->obj));
	}

	/**
	 * Creates the output information attribute in the config tree.
	 * The source attribute is a string containing information about the original generator of the data
	 * @param originDescription a string containing information about the original generator of the data
	 */
	void createSourceAttribute(const std::string &originDescription) {
		dv::Config::Node iNode = infoNode();

		iNode.create<dv::Config::AttributeType::STRING>("source", originDescription, {0, 8192},
			dv::Config::AttributeFlags::READ_ONLY | dv::Config::AttributeFlags::NO_EXPORT,
			"Description of the first origin of the data");
	}

	/**
	 * Adds size information attributes to the output info node
	 * @param sizeX The width dimension of the output
	 * @param sizeY The height dimension of the output
	 */
	void createSizeAttributes(int sizeX, int sizeY) {
		dv::Config::Node iNode = infoNode();

		iNode.create<dv::Config::AttributeType::INT>("sizeX", sizeX, {sizeX, sizeX},
			dv::Config::AttributeFlags::READ_ONLY | dv::Config::AttributeFlags::NO_EXPORT,
			"Width of the output data. (max x-coordinate + 1)");

		iNode.create<dv::Config::AttributeType::INT>("sizeY", sizeY, {sizeY, sizeY},
			dv::Config::AttributeFlags::READ_ONLY | dv::Config::AttributeFlags::NO_EXPORT,
			"Height of the output data. (max y-coordinate + 1)");
	}

	/**
	 * This constructor is called by the subclasses constructors
	 * @param name The configuration name of the module this output belongs to
	 * @param moduleData A pointer to the dv moduleData struct
	 */
	_RuntimeOutputCommon(const std::string &name, dvModuleData moduleData) : name_(name), moduleData_(moduleData) {
		if (!dv::Config::Node(moduleData->moduleNode).existsRelativeNode("outputs/" + name + "/")) {
			throw std::invalid_argument("Invalid output name '" + name + "'.");
		}
	}

public:
	/**
	 * Sets up the output. Has to be called in the constructor of the module.
	 * @param originDescription A description of the original creator of the data
	 */
	void setup(const std::string &originDescription) {
		createSourceAttribute(originDescription);
	}

	/**
	 * Sets this output up with the same parameters as the supplied input.
	 * @param input An input to copy the information from
	 */
	template<typename U>
	void setup(const RuntimeInput<U> &input) {
		input.infoNode().copyTo(infoNode());

		// Check that required attributes exist at least.
		getOriginDescription();
	}

	/**
	 * Sets this event output up with the same parameters as the supplied vector input.
	 * @param input A vector input to copy the information from
	 */
	template<typename U, typename TT>
	void setup(const RuntimeVectorInput<U, TT> &input) {
		input.infoNode().copyTo(infoNode());

		// Check that required attributes exist at least.
		getOriginDescription();
	}

	/**
	 * Returns a writeable output wrapper for the given type of this output.
	 * Allocates new output memory if necessary. The output can be committed
	 * by calling commit on the returned object.
	 * @return A wrapper to allocated output memory to write to
	 */
	OutputDataWrapper<T> data() {
		OutputDataWrapper<T> wrapper{allocateUnwrapped(), moduleData_, name_};
		return (wrapper);
	}

	/**
	 * Convenience shortcut to commit the data on this output directly
	 */
	void operator<<(commitType) {
		data() << dv::commit;
	}

	/**
	 * Returns an info node about the specified output, can be used to set output information.
	 * @return A node that can contain output information, such as "sizeX" or "sizeY"
	 */
	dv::Config::Node infoNode() {
		return (dvModuleOutputGetInfoNode(moduleData_, name_.c_str()));
	}

	/**
	 * Returns an info node about the specified output, can be used to set output information.
	 * @return A node that can contain output information, such as "sizeX" or "sizeY"
	 */
	const dv::Config::Node infoNode() const {
		return (dvModuleOutputGetInfoNode(moduleData_, name_.c_str()));
	}

	/**
	 * Returns the description of the origin of the data
	 * @return the description of the origin of the data
	 */
	const std::string getOriginDescription() const {
		return (infoNode().getString("source"));
	}
};

template<typename T, typename U>
class _RuntimeVectorOutputCommon : public _RuntimeOutputCommon<T> {
protected:
	_RuntimeVectorOutputCommon(const std::string &name, dvModuleData moduleData) :
		_RuntimeOutputCommon<T>(name, moduleData) {
	}

public:
	/**
	 * Returns a writeable output wrapper for the given type of this output.
	 * Allocates new output memory if necessary. The output can be committed
	 * by calling commit on the returned object.
	 * @return A wrapper to allocated output memory to write to
	 */
	OutputVectorDataWrapper<T, U> data() {
		OutputVectorDataWrapper<T, U> wrapper{
			_RuntimeOutputCommon<T>::allocateUnwrapped(), this->moduleData_, this->name_};
		return (wrapper);
	}

	/**
	 * Convenience shortcut to commit the data on this output directly
	 * This gets resolved to one of the various << functions
	 * of the data wrapper.
	 */
	template<typename K>
	_RuntimeVectorOutputCommon &operator<<(const K &rhs) {
		data() << rhs;
		return *this;
	}

	/**
	 * Convenience shortcut to commit the data on this output directly
	 */
	void operator<<(commitType) {
		data() << dv::commit;
	}
};

/**
 * Class that describes an output of a generic type at runtime.
 * Can be used to obtain information about the output, as well as getting a new
 * output object to send data to.
 *
 * @tparam T The type of the output data
 */
template<typename T>
class RuntimeOutput : public _RuntimeOutputCommon<T> {
public:
	RuntimeOutput(const std::string &name, dvModuleData moduleData) : _RuntimeOutputCommon<T>(name, moduleData) {
	}
};

/**
 * Class that describes an output of a generic vector type at runtime.
 * A generic vector input can be instantiated for any vector type (a type
 * with a member called 'elements' of type dv::cvector<U>).
 * Can be used to obtain information about the output, as well as getting a new
 * output object to send data to.
 *
 * @tparam T The type of the vector output data
 * @tparam U The type of the vector output data's elements
 */
template<typename T, typename U>
class RuntimeVectorOutput : public _RuntimeVectorOutputCommon<T, U> {
public:
	RuntimeVectorOutput(const std::string &name, dvModuleData moduleData) :
		_RuntimeVectorOutputCommon<T, U>(name, moduleData) {
	}
};

} // namespace dv

#endif // DV_SDK_WRAPPERS_HPP
