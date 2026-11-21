#include "pk_nds_bigmap.h"

#include <NDS.h>
#include <ARMJIT_Memory.h>

#include <cstdlib>
#include <cstring>

using namespace melonDS;

struct PkNdsBigMap {
    char code[5];
    uint32_t table;      // MAP_CONFIGS, in the field overlay
    uint32_t heapSetup;  // HEAP_SETUP_NTR, in the main binary
    uint32_t heapUser;   // its heap 1 (HEAPID_USER) size as the game makes it on a DS
    uint32_t count;      // MAP_CONFIGS entries
};

// How much heap 1 is grown: the field's map heap (MAP_CONFIGS' +68) is taken from it, and the wider window's is bigger.
constexpr uint32_t HEAP_GROWTH = 0x180000;

// From the decompilation's addresses for each version (pokebw2's b2_us / w2_us / black_us / white_us), as mamesj's
// pokemon-fulldex lists them; White 2 is Black 2's build a little further along, and the ports are the US builds with
// their tables moved.
static const PkNdsBigMap BIG_MAPS[] = {
    {"IREO", 0x021CA01C, 0x0208EE68, 0x1A3E00, 29}, // Black 2 (USA / Europe)
    {"IRES", 0x021C9FE0, 0x0208EE38, 0x1A3E00, 29}, // Black 2 (Spain)
    {"IRDO", 0x021CA050, 0x0208EE94, 0x1A3E00, 29}, // White 2 (USA / Europe)
    {"IRDS", 0x021C9FF4, 0x0208EE64, 0x1A3E00, 29}, // White 2 (Spain)
    {"IREF", 0x021CA058, 0x0208EE78, 0x1A3E00, 29}, // Black 2 (France)
    {"IRDF", 0x021CA06C, 0x0208EEA4, 0x1A3E00, 29}, // White 2 (France)
    {"IRED", 0x021C9F88, 0x0208EDE8, 0x1A3E00, 29}, // Black 2 (Germany)
    {"IRDD", 0x021C9F9C, 0x0208EE14, 0x1A3E00, 29}, // White 2 (Germany)
    {"IREI", 0x021C9F38, 0x0208EDE8, 0x1A3E00, 29}, // Black 2 (Italy)
    {"IRDI", 0x021C9F6C, 0x0208EE14, 0x1A3E00, 29}, // White 2 (Italy)
    {"IREJ", 0x021C9768, 0x0208EB48, 0x1A3800, 29}, // Black 2 (Japan, rev 1)
    {"IRDJ", 0x021C977C, 0x0208EB74, 0x1A3800, 29}, // White 2 (Japan, rev 1)
    {"IREK", 0x021CA830, 0x0208F554, 0x1A3E00, 29}, // Black 2 (Korea)
    {"IRDK", 0x021CA844, 0x0208F580, 0x1A3E00, 29}, // White 2 (Korea)
    {"IRBO", 0x021D3700, 0x0209D8B0, 0x199500, 21}, // Black (USA / Europe)
    {"IRBS", 0x021D36C4, 0x0209D880, 0x199500, 21}, // Black (Spain)
    {"IRBF", 0x021D3678, 0x0209D830, 0x199500, 21}, // Black (France)
    {"IRBD", 0x021D3638, 0x0209D830, 0x199500, 21}, // Black (Germany)
    {"IRBI", 0x021D35E8, 0x0209D830, 0x199500, 21}, // Black (Italy)
    {"IRBJ", 0x021D3530, 0x0209D7BC, 0x199000, 21}, // Black (Japan)
    {"IRBK", 0x021D3DEC, 0x0209DF4C, 0x199500, 21}, // Black (Korea)
    {"IRAO", 0x021D3718, 0x0209D8C8, 0x199500, 21}, // White (USA / Europe)
    {"IRAS", 0x021D36BC, 0x0209D898, 0x199500, 21}, // White (Spain)
    {"IRAF", 0x021D3690, 0x0209D848, 0x199500, 21}, // White (France)
    {"IRAD", 0x021D3650, 0x0209D848, 0x199500, 21}, // White (Germany)
    {"IRAI", 0x021D3600, 0x0209D848, 0x199500, 21}, // White (Italy)
    {"IRAJ", 0x021D3548, 0x0209D7D4, 0x199000, 21}, // White (Japan)
    {"IRAK", 0x021D3DE4, 0x0209DF64, 0x199500, 21}, // White (Korea)
};

const PkNdsBigMap* pk_nds_bigmap_for(const uint8_t* rom)
{
    if (!rom) return nullptr;
    // A development aid, like mamesj's: PK_NDS_BIGMAP=0 boots the cart with the game's own 4 MB and map window.
    const char* off = getenv("PK_NDS_BIGMAP");
    if (off && !strcmp(off, "0")) return nullptr;
    for (const PkNdsBigMap& v : BIG_MAPS)
        if (memcmp(rom + 0x0C, v.code, 4) == 0) return &v;
    return nullptr;
}

static uint16_t read16(NDS& n, uint32_t addr)
{
    uint16_t v;
    memcpy(&v, &n.MainRAM[addr & n.MainRAMMask], 2);
    return v;
}

static uint32_t read32(NDS& n, uint32_t addr)
{
    uint32_t v;
    memcpy(&v, &n.MainRAM[addr & n.MainRAMMask], 4);
    return v;
}

static void write16(NDS& n, uint32_t addr, uint16_t v) { memcpy(&n.MainRAM[addr & n.MainRAMMask], &v, 2); }

static void write32(NDS& n, uint32_t addr, uint32_t v)
{
    memcpy(&n.MainRAM[addr & n.MainRAMMask], &v, 4);
    n.JIT.CheckAndInvalidate<0, ARMJIT_Memory::memregion_MainRAM>(addr & ~3u);
}

void pk_nds_bigmap_ram(const PkNdsBigMap* bigMap, NDS& nds)
{
    if (!bigMap) return;
    // The upper 4 MB, where the game's arena will go: clear what an earlier game left there, then let Reset's 4 MB mask be
    // replaced before direct boot writes its parameters (at 0x027FFxxx, which the mask maps to the upper half now).
    memset(nds.MainRAM + 0x400000, 0, 0x400000);
    nds.MainRAMMask = 0x7FFFFF;
}

// The game's arena (NitroSDK's OS_ARENA_MAIN) at the upper 4 MB, below the ARM7's part of main RAM, and heap 1 as large as
// the game makes it on a DSi. Before its start-up makes the heaps; every frame repeats it, since the start-up can put the
// 4 MB arena back first.
static void arena(NDS& n, const PkNdsBigMap* v)
{
    if (n.MainRAMMask != 0x7FFFFF) return;
    // The game's protection region for main RAM (region 1, 4 MB from the start-up code) made 8 MB, or the first use of the
    // upper half faults.
    if (n.ARM9.PU_Region[1] == 0x0200002B) n.ARM9.CP15Write(0x610, 0x0200002D);
    write32(n, 0x027FFDA0, 0x02400000);
    write32(n, 0x027FFDC4, 0x027E0000);
    if (read32(n, v->heapSetup + 8) == v->heapUser) write32(n, v->heapSetup + 8, v->heapUser + HEAP_GROWTH);
}

// MAP_CONFIGS: 72-byte entries. Outdoor maps load 2x2 or 2x3 chunks around the player; the farthest zoom step needs more
// land each way than that window keeps, so those entries are made 5x3 and their heap grows by the chunks added (each chunk's
// buffer and a little for its own state). One-deep entries (corridors, interiors) are left as they are. Idempotent, so a
// reloaded overlay is patched again; entries already 5x3 are left alone.
static void table(NDS& n, const PkNdsBigMap* v)
{
    if (read32(n, v->table) != 0x200000 || read32(n, v->table + 4) != 0x400000) return;
    // The map's heap is a single block from heap 1, so it can't ask for more than the grown heap holds.
    const uint32_t limit = v->heapUser + HEAP_GROWTH;
    for (uint32_t i = 0; i < v->count; i++) {
        const uint32_t e = v->table + i * 72;
        const uint16_t dx = read16(n, e + 8), dz = read16(n, e + 10);
        const uint32_t buffer = read32(n, e + 52), heap = read32(n, e + 68);
        if (dx < 2 || dz < 2 || dx > 5 || dz > 3 || buffer > 0x20000) continue;
        const uint32_t added = 15 - (uint32_t) dx * dz;
        const uint32_t grown = heap + added * (buffer + 0x3000);
        if (grown > limit) continue;
        write16(n, e + 8, 5);
        write16(n, e + 10, 3);
        write32(n, e + 68, grown);
    }
}

void pk_nds_bigmap_boot(const PkNdsBigMap* bigMap, NDS& nds)
{
    if (bigMap) arena(nds, bigMap);
}

void pk_nds_bigmap_frame(const PkNdsBigMap* bigMap, NDS& nds)
{
    if (!bigMap) return;
    arena(nds, bigMap);
    // Only with the bigger heap: the table's own heap sizes were made for it, and on the DS's 4 MB the field's doesn't fit.
    if (read32(nds, bigMap->heapSetup + 8) != bigMap->heapUser + HEAP_GROWTH) return;
    table(nds, bigMap);
}

bool pk_nds_bigmap_state(const PkNdsBigMap* bigMap, NDS& nds)
{
    if (!bigMap) return false;
    if (read32(nds, bigMap->heapSetup + 8) == bigMap->heapUser + HEAP_GROWTH) return true;
    // Before the game's heaps even (a state taken at its start): the arena is still ours.
    if (read32(nds, 0x027FFDA0) == 0x02400000) return true;
    // A state made before the patch: its game is all in the lower 4 MB. The mask stays 8 MB (changing it now would leave the
    // JIT's fast memory pages mapped for the other size); the upper half, where the DS's main RAM mirrors go now, is made a
    // copy of the lower, so every address the game used before reads the same.
    memcpy(nds.MainRAM + 0x400000, nds.MainRAM, 0x400000);
    return false;
}
