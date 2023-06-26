#ifndef DV_SDK_UTILS_H_
#define DV_SDK_UTILS_H_

// Suppress unused argument warnings, if needed
#define UNUSED_ARGUMENT(arg) (void) (arg)

// Common includes, useful for everyone.
#ifdef __cplusplus

#	include <cerrno>
#	include <cinttypes>
#	include <cstddef>
#	include <cstdint>
#	include <cstdlib>

#else

#	include <errno.h>
#	include <inttypes.h>
#	include <stdbool.h>
#	include <stddef.h>
#	include <stdint.h>
#	include <stdlib.h>

#endif

// Need to include windows.h first on Windows, as it will redefine
// ERORR, WARNING etc., which we then overwrite below with logLevel.
#if defined(_WIN32) || defined(_WIN64) || defined(__CYGWIN__)
#	define WIN32_LEAN_AND_MEAN
#	include <windows.h>
#endif

#include "config/dvConfig.h"

#ifdef __cplusplus
extern "C" {
#endif

	enum dvLogLevel {
		DVLOG_ERROR   = 3,
		DVLOG_WARNING = 4,
		DVLOG_INFO    = 6,
		DVLOG_DEBUG   = 7,
	};

	DVSDK_EXPORT void dvLog(enum dvLogLevel level, const char *message);

#ifdef __cplusplus
}

#	include "config/dvConfig.hpp"
#	include "version.hpp"

#	include <algorithm>
#	include <vector>
#	include <utility>
#	include <memory>
#	include <string>
#	include <stdexcept>
#	include <type_traits>
#	include <dv-processing/core/utils.hpp>

namespace dv {

// Undefine the log-level names, to avoid conflicts with macros
// on Windows/MinGW for example.
#	undef ERROR
#	undef WARNING
#	undef INFO
#	undef DEBUG

enum class logLevel {
	ERROR   = DVLOG_ERROR,
	WARNING = DVLOG_WARNING,
	INFO    = DVLOG_INFO,
	DEBUG   = DVLOG_DEBUG,
};

namespace Cfg  = dv::Config;
using CfgType  = dv::Config::AttributeType;
using CfgFlags = dv::Config::AttributeFlags;

template<typename T>
using unique_ptr_deleter = std::unique_ptr<T, void (*)(T *)>;

using unique_ptr_void = unique_ptr_deleter<void>;

template<typename T>
inline unique_ptr_void make_unique_void(T *ptr) {
	return unique_ptr_void(ptr, [](void *data) {
		T *p = static_cast<T *>(data);
		delete p;
	});
}

template<typename T>
inline std::shared_ptr<T> shared_ptr_wrap_extra_deleter(std::shared_ptr<T> in, std::function<void(T *)> action) {
	if (!in) {
		return {};
	}
	if (!action) {
		return in;
	}

	auto new_deleter = [action](std::shared_ptr<T> *p) {
		action(p->get());
		delete p;
	};

	auto tmp = std::shared_ptr<std::shared_ptr<T>>(new std::shared_ptr<T>(std::move(in)), std::move(new_deleter));

	if (!tmp) {
		return {};
	}

	// aliasing constructor:
	return {tmp, tmp.get()->get()};
}

inline void Log(logLevel level, const char *msg) {
	dvLog(static_cast<enum dvLogLevel>(level), msg);
}

inline void Log(logLevel level, const std::string &msg) {
	dvLog(static_cast<enum dvLogLevel>(level), msg.c_str());
}

#	if defined(FMT_VERSION) && FMT_VERSION >= 80000

template<typename... Args>
inline void Log(logLevel level, const fmt::format_string<Args...> format, Args &&...args) {
	dvLog(static_cast<enum dvLogLevel>(level), fmt::format(format, std::forward<Args>(args)...).c_str());
}

#	else

template<typename S, typename... Args>
inline void Log(logLevel level, const S &format, Args &&...args) {
	dvLog(static_cast<enum dvLogLevel>(level), fmt::format(format, std::forward<Args>(args)...).c_str());
}

#	endif

} // namespace dv

#endif

#endif /* DV_SDK_UTILS_H_ */
