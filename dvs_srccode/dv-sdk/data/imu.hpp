#ifndef DV_SDK_IMU_HPP
#define DV_SDK_IMU_HPP

#include "wrappers.hpp"

#include <dv-processing/data/imu_base.hpp>

namespace dv {

template<>
class InputVectorDataWrapper<dv::IMUPacket, dv::IMU> : public _InputVectorDataWrapperCommon<dv::IMUPacket, dv::IMU> {
public:
	InputVectorDataWrapper(std::shared_ptr<const IMUPacket> p) :
		_InputVectorDataWrapperCommon<dv::IMUPacket, dv::IMU>(std::move(p)) {
	}
};

template<>
class OutputVectorDataWrapper<dv::IMUPacket, dv::IMU> : public _OutputVectorDataWrapperCommon<dv::IMUPacket, dv::IMU> {
public:
	OutputVectorDataWrapper(IMUPacket *p, dvModuleData m, const std::string &n) :
		_OutputVectorDataWrapperCommon<dv::IMUPacket, dv::IMU>(p, m, n) {
	}

	// Un-hide copy assignment.
	using _OutputDataWrapperCommon<dv::IMUPacket>::operator=;
};

} // namespace dv

#endif // DV_SDK_IMU_HPP
