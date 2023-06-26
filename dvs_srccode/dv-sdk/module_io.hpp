#ifndef DV_SDK_MODULE_IO_HPP
#define DV_SDK_MODULE_IO_HPP

#include "data/bounding_box.hpp"
#include "data/event.hpp"
#include "data/frame.hpp"
#include "data/imu.hpp"
#include "data/trigger.hpp"
#include "data/wrappers.hpp"
#include "module.h"
#include "utils.h"

#include <opencv2/core.hpp>

namespace dv {

/**
 * Definition of a module input. Every input to a module has a unique (for the module) name,
 * as well as a type. The type is the 4-character flatbuffers identifier
 */
class InputDefinition {
public:
	std::string name;
	std::string typeName;
	bool optional;

	InputDefinition(const std::string &n, const std::string &t, bool opt = false) :
		name(n),
		typeName(t),
		optional(opt) {
	}
};

/**
 * Definition of a module output. Every output has a unique (for the module) name,
 * as well as a type. THe type is the 4-character flatbuffers identifier.
 */
class OutputDefinition {
public:
	std::string name;
	std::string typeName;

	OutputDefinition(const std::string &n, const std::string &t) : name(n), typeName(t) {
	}
};

/**
 * Vector decorator that gives convenience methods to add various kinds of inputs to
 * a module
 */
class InputDefinitionList {
private:
	std::vector<InputDefinition> inputs;

public:
	/**
	 * Adds an input of a generic type to this module.
	 *
	 * @param name The name of this input
	 * @param typeIdentifier The identifier of the FlatBuffers data type used for this input
	 * @param optional A flag that describes if this input is optional or not. Optional
	 *                 inputs are not required to be connected for successful module startup.
	 */
	void addInput(const std::string &name, const std::string &typeIdentifier, bool optional = false) {
		inputs.emplace_back(name, typeIdentifier, optional);
	}

	/**
	 * Adds an event data input to this module.
	 *
	 * @param name The name of the event data input
	 * @param optional A flag to set this input as optional
	 */
	void addEventInput(const std::string &name, bool optional = false) {
		addInput(name, dv::EventPacket::TableType::identifier, optional);
	}

	/**
	 * Adds a frame input to this module.
	 *
	 * @param name The name of the frame input
	 * @param optional A flag to set this input as optional
	 */
	void addFrameInput(const std::string &name, bool optional = false) {
		addInput(name, dv::Frame::TableType::identifier, optional);
	}

	/**
	 * Adds an IMU input to this module.
	 *
	 * @param name The name of the IMU input
	 * @param optional A flag to set this input as optional
	 */
	void addIMUInput(const std::string &name, bool optional = false) {
		addInput(name, dv::IMUPacket::TableType::identifier, optional);
	}

	/**
	 * Adds a trigger input to this module.
	 *
	 * @param name The name of the trigger input
	 * @param optional A flag to set this input as optional
	 */
	void addTriggerInput(const std::string &name, bool optional = false) {
		addInput(name, dv::TriggerPacket::TableType::identifier, optional);
	}

	/**
	 * Adds a bounding box input to this module.
	 *
	 * @param name The name of the bounding box input
	 * @param optional A flag to set this input as optional
	 */
	void addBoundingBoxInput(const std::string &name, bool optional = false) {
		addInput(name, dv::BoundingBoxPacket::TableType::identifier, optional);
	}

	/**
	 * __INTERNAL USE__
	 * Returns the list of configured input definitions
	 * @return The list of configured input definitions
	 */
	const std::vector<InputDefinition> &getInputs() const {
		return (inputs);
	}
};

/**
 * Vector decorator that exposes convenience functions to add various types of outputs to
 * a module
 */
class OutputDefinitionList {
private:
	std::vector<OutputDefinition> outputs;

public:
	/**
	 * Adds an output of a generic type to this module.
	 *
	 * @param name The name of this output
	 * @param typeIdentifier The identifier of the FlatBuffers data type used for this input
	 */
	void addOutput(const std::string &name, const std::string &typeIdentifier) {
		outputs.emplace_back(name, typeIdentifier);
	}

	/**
	 * Adds an event output to this module.
	 *
	 * @param name The name of the event output
	 */
	void addEventOutput(const std::string &name) {
		addOutput(name, dv::EventPacket::TableType::identifier);
	}

	/**
	 * Adds a frame output to this module.
	 *
	 * @param name The name of the frame output
	 */
	void addFrameOutput(const std::string &name) {
		addOutput(name, dv::Frame::TableType::identifier);
	}

	/**
	 * Adds an IMU output to this module.
	 *
	 * @param name The name of the IMU output
	 */
	void addIMUOutput(const std::string &name) {
		addOutput(name, dv::IMUPacket::TableType::identifier);
	}

	/**
	 * Adds a trigger output to this module.
	 *
	 * @param name the name of the trigger output
	 */
	void addTriggerOutput(const std::string &name) {
		addOutput(name, dv::TriggerPacket::TableType::identifier);
	}

	/**
	 * Adds a bounding box output to this module.
	 *
	 * @param name the name of the bounding box output
	 */
	void addBoundingBoxOutput(const std::string &name) {
		addOutput(name, dv::BoundingBoxPacket::TableType::identifier);
	}

	/**
	 * __INTERNAL USE__
	 * Returns the list of configured outputs
	 * @return the list of configured outputs
	 */
	const std::vector<OutputDefinition> &getOutputs() const {
		return (outputs);
	}
};

class RuntimeInputs {
private:
	dvModuleData moduleData;

public:
	RuntimeInputs(dvModuleData m) : moduleData(m) {
	}

	/**
	 * Returns the information about the input with the specified name.
	 * The type of the input has to be specified as well.
	 * @tparam T The type of the input
	 * @param name The name of the input
	 * @return An object to access the information about the input
	 */
	template<typename T>
	const RuntimeInput<T> getInput(const std::string &name) const {
		return RuntimeInput<T>(name, moduleData);
	}

	/**
	 * Returns the information about the vector input with the specified name.
	 * The type of the vector input has to be specified as well.
	 *
	 * @tparam T The type of the vector input
	 * @tparam U The type of the vector input's elements
	 *
	 * @param name The name of the input
	 * @return An object to access the information about the input
	 */
	template<typename T, typename U>
	const RuntimeVectorInput<T, U> getVectorInput(const std::string &name) const {
		return RuntimeVectorInput<T, U>(name, moduleData);
	}

	/**
	 * (Convenience) Function to get an event input
	 * @param name the name of the event input stream
	 * @return An object to access information about the input stream
	 */
	const RuntimeVectorInput<dv::EventPacket, dv::Event> getEventInput(const std::string &name) const {
		return getVectorInput<dv::EventPacket, dv::Event>(name);
	}

	/**
	 * (Convenience) Function to get an frame input
	 * @param name the name of the frame input stream
	 * @return An object to access information about the input stream
	 */
	const RuntimeInput<dv::Frame> getFrameInput(const std::string &name) const {
		return getInput<dv::Frame>(name);
	}

	/**
	 * (Convenience) Function to get an IMU input
	 * @param name the name of the IMU input stream
	 * @return An object to access information about the input stream
	 */
	const RuntimeVectorInput<dv::IMUPacket, dv::IMU> getIMUInput(const std::string &name) const {
		return getVectorInput<dv::IMUPacket, dv::IMU>(name);
	}

	/**
	 * (Convenience) Function to get an trigger input
	 * @param name the name of the trigger input stream
	 * @return An object to access information about the input stream
	 */
	const RuntimeVectorInput<dv::TriggerPacket, dv::Trigger> getTriggerInput(const std::string &name) const {
		return getVectorInput<dv::TriggerPacket, dv::Trigger>(name);
	}

	/**
	 * (Convenience) Function to get an boundingBox input
	 * @param name the name of the boundingBox input stream
	 * @return An object to access information about the input stream
	 */
	const RuntimeVectorInput<dv::BoundingBoxPacket, dv::BoundingBox> getBoundingBoxInput(
		const std::string &name) const {
		return getVectorInput<dv::BoundingBoxPacket, dv::BoundingBox>(name);
	}

	/**
	 * Returns an info node about the specified input. Can be used to determine dimensions of an
	 * input/output
	 * @return A node that contains the specified inputs information, such as "sizeX" or "sizeY"
	 */
	const dv::Config::Node infoNode(const std::string &name) const {
		if (!isConnected(name)) {
			throw std::out_of_range("Unconnected input '" + name + "': cannot get info node.");
		}

		// const_cast and then re-add const manually. Needed for transition to C++ type.
		return (const_cast<dvConfigNode>(dvModuleInputGetInfoNode(moduleData, name.c_str())));
	}

	/**
	 * Returns true, if this optional input is actually connected to an output of another module
	 * @return true, if this input is connected
	 */
	bool isConnected(const std::string &name) const {
		if (!dv::Config::Node(moduleData->moduleNode).existsRelativeNode("inputs/" + name + "/")) {
			throw std::invalid_argument("Invalid input name '" + name + "'.");
		}

		return (dvModuleInputIsConnected(moduleData, name.c_str()));
	}
};

class RuntimeOutputs {
private:
	dvModuleData moduleData;

public:
	RuntimeOutputs(dvModuleData m) : moduleData(m) {
	}

	/**
	 * Function to get an output
	 * @param name the name of the output stream
	 * @return An object to access the modules output
	 */
	template<typename T>
	RuntimeOutput<T> getOutput(const std::string &name) {
		return RuntimeOutput<T>(name, moduleData);
	}

	/**
	 * Function to get a vector output
	 * @param name the name of the output stream
	 * @return An object to access the modules output
	 */
	template<typename T, typename U>
	RuntimeVectorOutput<T, U> getVectorOutput(const std::string &name) {
		return RuntimeVectorOutput<T, U>(name, moduleData);
	}

	/**
	 * (Convenience) Function to get an event output
	 * @param name the name of the event output stream
	 * @return An object to access the modules output
	 */
	RuntimeVectorOutput<dv::EventPacket, dv::Event> getEventOutput(const std::string &name) {
		return getVectorOutput<dv::EventPacket, dv::Event>(name);
	}

	/**
	 * (Convenience) Function to get an frame output
	 * @param name the name of the frame output stream
	 * @return An object to access the modules output
	 */
	RuntimeOutput<dv::Frame> getFrameOutput(const std::string &name) {
		return getOutput<dv::Frame>(name);
	}

	/**
	 * (Convenience) Function to get an imu output
	 * @param name the name of the imu output stream
	 * @return An object to access the modules output
	 */
	RuntimeVectorOutput<dv::IMUPacket, dv::IMU> getIMUOutput(const std::string &name) {
		return getVectorOutput<dv::IMUPacket, dv::IMU>(name);
	}

	/**
	 * (Convenience) Function to get an trigger output
	 * @param name the name of the trigger output stream
	 * @return An object to access the modules output
	 */
	RuntimeVectorOutput<dv::TriggerPacket, dv::Trigger> getTriggerOutput(const std::string &name) {
		return getVectorOutput<dv::TriggerPacket, dv::Trigger>(name);
	}

	/**
	 * (Convenience) Function to get a bounding box output
	 * @param name the name of the trigger output stream
	 * @return An object to access the modules output
	 */
	RuntimeVectorOutput<dv::BoundingBoxPacket, dv::BoundingBox> getBoundingBoxOutput(const std::string &name) {
		return getVectorOutput<dv::BoundingBoxPacket, dv::BoundingBox>(name);
	}

	/**
	 * Returns an info node about the specified output, can be used toset output information.
	 * @return A node that can contain output information, such as "sizeX" or "sizeY"
	 */
	dv::Config::Node infoNode(const std::string &name) {
		if (!dv::Config::Node(moduleData->moduleNode).existsRelativeNode("outputs/" + name + "/")) {
			throw std::invalid_argument("Invalid output name '" + name + "'.");
		}

		return (dvModuleOutputGetInfoNode(moduleData, name.c_str()));
	}
};

} // namespace dv

#endif // DV_SDK_MODULE_IO_HPP
