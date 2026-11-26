# Test fixtures

`app/src/test/resources/fixtures/<key>/` holds RAM dumps of games running, used by the decode tests. They come from
commercial ROMs, so they are **not in the repo** (the directory is gitignored). Without them those tests skip; the
skip message names the missing fixture.

To run them, capture your own from ROMs you own: `scripts/capture_fixture.sh <key>` (keys and ROM names in
`scripts/roms.conf`; `scripts/capture_fixture_headless.sh` for the headless ones). The 3DS `make -C 3ds preview` and
the `ui-preview` render tasks read the same directory.

`make check` runs `scripts/check_no_fixtures.py`, which fails if a dump is tracked.
