package com.pokedaisy.app.companion.data

import org.junit.Assume

/**
 * Replays a captured real-device EWRAM+IWRAM snapshot (see
 * scripts/capture_fixture.sh, which writes app/src/test/resources/fixtures/<key>/
 * {ewram,iwram}.bin via PokeDaisyActivity's dumpFixtureIfPending()) as a
 * [MemoryReader] - so readNativeTelemetry can be exercised
 * against real device state with no device or emulator needed.
 *
 * Deliberately whole-region (full 256 KiB EWRAM + 32 KiB IWRAM), not a capture
 * of just the fields one decoder happens to read: this lets ANY address a
 * decoder asks for resolve correctly, including pointer chases through
 * per-boot-randomized offsets (gSaveBlock2Ptr's ASLR-style offset, bag pocket
 * pointers, ...) that a narrower capture could miss.
 */
class FixtureMemoryReader private constructor(
    private val ewram: ByteArray,
    private val iwram: ByteArray,
    // A Game Boy / Color fixture (wram.bin / hram.bin, mgba_dump's GB `dump`) instead:
    // WRAM at 0xC000 (the bank in D000 at capture time), HRAM at 0xFF80.
    private val gbRegions: List<Pair<Long, ByteArray>> = emptyList(),
) : MemoryReader {
    override fun readCoreMemory(addr: Long, size: Int): ByteArray {
        val (base, buf) = regionFor(addr, size)
            ?: throw TelemetryDecodeException(
                "fixture has no data for 0x${addr.toString(16)}..+$size " +
                    "(captured EWRAM 0x${EWRAM_BASE.toString(16)}+0x${ewram.size.toString(16)} " +
                    "and IWRAM 0x${IWRAM_BASE.toString(16)}+0x${iwram.size.toString(16)} only)",
            )
        val off = (addr - base).toInt()
        return buf.copyOfRange(off, off + size)
    }

    private fun regionFor(addr: Long, size: Int): Pair<Long, ByteArray>? = when {
        gbRegions.isNotEmpty() -> gbRegions.firstOrNull { (base, buf) -> addr >= base && addr + size <= base + buf.size }
        addr >= EWRAM_BASE && addr + size <= EWRAM_BASE + ewram.size -> EWRAM_BASE to ewram
        addr >= IWRAM_BASE && addr + size <= IWRAM_BASE + iwram.size -> IWRAM_BASE to iwram
        else -> null
    }

    companion object {
        private const val EWRAM_BASE = 0x02000000L
        private const val IWRAM_BASE = 0x03000000L

        /** One raw file under fixtures/ (e.g. "platinum/mainram.bin"); the test skips when it isn't there. */
        fun bytes(path: String): ByteArray {
            val stream = FixtureMemoryReader::class.java.classLoader.getResourceAsStream("fixtures/$path")
            Assume.assumeTrue("no fixture fixtures/$path - see test-fixtures/README.md", stream != null)
            return stream!!.use { it.readBytes() }
        }

        /** A fixture directory (e.g. "pbr_battle"); the test skips when it isn't there. */
        fun dir(path: String): java.io.File {
            val url = FixtureMemoryReader::class.java.classLoader.getResource("fixtures/$path")
            Assume.assumeTrue("no fixture fixtures/$path - see test-fixtures/README.md", url != null)
            return java.io.File(url!!.toURI())
        }

        /** The fixture directories present (empty without fixtures). */
        fun keys(): List<String> =
            FixtureMemoryReader::class.java.classLoader.getResource("fixtures")?.let { java.io.File(it.toURI()).list()?.toList() } ?: emptyList()

        /** A dump outside the fixtures: [dir]/ewram.bin + iwram.bin (scripts/verify_ports.py's); skips when absent. */
        fun fromDir(dir: java.io.File): FixtureMemoryReader {
            for (name in listOf("ewram.bin", "iwram.bin")) {
                Assume.assumeTrue("no dump $dir/$name (needs your own ROM)", java.io.File(dir, name).isFile)
            }
            return FixtureMemoryReader(java.io.File(dir, "ewram.bin").readBytes(), java.io.File(dir, "iwram.bin").readBytes())
        }

        /** [key] must match a directory under app/src/test/resources/fixtures/
         * (and a line in scripts/roms.conf - that's where it came from); the test skips when it isn't there. */
        fun load(key: String): FixtureMemoryReader {
            fun stream(name: String) = FixtureMemoryReader::class.java.classLoader.getResourceAsStream("fixtures/$key/$name")
            // The RAM dumps are not in the repo (they come from commercial ROMs): without them the test skips.
            // Capture your own with scripts/capture_fixture.sh <key>; see test-fixtures/README.md.
            fun read(name: String): ByteArray = stream(name)?.use { it.readBytes() }
                ?: error("fixture fixtures/$key has no $name")
            Assume.assumeTrue(
                "no fixture fixtures/$key - run scripts/capture_fixture.sh $key (see test-fixtures/README.md)",
                listOf("mainram.bin", "wram.bin", "ewram.bin").any { stream(it) != null },
            )
            if (stream("mainram.bin") != null) {
                // A DS game's main RAM (nds_dump's dump: 4 MB at 0x02000000, where Gen4Reader reads it).
                return FixtureMemoryReader(read("mainram.bin"), ByteArray(0))
            }
            if (stream("wram.bin") != null) {
                // A Color cart's whole WRAM (mgba_dump's wramx.bin, 8 banks): bank B's D000-DFFF at
                // (B << 16) | 0xD000, the way the app's native reader takes them (Gen2Reader's wramx).
                val banks = if (stream("wramx.bin") != null) read("wramx.bin").let { x ->
                    (0 until x.size / 0x1000).map { b -> ((b.toLong() shl 16) or 0xD000L) to x.copyOfRange(b * 0x1000, b * 0x1000 + 0x1000) }
                } else emptyList()
                return FixtureMemoryReader(ByteArray(0), ByteArray(0), listOf(0xC000L to read("wram.bin"), 0xFF80L to read("hram.bin")) + banks)
            }
            return FixtureMemoryReader(read("ewram.bin"), read("iwram.bin"))
        }
    }
}
