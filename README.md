# geri-smx-decoder

This is the geri-smx-decoder project.

It provides simple API to read and decode SMX data frames embedded into GBT protocol of GERI system. It reads data directly from GERI but allows also to read from file for debugging.

### GERI payload

The GERI data frame consists of:
1. Start frame containing `START` marker `0x579acce7` and event number.
1. Last system time word (must be matched against system time of previous frame).
1. Missing event bit
1. Empty word
1. Series of data words (SMX words)
1. Stop frame with `STOP` marker and repeated event number.
1. Current system time
1. Two empty words

Each word of the frame is a 64-bit value. Here is an example for a single event frame:
```
0x000daef7579acce7  // 000daef7 -- event number, 579acce7 -- START marker
0x000000bc2c102745  // system timestamp of the previous frame
0x0000000000000000  // bit 0 == 0 - no data dropped flag
0x0000000000000000  // all 0
0x08000450097ffdde  // payload data
0x0800054009f7df76  // payload data
...
0x08c2082609e0820d  // ...
0x08000428097ff9b0  // ...
0x0800051809e1861b  // ...
0x08e1861b097fface  // ...
0x000daef7ed9acce7  // payload data
0x000000bc2c1641c5  // repeated event number + STOP marker
0x0000000000000000  // all 0s
0x0000000000000000  // all 0s
```
The next frame would start with:
```
0x000daef8579acce7  // 000daef8 -- next event number, 579acce7 -- START marker
0x000000bc2c1641c5  // system timestamp matching of previous event
...
```

### Hit data
Each payload data word contains of two 32-bit hit data. Hit data are sorted and the order is deterministic. The first is always the word in the lower 32-bits.

32-bit hit word consists of 8-bit GBT/uplink address (the MS8B part) and 24-bit SXM frame (the LS24B part). The GBT number is encoded on the upper 3-bits and uplink on the lower 5-bits of the GBT/uplink address byte.

To decode the SMX frame refer to the SMX documentation.

# Usage

In the standalone applications add `geri-smx-decoder/include` path to your include patha or copy `geri-smx-decoder.hpp` to your project include location.

The project uses c++23 features thus you must compile with `-std=c++23` flags (or newer).

# Building and installing

See the [BUILDING](BUILDING.md) document.

# Contributing

See the [CONTRIBUTING](CONTRIBUTING.md) document.

# Licensing

<!--
Please go to https://choosealicense.com/licenses/ and choose a license that
fits your needs. The recommended license for a project of this type is the
Boost Software License 1.0.
-->
