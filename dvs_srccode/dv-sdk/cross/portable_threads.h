#ifndef DV_SDK_PORTABLE_THREADS_H_
#define DV_SDK_PORTABLE_THREADS_H_

#ifdef __cplusplus

#	include <cstdlib>

#else

#	include <stdbool.h>
#	include <stdlib.h>

#endif

#include "dv-sdk/api_visibility.h"

#ifdef __cplusplus
extern "C" {
#endif

	DVSDK_EXPORT bool portable_thread_set_name(const char *name);
	DVSDK_EXPORT bool portable_thread_set_priority_highest(void);

#ifdef __cplusplus
}
#endif

#endif /* DV_SDK_PORTABLE_THREADS_H_ */
