// Wide map loading for Black / White / Black 2 / White 2: the DS's own map window is 2x2 or 2x3 chunks, sized from
// MAP_CONFIGS in the field overlay. With 8 MB of main RAM the game's arena moves into the upper 4 MB and heap 1 grows, then
// those entries are made 5x3 before the field heap is sized and the map is set up: more land each way, out to EXPAND's
// farthest zoom step, instead of the void the game never loaded.
// The table patch alone doesn't fit in 4 MB (the bigger field heap exhausts it): the arena patch is the known working version
// (mamesj's pokemon-fulldex).
#pragma once

#include <stdint.h>

namespace melonDS {
class NDS;
}

struct PkNdsBigMap;

// The cart's table and heap setup if its game code is one of the versions the module knows (NULL for any other cart). The
// ROM's bytes; its game code is at 0x0C.
const PkNdsBigMap* pk_nds_bigmap_for(const uint8_t* rom);

// Before direct boot: 8 MB of main RAM (the upper half cleared). After it: the game's arena and its heap 1 in the upper half.
void pk_nds_bigmap_ram(const PkNdsBigMap* bigMap, melonDS::NDS& nds);
void pk_nds_bigmap_boot(const PkNdsBigMap* bigMap, melonDS::NDS& nds);

// After every frame: the arena and heap again until the game has made them (its start-up can put the 4 MB ones back),
// then the map table's outdoor entries made five wide and three tall once the field overlay is loaded and the heap is the
// bigger one.
void pk_nds_bigmap_frame(const PkNdsBigMap* bigMap, melonDS::NDS& nds);

// After a savestate load: whether it holds the game with the bigger heap (an 8 MB one). If not, it was made before wide map
// loading: the caller drops it for the session, and the upper half is made a mirror of the lower (that state's whole game
// is down there, and the DS's main RAM mirrors used to go there).
bool pk_nds_bigmap_state(const PkNdsBigMap* bigMap, melonDS::NDS& nds);
