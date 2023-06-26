#pragma once

#include <string_view>

/**
 * dv-runtime version (MAJOR * 10000 + MINOR * 100 + PATCH).
 */
#define DV_RUNTIME_VERSION_MAJOR 1
#define DV_RUNTIME_VERSION_MINOR 6
#define DV_RUNTIME_VERSION_PATCH 1
#define DV_RUNTIME_VERSION \
	((DV_RUNTIME_VERSION_MAJOR * 10000) + (DV_RUNTIME_VERSION_MINOR * 100) + DV_RUNTIME_VERSION_PATCH)

/**
 * dv-runtime name string.
 */
#define DV_RUNTIME_NAME_STRING "dv-runtime"

/**
 * dv-runtime version string.
 */
#define DV_RUNTIME_VERSION_STRING "1.6.1"

namespace dv::runtime {

static constexpr int VERSION_MAJOR{DV_RUNTIME_VERSION_MAJOR};
static constexpr int VERSION_MINOR{DV_RUNTIME_VERSION_MINOR};
static constexpr int VERSION_PATCH{DV_RUNTIME_VERSION_PATCH};
static constexpr int VERSION{DV_RUNTIME_VERSION};

static constexpr std::string_view NAME_STRING{DV_RUNTIME_NAME_STRING};
static constexpr std::string_view VERSION_STRING{DV_RUNTIME_VERSION_STRING};

} // namespace dv::runtime
