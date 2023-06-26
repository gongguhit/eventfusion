#ifndef DVCONFIG_H_
#define DVCONFIG_H_

#ifdef __cplusplus

#	include <cerrno>
#	include <cinttypes>
#	include <cstdint>
#	include <cstdlib>

#else

#	include <errno.h>
#	include <inttypes.h>
#	include <stdbool.h>
#	include <stdint.h>
#	include <stdlib.h>

#endif

#include "dv-sdk/api_visibility.h"

#ifdef __cplusplus
extern "C" {
#endif

	// dv::Config Node
	typedef struct dv_config_node *dvConfigNode;
	typedef const struct dv_config_node *dvConfigNodeConst;

	enum dvConfigAttributeType {
		DVCFG_TYPE_UNKNOWN = -1,
		DVCFG_TYPE_BOOL    = 0,
		DVCFG_TYPE_INT     = 1,
		DVCFG_TYPE_LONG    = 2,
		DVCFG_TYPE_FLOAT   = 3,
		DVCFG_TYPE_DOUBLE  = 4,
		DVCFG_TYPE_STRING  = 5,
	};

	union dvConfigAttributeValue {
		bool boolean;
		int32_t iint;
		int64_t ilong;
		float ffloat;
		double ddouble;
		char *string;
	};

	union dvConfigAttributeRange {
		int32_t intRange;
		int64_t longRange;
		float floatRange;
		double doubleRange;
		int32_t stringRange;
	};

	struct dvConfigAttributeRanges {
		union dvConfigAttributeRange min;
		union dvConfigAttributeRange max;
	};

	enum dvConfigAttributeFlags {
		DVCFG_FLAGS_NORMAL    = 0,
		DVCFG_FLAGS_READ_ONLY = 1,
		DVCFG_FLAGS_NO_EXPORT = 2,
		DVCFG_FLAGS_IMPORTED  = 4,
	};

	enum dvConfigNodeEvents {
		DVCFG_NODE_CHILD_ADDED   = 0,
		DVCFG_NODE_CHILD_REMOVED = 1,
	};

	enum dvConfigAttributeEvents {
		DVCFG_ATTRIBUTE_ADDED           = 0,
		DVCFG_ATTRIBUTE_MODIFIED        = 1,
		DVCFG_ATTRIBUTE_REMOVED         = 2,
		DVCFG_ATTRIBUTE_MODIFIED_CREATE = 3,
	};

	typedef void (*dvConfigNodeChangeListener)(
		dvConfigNode node, void *userData, enum dvConfigNodeEvents event, const char *changeNode);

	typedef void (*dvConfigAttributeChangeListener)(dvConfigNode node, void *userData,
		enum dvConfigAttributeEvents event, const char *changeKey, enum dvConfigAttributeType changeType,
		union dvConfigAttributeValue changeValue);

	DVSDK_EXPORT const char *dvConfigNodeGetName(dvConfigNodeConst node);
	DVSDK_EXPORT const char *dvConfigNodeGetPath(dvConfigNodeConst node);

	/**
	 * This returns a reference to a node, and as such must be carefully mediated with
	 * any dvConfigNodeRemoveNode() calls.
	 */
	DVSDK_EXPORT dvConfigNode dvConfigNodeGetParent(dvConfigNodeConst node);
	/**
	 * Remember to free the resulting array. This returns references to nodes,
	 * and as such must be carefully mediated with any dvConfigNodeRemoveNode() calls.
	 */
	DVSDK_EXPORT dvConfigNode *dvConfigNodeGetChildren(
		dvConfigNodeConst node, size_t *numChildren); // Walk all children.

	DVSDK_EXPORT void dvConfigNodeAddNodeListener(
		dvConfigNode node, void *userData, dvConfigNodeChangeListener node_changed);
	DVSDK_EXPORT void dvConfigNodeRemoveNodeListener(
		dvConfigNode node, void *userData, dvConfigNodeChangeListener node_changed);
	DVSDK_EXPORT void dvConfigNodeRemoveAllNodeListeners(dvConfigNode node);

	DVSDK_EXPORT void dvConfigNodeAddAttributeListener(
		dvConfigNode node, void *userData, dvConfigAttributeChangeListener attribute_changed);
	DVSDK_EXPORT void dvConfigNodeRemoveAttributeListener(
		dvConfigNode node, void *userData, dvConfigAttributeChangeListener attribute_changed);
	DVSDK_EXPORT void dvConfigNodeRemoveAllAttributeListeners(dvConfigNode node);

	/**
	 * Careful, only use if no references exist to this node and all its children.
	 * References are created by dvConfigTreeGetNode(), dvConfigNodeGetRelativeNode(),
	 * dvConfigNodeGetParent() and dvConfigNodeGetChildren().
	 */
	DVSDK_EXPORT void dvConfigNodeRemoveNode(dvConfigNode node);
	/**
	 * Careful, only use if no references exist to this node's children.
	 * References are created by dvConfigTreeGetNode(), dvConfigNodeGetRelativeNode(),
	 * dvConfigNodeGetParent() and dvConfigNodeGetChildren().
	 */
	DVSDK_EXPORT void dvConfigNodeRemoveSubTree(dvConfigNode node);
	DVSDK_EXPORT void dvConfigNodeClearSubTree(dvConfigNode startNode, bool clearStartNode);

	DVSDK_EXPORT void dvConfigNodeCopy(dvConfigNodeConst source, dvConfigNode destination);

	DVSDK_EXPORT void dvConfigNodeCreateAttribute(dvConfigNode node, const char *key, enum dvConfigAttributeType type,
		union dvConfigAttributeValue defaultValue, const struct dvConfigAttributeRanges ranges, int flags,
		const char *description);
	DVSDK_EXPORT void dvConfigNodeRemoveAttribute(dvConfigNode node, const char *key, enum dvConfigAttributeType type);
	DVSDK_EXPORT void dvConfigNodeRemoveAllAttributes(dvConfigNode node);
	DVSDK_EXPORT bool dvConfigNodeExistsAttribute(
		dvConfigNodeConst node, const char *key, enum dvConfigAttributeType type);
	DVSDK_EXPORT bool dvConfigNodePutAttribute(
		dvConfigNode node, const char *key, enum dvConfigAttributeType type, union dvConfigAttributeValue value);
	DVSDK_EXPORT union dvConfigAttributeValue dvConfigNodeGetAttribute(
		dvConfigNodeConst node, const char *key, enum dvConfigAttributeType type);
	DVSDK_EXPORT bool dvConfigNodeUpdateReadOnlyAttribute(
		dvConfigNode node, const char *key, enum dvConfigAttributeType type, union dvConfigAttributeValue value);

	DVSDK_EXPORT void dvConfigNodeCreateBool(
		dvConfigNode node, const char *key, bool defaultValue, int flags, const char *description);
	DVSDK_EXPORT bool dvConfigNodePutBool(dvConfigNode node, const char *key, bool value);
	DVSDK_EXPORT bool dvConfigNodeGetBool(dvConfigNodeConst node, const char *key);
	DVSDK_EXPORT void dvConfigNodeCreateInt(dvConfigNode node, const char *key, int32_t defaultValue, int32_t minValue,
		int32_t maxValue, int flags, const char *description);
	DVSDK_EXPORT bool dvConfigNodePutInt(dvConfigNode node, const char *key, int32_t value);
	DVSDK_EXPORT int32_t dvConfigNodeGetInt(dvConfigNodeConst node, const char *key);
	DVSDK_EXPORT void dvConfigNodeCreateLong(dvConfigNode node, const char *key, int64_t defaultValue, int64_t minValue,
		int64_t maxValue, int flags, const char *description);
	DVSDK_EXPORT bool dvConfigNodePutLong(dvConfigNode node, const char *key, int64_t value);
	DVSDK_EXPORT int64_t dvConfigNodeGetLong(dvConfigNodeConst node, const char *key);
	DVSDK_EXPORT void dvConfigNodeCreateFloat(dvConfigNode node, const char *key, float defaultValue, float minValue,
		float maxValue, int flags, const char *description);
	DVSDK_EXPORT bool dvConfigNodePutFloat(dvConfigNode node, const char *key, float value);
	DVSDK_EXPORT float dvConfigNodeGetFloat(dvConfigNodeConst node, const char *key);
	DVSDK_EXPORT void dvConfigNodeCreateDouble(dvConfigNode node, const char *key, double defaultValue, double minValue,
		double maxValue, int flags, const char *description);
	DVSDK_EXPORT bool dvConfigNodePutDouble(dvConfigNode node, const char *key, double value);
	DVSDK_EXPORT double dvConfigNodeGetDouble(dvConfigNodeConst node, const char *key);
	DVSDK_EXPORT void dvConfigNodeCreateString(dvConfigNode node, const char *key, const char *defaultValue,
		int32_t minLength, int32_t maxLength, int flags, const char *description);
	DVSDK_EXPORT bool dvConfigNodePutString(dvConfigNode node, const char *key, const char *value);
	DVSDK_EXPORT char *dvConfigNodeGetString(dvConfigNodeConst node, const char *key);

	DVSDK_EXPORT bool dvConfigNodeExportNodeToXML(dvConfigNodeConst node, const char *filePath, bool exportAll);
	DVSDK_EXPORT bool dvConfigNodeExportSubTreeToXML(dvConfigNodeConst node, const char *filePath, bool exportAll);
	DVSDK_EXPORT bool dvConfigNodeImportNodeFromXML(dvConfigNode node, const char *filePath, bool strict);
	DVSDK_EXPORT bool dvConfigNodeImportSubTreeFromXML(dvConfigNode node, const char *filePath, bool strict);

	DVSDK_EXPORT char *dvConfigNodeExportNodeToXMLString(dvConfigNodeConst node, bool exportAll);
	DVSDK_EXPORT char *dvConfigNodeExportSubTreeToXMLString(dvConfigNodeConst node, bool exportAll);
	DVSDK_EXPORT bool dvConfigNodeImportNodeFromXMLString(dvConfigNode node, const char *xmlStr, bool strict);
	DVSDK_EXPORT bool dvConfigNodeImportSubTreeFromXMLString(dvConfigNode node, const char *xmlStr, bool strict);

	DVSDK_EXPORT bool dvConfigNodeStringToAttributeConverter(
		dvConfigNode node, const char *key, const char *type, const char *value, bool overrideReadOnly);
	DVSDK_EXPORT const char **dvConfigNodeGetChildNames(dvConfigNodeConst node, size_t *numNames);
	DVSDK_EXPORT const char **dvConfigNodeGetAttributeKeys(dvConfigNodeConst node, size_t *numKeys);
	DVSDK_EXPORT enum dvConfigAttributeType dvConfigNodeGetAttributeType(dvConfigNodeConst node, const char *key);
	DVSDK_EXPORT struct dvConfigAttributeRanges dvConfigNodeGetAttributeRanges(
		dvConfigNodeConst node, const char *key, enum dvConfigAttributeType type);
	DVSDK_EXPORT int dvConfigNodeGetAttributeFlags(
		dvConfigNodeConst node, const char *key, enum dvConfigAttributeType type);
	DVSDK_EXPORT char *dvConfigNodeGetAttributeDescription(
		dvConfigNodeConst node, const char *key, enum dvConfigAttributeType type);

	DVSDK_EXPORT void dvConfigNodeAttributeModifierButton(dvConfigNode node, const char *key, const char *buttonLabel);
	DVSDK_EXPORT void dvConfigNodeAttributeModifierListOptions(
		dvConfigNode node, const char *key, const char *listOptions, bool allowMultipleSelections);
	DVSDK_EXPORT void dvConfigNodeAttributeModifierFileChooser(
		dvConfigNode node, const char *key, const char *typeAndExtensions);
	DVSDK_EXPORT void dvConfigNodeAttributeModifierUnit(
		dvConfigNode node, const char *key, const char *unitInformation);
	DVSDK_EXPORT void dvConfigNodeAttributeModifierPriorityAttributes(
		dvConfigNode node, const char *priorityAttributes);
	DVSDK_EXPORT void dvConfigNodeAttributeModifierGUISupport(dvConfigNode node);
	DVSDK_EXPORT void dvConfigNodeAttributeBooleanReset(dvConfigNode node, const char *key);

	DVSDK_EXPORT bool dvConfigNodeExistsRelativeNode(dvConfigNodeConst node, const char *nodePath);
	/**
	 * This returns a reference to a node, and as such must be carefully mediated with
	 * any dvConfigNodeRemoveNode() calls.
	 */
	DVSDK_EXPORT dvConfigNode dvConfigNodeGetRelativeNode(dvConfigNode node, const char *nodePath);

	// dv::Config Helper functions
	DVSDK_EXPORT const char *dvConfigHelperTypeToStringConverter(enum dvConfigAttributeType type);
	DVSDK_EXPORT enum dvConfigAttributeType dvConfigHelperStringToTypeConverter(const char *typeString);
	DVSDK_EXPORT char *dvConfigHelperValueToStringConverter(
		enum dvConfigAttributeType type, union dvConfigAttributeValue value);
	DVSDK_EXPORT union dvConfigAttributeValue dvConfigHelperStringToValueConverter(
		enum dvConfigAttributeType type, const char *valueString);
	DVSDK_EXPORT char *dvConfigHelperFlagsToStringConverter(int flags);
	DVSDK_EXPORT int dvConfigHelperStringToFlagsConverter(const char *flagsString);
	DVSDK_EXPORT char *dvConfigHelperRangesToStringConverter(
		enum dvConfigAttributeType type, struct dvConfigAttributeRanges ranges);
	DVSDK_EXPORT struct dvConfigAttributeRanges dvConfigHelperStringToRangesConverter(
		enum dvConfigAttributeType type, const char *rangesString);

	// dv::Config Tree
	typedef struct dv_config_tree *dvConfigTree;
	typedef const struct dv_config_tree *dvConfigTreeConst;
	typedef void (*dvConfigTreeErrorLogCallback)(const char *msg, bool fatal);

	DVSDK_EXPORT dvConfigTree dvConfigTreeGlobal(void);
	DVSDK_EXPORT dvConfigTree dvConfigTreeNew(void);
	DVSDK_EXPORT void dvConfigTreeDelete(dvConfigTree tree);
	DVSDK_EXPORT void dvConfigTreeErrorLogCallbackSet(dvConfigTreeErrorLogCallback error_log_cb);
	DVSDK_EXPORT dvConfigTreeErrorLogCallback dvConfigTreeErrorLogCallbackGet(void);

	DVSDK_EXPORT bool dvConfigTreeExistsNode(dvConfigTreeConst st, const char *nodePath);
	/**
	 * This returns a reference to a node, and as such must be carefully mediated with
	 * any dvConfigNodeRemoveNode() calls.
	 */
	DVSDK_EXPORT dvConfigNode dvConfigTreeGetNode(dvConfigTree st, const char *nodePath);

	typedef union dvConfigAttributeValue (*dvConfigAttributeUpdater)(
		void *userData, const char *key, enum dvConfigAttributeType type);

	DVSDK_EXPORT void dvConfigNodeAttributeUpdaterAdd(dvConfigNode node, const char *key,
		enum dvConfigAttributeType type, dvConfigAttributeUpdater updater, void *updaterUserData, bool runOnce);
	DVSDK_EXPORT void dvConfigNodeAttributeUpdaterRemove(dvConfigNode node, const char *key,
		enum dvConfigAttributeType type, dvConfigAttributeUpdater updater, void *updaterUserData);
	DVSDK_EXPORT void dvConfigNodeAttributeUpdaterRemoveAll(dvConfigNode node);
	DVSDK_EXPORT void dvConfigTreeAttributeUpdaterRemoveAll(dvConfigTree tree);
	DVSDK_EXPORT bool dvConfigTreeAttributeUpdaterRun(dvConfigTree tree);

	/**
	 * Listener must be able to deal with userData being NULL at any moment.
	 * This can happen due to concurrent changes from this setter.
	 */
	DVSDK_EXPORT void dvConfigTreeGlobalNodeListenerSet(
		dvConfigTree tree, dvConfigNodeChangeListener node_changed, void *userData);
	/**
	 * Listener must be able to deal with userData being NULL at any moment.
	 * This can happen due to concurrent changes from this setter.
	 */
	DVSDK_EXPORT void dvConfigTreeGlobalAttributeListenerSet(
		dvConfigTree tree, dvConfigAttributeChangeListener attribute_changed, void *userData);

#ifdef __cplusplus
}
#endif

#endif /* DVCONFIG_H_ */
