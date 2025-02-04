#pragma once

#include <cassert>
#include <cctype>
#include <cerrno>
#include <cfenv>
#include <cfloat>
#include <cinttypes>
#include <climits>
#include <clocale>
#include <cmath>
#include <csetjmp>
#include <csignal>
#include <cstdarg>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <cuchar>
#include <cwchar>
#include <cwctype>

#include <algorithm>
#include <any>
#include <array>
#include <atomic>
#include <barrier>
#include <bit>
#include <bitset>
#include <charconv>
#include <chrono>
#include <compare>
#include <complex>
#include <concepts>
#include <condition_variable>
#include <deque>
#include <exception>
#include <execution>
#include <filesystem>
#include <format>
#include <forward_list>
#include <fstream>
#include <functional>
#include <future>
#include <initializer_list>
#include <iomanip>
#include <iosfwd>
#include <ios>
#include <iostream>
#include <istream>
#include <iterator>
#include <latch>
#include <limits>
#include <locale>
#include <map>
#include <memory>
#include <memory_resource>
#include <mutex>
#include <new>
#include <numbers>
#include <numeric>
#include <optional>
#include <ostream>
#include <queue>
#include <random>
#include <ranges>
#include <regex>
#include <ratio>
#include <scoped_allocator>
#include <semaphore>
#include <set>
#include <shared_mutex>
#include <source_location>
#include <span>
#include <sstream>
#include <stack>
#include <stdexcept>
#include <streambuf>
#include <string>
#include <string_view>
#include <syncstream>
#include <system_error>
#include <thread>
#include <tuple>
#include <typeindex>
#include <typeinfo>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <valarray>
#include <variant>
#include <vector>
#include <version>

#include <RE/Skyrim.h>
#include <SKSE/SKSE.h>
#include <REL/Relocation.h>

#include <ShlObj_core.h>
#include <Windows.h>
#include <Psapi.h>
#undef cdecl // Workaround for Clang 14 CMake configure error.

#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/msvc_sink.h>

#include <BGSEntryPointFunctionDataTwoValue.h>



#include "Lexicon.h"

#include "GameObjectStuff.h"

//namespace logger = SKSE::log;




//#define ARTH_OBJECT_TYPE RE::TESForm
//#define ARTH_CONTEXT_TYPE RE::ExtraDataList
//#define ARTH_ENUM_TYPE RE::FormType

#define ARTHMETIC_LOGGER(mc_level, mc_text, ...) logger::mc_level(mc_text __VA_OPT__(,)__VA_ARGS__)

#define AVG_SOURCE 1
#include "API_ActorValueGenerator.h"
#undef AVG_SOURCE

#include <toml++/toml.h>

#include <boost/algorithm/string.hpp>
#include <boost/regex.hpp>

#include "Plugin.h"



namespace AVG
{
    using namespace RGL_NAMESPACE;
    using namespace RGL_INCLUDE_NAMESPACE;
}

// Compatible declarations with other sample projects.
#define DLLEXPORT __declspec(dllexport)

using namespace std::literals;
using namespace REL::literals;



namespace logger
{
    using namespace SKSE::log;
}

namespace util {
    using SKSE::stl::report_and_fail;
}



#define RELOCATION_OFFSET(SE, AE, ...) REL::VariantOffset(SE, AE, SE).offset()

//Move these eventually pls.
inline LEX::IScript* commons = nullptr;
inline LEX::IScript* legacy = nullptr;