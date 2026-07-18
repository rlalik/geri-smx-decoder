![GERI-SMX-DECODER: API for SMX decoder](https://img.shields.io/badge/GERI%20SMX%20DECODER-API%20for%20SMX%20payload%20decoding-purple)
![GitHub](https://img.shields.io/github/license/rlalik/geri-smx-decoder)
[![Continuous Integration](https://github.com/rlalik/geri-smx-decoder/actions/workflows/ci.yml/badge.svg)](https://github.com/rlalik/geri-smx-decoder/actions/workflows/ci.yml)
[![Coverage Status](https://coveralls.io/repos/github/rlalik/geri-smx-decoder/badge.svg)](https://coveralls.io/github/rlalik/geri-smx-decoder)
[![C++11](https://github.com/rlalik/geri-smx-decoder/actions/workflows/c++11.yml/badge.svg)](https://github.com/rlalik/geri-smx-decoder/actions/workflows/c++11.yml)
[![C++14](https://github.com/rlalik/geri-smx-decoder/actions/workflows/c++14.yml/badge.svg)](https://github.com/rlalik/geri-smx-decoder/actions/workflows/c++14.yml)
[![C++17](https://github.com/rlalik/geri-smx-decoder/actions/workflows/c++17.yml/badge.svg)](https://github.com/rlalik/geri-smx-decoder/actions/workflows/c++17.yml)
[![C++20](https://github.com/rlalik/geri-smx-decoder/actions/workflows/c++20.yml/badge.svg)](https://github.com/rlalik/geri-smx-decoder/actions/workflows/c++20.yml)
[![C++23](https://github.com/rlalik/geri-smx-decoder/actions/workflows/c++23.yml/badge.svg)](https://github.com/rlalik/geri-smx-decoder/actions/workflows/c++23.yml)
[![C++26](https://github.com/rlalik/geri-smx-decoder/actions/workflows/c++26.yml/badge.svg)](https://github.com/rlalik/geri-smx-decoder/actions/workflows/c++26.yml)
![GitHub release (latest by date)](https://img.shields.io/github/v/release/rlalik/geri-smx-decoder)
![GitHub tag (latest by date)](https://img.shields.io/github/v/tag/rlalik/geri-smx-decoder)

# geri-smx-decoder

**GERI SMX decoder** provides simple API to read and decode SMX data frames embedded into GBT protocol of GERI system. It reads data directly from GERI but allows also to read from file for debugging.

Project page: https://github.com/rlalik/geri-smx-decoder

## How to start

Full usage examples are available in the `example` directory of this project.

1. Include the header in your project:
```c++
#include "geri-smx-decoder/geri-smx-decoder.hpp"
```
2. Create file reader, pass the file name:
```c++
geri::file_reader frdr(filename);
```
3. Create the decored using reader as a source:
```c++
auto decoder = geri::payload_decoder(&frdr);
```
4. Access the single frame:
```c++
auto res = decoder.decode_frame();
```
5. Do something with the hit data:
```c++
for (const auto& hit : res.hits)
{
    // do something with the hit, using struct members:
	// hit.gbt, hit.uplink, hit.channel, hit.adc, hit.full_ts, hit.event_missing
}
```
NOTE: Each call to this function will read the next event from the source (file). If EOF is reached, function will throw `std::out_of_range`. Once can use it to construct infinite loop to read the full file:
```c++
while (true)
{
    try
    {
        auto res = decoder.decode_frame();
        for (const auto& hit : res.hits)
        {
			// do something with the hit, using struct members:
			// hit.gbt, hit.uplink, hit.channel, hit.adc, hit.full_ts, hit.event_missing
        }
    }
    catch (const std::out_of_range&)
    {
        break;
    }
}
```

## GERI payload

The GERI data frame consists of:
1. Start frame containing `START` marker `0x579acce7` and event number.
2. Last system time word (must be matched against system time of previous frame).
3. Missing event bit
4. Empty word
5. Series of data words (SMX words)
6. Stop frame with `STOP` marker and repeated event number.
7. Current system time
8. Two empty words

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

## Hit data
Each payload data word contains of two 32-bit hit data. Hit data are sorted and the order is deterministic. The first is always the word in the lower 32-bits.

32-bit hit word consists of 8-bit GBT/uplink address (the MS8B part) and 24-bit SXM frame (the LS24B part). The GBT number is encoded on the upper 3-bits and uplink on the lower 5-bits of the GBT/uplink address byte.

To decode the SMX frame refer to the SMX documentation.

# Usage

In the standalone applications add `geri-smx-decoder/include` path to your include paths or copy `geri-smx-decoder.hpp` to your project include location.

The project uses C++23 features but also has some workaround for C++11.

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
