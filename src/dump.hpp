#pragma once
#include "opts.hpp"

/// Run non-interactive hex dump, writing to stdout.
/// Respects o.color for ANSI output, o.dump must be true when called.
void run_dump(const Opts& o);
