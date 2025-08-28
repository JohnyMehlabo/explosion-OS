#pragma once

#define FLAG_SET(x, b) (x) | 1 << (b)
#define FLAG_UNSET(x, b) (x) & ~(1 << (b))