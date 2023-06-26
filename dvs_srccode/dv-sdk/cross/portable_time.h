#ifndef DV_SDK_PORTABLE_TIME_H_
#define DV_SDK_PORTABLE_TIME_H_

#ifdef __cplusplus

#	include <cstdlib>
#	include <ctime>

#else

#	include <stdbool.h>
#	include <stdlib.h>
#	include <time.h>

#endif

#include "dv-sdk/api_visibility.h"

#ifdef __cplusplus
extern "C" {
#endif

	DVSDK_EXPORT bool portable_clock_gettime_monotonic(struct timespec *monoTime);
	DVSDK_EXPORT bool portable_clock_gettime_realtime(struct timespec *realTime);

#ifdef __cplusplus
}
#endif

#endif /* DV_SDK_PORTABLE_TIME_H_ */
