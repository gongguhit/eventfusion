#ifndef DV_SDK_PORTABLE_IO_H_
#define DV_SDK_PORTABLE_IO_H_

#ifdef __cplusplus

#	include <cstdlib>

#else

#	include <stdlib.h>

#endif

#include "dv-sdk/api_visibility.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Make PATH_MAX available by including the right headers.
 * This makes it easier to work cross-platform, especially
 * on MacOS X or Linux where this is in a different file.
 */
#include <limits.h>

#if defined(__linux__)
#	include <linux/limits.h>
#endif

#if defined(__APPLE__)
#	include <sys/syslimits.h>
#endif

#ifndef PATH_MAX
#	error "No PATH_MAX defined."
#endif

	/**
	 * Get the user's home directory path as a string.
	 * Returned string is a dynamically allocated copy,
	 * always remember to free() it to avoid a memory leak.
	 *
	 * @return string containing user home directory path.
	 *         Always remember to free() this!
	 */
	DVSDK_EXPORT char *portable_get_user_home_directory(void);

	/**
	 * Get the current executable's location as a string.
	 * Returned string is a dynamically allocated copy,
	 * always remember to free() it to avoid a memory leak.
	 *
	 * @return string containing current executable path.
	 *         Always remember to free() this!
	 */
	DVSDK_EXPORT char *portable_get_executable_location(void);

	/**
	 * Get the user name of the user that's currently
	 * executing the program.
	 *
	 * @return string containing user name.
	 *         Always remember to free() this!
	 */
	DVSDK_EXPORT char *portable_get_user_name(void);

#ifdef __cplusplus
}

#	include <string>
#	include <stdexcept>

namespace dv {

inline std::string portable_get_user_home_directory() {
	const auto homeDir = ::portable_get_user_home_directory();
	if (homeDir == nullptr) {
		throw std::runtime_error("No home or temporary directory found on this system.");
	}

	const std::string outputDir{homeDir};

	free(homeDir);

	return (outputDir);
}

inline std::string portable_get_executable_location() {
	const auto execLocation = ::portable_get_executable_location();
	if (execLocation == nullptr) {
		throw std::runtime_error("Could not determine executable location.");
	}

	const std::string outputLocation{execLocation};

	free(execLocation);

	return (outputLocation);
}

inline std::string portable_get_user_name() {
	const auto userName = ::portable_get_user_name();
	if (userName == nullptr) {
		throw std::runtime_error("Could not determine user name.");
	}

	const std::string outputUser{userName};

	free(userName);

	return (outputUser);
}

} // namespace dv

#endif

#endif /* DV_SDK_PORTABLE_IO_H_ */
