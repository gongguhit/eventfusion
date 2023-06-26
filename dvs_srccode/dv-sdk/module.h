/*
 * Public header for support library.
 * Modules can use this and link to it.
 */

#ifndef DV_SDK_MODULE_H_
#define DV_SDK_MODULE_H_

#include "data/types.hpp"
#include "utils.h"

#ifdef __cplusplus
extern "C" {
#endif

	struct dvModuleDataS {
		dvConfigNode moduleNode;
		void *moduleState;
	};

	typedef struct dvModuleDataS *dvModuleData;

	struct dvModuleFunctionsS {
		bool (*const moduleStaticInit)(
			dvModuleData moduleData); // Can be NULL. ModuleState is always NULL, do not dereference/use.
		bool (*const moduleInit)(dvModuleData moduleData);   // Can be NULL.
		void (*const moduleRun)(dvModuleData moduleData);    // Must be defined.
		void (*const moduleConfig)(dvModuleData moduleData); // Can be NULL.
		void (*const moduleExit)(dvModuleData moduleData);   // Can be NULL.
	};

	typedef struct dvModuleFunctionsS const *dvModuleFunctions;

	struct dvModuleInfoS {
		// Module version (informative).
		int32_t version;
		// Module description (informative).
		const char *description;
		// Size in bytes of module state.
		size_t memSize;
		// Functions to execute to run module.
		dvModuleFunctions functions;
	};

	typedef struct dvModuleInfoS const *dvModuleInfo;

	/**
	 * Function to be implemented by modules.
	 * Must return a dvModuleInfoS structure pointer,
	 * with all the information from your module.
	 */
	DVSDK_EXPORT dvModuleInfo dvModuleGetInfo150(void);

	enum dvModuleHooks {
		DV_HOOK_DEVICE_DISCOVERY = 0,
	};

	/**
	 * Optional function to be implemented by modules.
	 * Must return a pointer to the right structure based
	 * on given enum value, or NULL if not supported.
	 */
	DVSDK_EXPORT void *dvModuleGetHooks(enum dvModuleHooks hook);

	// Functions available for use: module connectivity.
	DVSDK_EXPORT void dvModuleRegisterType(dvModuleData moduleData, const struct dvType type);
	DVSDK_EXPORT void dvModuleRegisterOutput(dvModuleData moduleData, const char *name, const char *typeName);
	DVSDK_EXPORT void dvModuleRegisterInput(
		dvModuleData moduleData, const char *name, const char *typeName, bool optional);

	DVSDK_EXPORT struct dvTypedObject *dvModuleOutputAllocate(dvModuleData moduleData, const char *name);
	DVSDK_EXPORT void dvModuleOutputCommit(dvModuleData moduleData, const char *name);

	DVSDK_EXPORT const struct dvTypedObject *dvModuleInputGet(dvModuleData moduleData, const char *name);
	DVSDK_EXPORT void dvModuleInputAdvance(dvModuleData moduleData, const char *name);
	DVSDK_EXPORT void dvModuleInputDismiss(dvModuleData moduleData, const char *name, const struct dvTypedObject *data);

	DVSDK_EXPORT dvConfigNode dvModuleOutputGetInfoNode(dvModuleData moduleData, const char *name);
	DVSDK_EXPORT dvConfigNodeConst dvModuleInputGetInfoNode(dvModuleData moduleData, const char *name);
	DVSDK_EXPORT bool dvModuleInputIsConnected(dvModuleData moduleData, const char *name);

#ifdef __cplusplus
}
#endif

#endif /* DV_SDK_MODULE_H_ */
