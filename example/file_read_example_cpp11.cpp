#include "geri-smx-decoder/geri-smx-decoder.hpp"

#include <algorithm>
#include <chrono>
#include <cstdio>
#include <stdexcept>

#include <getopt.h>

namespace
{

auto parse_file(const char* filename, int verbose) -> void
{
    std::printf("Reading file: %s\n", filename);
    geri::file_reader frdr(filename);

    auto decoder = geri::payload_decoder(&frdr);

    int n_evts = 0;

    const std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    while (true)
    {
        try
        {
            auto res = decoder.decode_frame();
            if (verbose > 0) { std::printf("  Event: %u  payload size: %lu hits\n", res.event_no, res.hits.size()); }
            if (verbose > 1)
            {
                for (const auto& hit : res.hits)
                {
                    std::printf("  Hit  GBT: %u  Uplink: %2d  channel: %3d  adc: %3u  full ts: %016x  em: %d\n",
                                hit.gbt, hit.uplink, hit.channel, hit.adc, hit.full_ts, hit.event_missing);
                }
            }
            n_evts++;
        }
        catch (const std::out_of_range&)
        {
            break;
        }
    }
    const std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();

    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin);

    const auto s_to_ms_conv{1000.0};
    std::printf("Read %d events in %f s -- %.4f evts/s\n", n_evts, duration.count() / s_to_ms_conv,
                n_evts * s_to_ms_conv / duration.count());
}

} // namespace

auto main(int argc, char** argv) -> int
{
    int verbose{0};

    int code{0};
    while ((code = getopt(argc, argv, "vV")) != -1)
    {
        switch (code)
        {
            case 'v':
                verbose = std::max(1, verbose);
                break;
            case 'V':
                verbose = 2;
                break;
            default:
                abort();
        }
    }

    for (int index = optind; index < argc; index++)
    {
        parse_file(argv[index], verbose);
    }

    return 0;
}
