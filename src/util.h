#pragma once

using RET_TYPE = bool;

#define RET_ERROR 1
#define RET_SUCCESS 0

#define min(a, b) a > b ? b : a
#define clamp(v, lo, hi) v < hi ? (v > lo ? v : lo) : hi