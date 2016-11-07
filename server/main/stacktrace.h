// ==========================================================================
// Project:   Xy Server
// Copyright: ©2016 Xy Group Ltd. All rights reserved.
// ==========================================================================

#ifndef XY_STACKTRACE_H
#define XY_STACKTRACE_H

namespace xy {

// Returns the demangled stack backtrace of the caller function.
auto stacktrace(unsigned int max_frames = 63) -> char *;

} // xy namespace

#endif // XY_STACKTRACE_H