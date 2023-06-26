#ifndef DV_SDK_TRIGGER_HPP
#define DV_SDK_TRIGGER_HPP

#include "wrappers.hpp"

#include <dv-processing/data/trigger_base.hpp>

namespace dv {

template<>
class InputVectorDataWrapper<dv::TriggerPacket, dv::Trigger> :
	public _InputVectorDataWrapperCommon<dv::TriggerPacket, dv::Trigger> {
public:
	InputVectorDataWrapper(std::shared_ptr<const dv::TriggerPacket> p) :
		_InputVectorDataWrapperCommon<dv::TriggerPacket, dv::Trigger>(std::move(p)) {
	}
};

template<>
class OutputVectorDataWrapper<dv::TriggerPacket, dv::Trigger> :
	public _OutputVectorDataWrapperCommon<dv::TriggerPacket, dv::Trigger> {
public:
	OutputVectorDataWrapper(dv::TriggerPacket *p, dvModuleData m, const std::string &n) :
		_OutputVectorDataWrapperCommon<dv::TriggerPacket, dv::Trigger>(p, m, n) {
	}

	// Un-hide copy assignment.
	using _OutputDataWrapperCommon<dv::TriggerPacket>::operator=;
};

} // namespace dv

#endif // DV_SDK_TRIGGER_HPP
