#include <cstdint>

extern const uint32_t g_draw_call_count { 100000 };

// I make assumptions about even divisibility with 100 (for percentage reasons)
static_assert(g_draw_call_count % 100 == 0);
