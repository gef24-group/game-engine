#pragma once

#ifdef PROFILE
#include "tracy/Tracy.hpp"
#ifdef ZoneScoped
#endif
// NOLINTNEXTLINE(clang-tidy, readability-identifier-naming)
#define TracySetThreadName(name) tracy::SetThreadName(name)
#else
// NOLINTNEXTLINE(clang-tidy, readability-identifier-naming)
#define ZoneScoped
// NOLINTNEXTLINE(clang-tidy, readability-identifier-naming)
#define ZoneText(text, size)
// NOLINTNEXTLINE(clang-tidy, readability-identifier-naming)
#define ZoneScopedNC(name, color)
// NOLINTNEXTLINE(clang-tidy, readability-identifier-naming)
#define FrameMark
// NOLINTNEXTLINE(clang-tidy, readability-identifier-naming)
#define TracySetThreadName(name)
// NOLINTNEXTLINE(clang-tidy, readability-identifier-naming)
#define FrameImage(image, width, height, offset, flip)
#endif

#define PROFILED