#pragma once

//#include "optick.h"

#include <algorithm>
#include <array>
#include <cstdint>
#include <functional>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <memory>
#include <nlohmann/json.hpp>
using json = nlohmann::json;
#include <optional>
#include <string>
#include <sstream>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include <Pigeon/Core/Log.h>
#include <Pigeon/Core/UUID.h>

#ifdef PG_PLATFORM_WINDOWS
	#include <Windows.h>
#endif