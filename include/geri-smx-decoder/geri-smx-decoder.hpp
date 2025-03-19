#pragma once

#include <cstdint>
#include <cstdlib>
#include <map>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#ifdef __cpp_lib_format
#include <format>
#endif

#ifdef __cpp_lib_print
#include <print>
#endif

#ifndef __cpp_lib_format
#define BYTE_TO_BINARY_PATTERN "%c%c%c%c%c%c%c%c"
#define BITS2_TO_BINARY_PATTERN "%c%c"
#define BITS12_TO_BINARY_PATTERN "%c%c%c%c%c%c%c%c%c%c%c%c"
// clang-format off
#define BYTE_TO_BINARY(byte)  \
((byte) & 0x80 ? '1' : '0'), \
((byte) & 0x40 ? '1' : '0'), \
((byte) & 0x20 ? '1' : '0'), \
((byte) & 0x10 ? '1' : '0'), \
((byte) & 0x08 ? '1' : '0'), \
((byte) & 0x04 ? '1' : '0'), \
((byte) & 0x02 ? '1' : '0'), \
((byte) & 0x01 ? '1' : '0')

#define BITS2_TO_BINARY(byte)  \
((byte) & 0x2 ? '1' : '0'), \
((byte) & 0x1 ? '1' : '0')

#define BITS12_TO_BINARY(byte)  \
((byte) & 0x800 ? '1' : '0'), \
((byte) & 0x400 ? '1' : '0'), \
((byte) & 0x200 ? '1' : '0'), \
((byte) & 0x100 ? '1' : '0'), \
((byte) & 0x080 ? '1' : '0'), \
((byte) & 0x040 ? '1' : '0'), \
((byte) & 0x020 ? '1' : '0'), \
((byte) & 0x010 ? '1' : '0'), \
((byte) & 0x008 ? '1' : '0'), \
((byte) & 0x004 ? '1' : '0'), \
((byte) & 0x002 ? '1' : '0'), \
((byte) & 0x001 ? '1' : '0')
// clang-format on
#endif

namespace geri
{

namespace exceptions
{
/**
 * Thrown when the ts_msb<9:8> and hit.ts<9:8> bits don't match.
 */
class ts_match_error : public std::exception
{
public:
    ts_match_error(uint16_t event_ts, uint16_t hit_tsb) : ts_event{event_ts}, ts_hit{hit_tsb}
    {
#ifdef __cpp_lib_format
        message = std::format("Event ts = {:#016b} , hit ts = {:#012b}:  bits<9:8> == {:#04b} vs {:#04b}", ts_event,
                              ts_hit, (ts_event >> 8) & 0x3, (ts_hit >> 8) & 0x3);
#else
        char buf[200];
        sprintf(buf,
                "Event ts = 0b " BYTE_TO_BINARY_PATTERN " " BYTE_TO_BINARY_PATTERN
                " , hit ts = 0b" BITS12_TO_BINARY_PATTERN ":  bits<9:8> == 0b" BITS2_TO_BINARY_PATTERN
                " vs 0b" BITS2_TO_BINARY_PATTERN,
                BYTE_TO_BINARY(ts_event >> 8), BYTE_TO_BINARY(ts_event), BITS12_TO_BINARY(ts_hit),
                BITS2_TO_BINARY((ts_event >> 8) & 0x3), BITS2_TO_BINARY((ts_hit >> 8) & 0x3));
        message = buf;
#endif
    }

    auto what() const noexcept -> const char* override { return message.c_str(); }

private:
    uint16_t ts_event, ts_hit;
    std::string message;
};

/**
 * Throws when the TS_MSB frame is incorrect.
 */
class ts_msb_error : public std::exception
{
public:
    explicit ts_msb_error(uint32_t _ts_msb) : ts_msb{_ts_msb} {}

private:
    uint32_t ts_msb;
};

class invalid_gbt_frame : public std::exception
{
};

} // namespace exceptions

namespace smx
{
/**
 * SMX uplink frame types. See SMX documentation for details.
 */
enum class UPLINK_FRAME_TYPE : std::uint8_t
{
    dummy_hit,
    hit,
    ts_msb,
    rdata_ack,
    ack,
    nack,
    alert_ack,
    seq_error
};

/**
 * Decodes the SMX uplink frame header type.
 *
 * @param word the 24-bit data word
 * @return the frame type
 */
constexpr auto get_uplink_frame_type(uint32_t word) -> UPLINK_FRAME_TYPE
{
    auto header = word >> 19;

    if (header & 0x10)
    {
        // it's something else than hit
        if ((header & 0b11000) == 0b11000) { return UPLINK_FRAME_TYPE::ts_msb; }
        if ((header & 0b11100) == 0b10100) { return UPLINK_FRAME_TYPE::rdata_ack; }
        if ((header & 0b11111) == 0b10001) { return UPLINK_FRAME_TYPE::ack; }
        if ((header & 0b11111) == 0b10010) { return UPLINK_FRAME_TYPE::nack; }
        if ((header & 0b11111) == 0b10011) { return UPLINK_FRAME_TYPE::alert_ack; }
        if ((header & 0b11111) == 0b10000) { return UPLINK_FRAME_TYPE::seq_error; }
    }

    // it's hit
    if ((word & 0x7ffe00) == 0x0) { return UPLINK_FRAME_TYPE::dummy_hit; }

    return UPLINK_FRAME_TYPE::hit;
}

/** store single hit data.
 */
struct hit
{
    uint8_t channel{0};
    uint8_t adc{0};
    uint16_t ts{0};
    uint16_t full_ts{0};
    bool event_missing{false};
};

/**
 * Decode HIT uplink frame. Bits configuration (3 8-bit words, MSB first):
 *   ```0ccccccc aaaaattt ttttttte```
 * where: e - event missing bit
 *        t - 10-bit timestamp bits <9:0>
 *        a - 5-bit adc value, always > 0
 *        c - 7-bit address channel
 *
 * @param word 24-bit board
 * @return the hit structure
 */
inline auto decode_smx_hit(uint32_t word, uint16_t event_ts) -> hit
{
    hit decoded_hit;

    // decode hit data
    decoded_hit.event_missing = word & 0b1; // [0] event missed
    word >>= 1;

    decoded_hit.ts = word & 0x3ff;          //  [10-1] timestamp <9:0>
    word >>= 10;

    decoded_hit.adc = word & 0x1f;          //  [15-11] adc
    word >>= 5;

    decoded_hit.channel = word & 0x3f;      //  [22-16] channel address

    if ((event_ts != 0x0) and ((event_ts >> 8) & 0x3) != (decoded_hit.ts >> 8))
        throw geri::exceptions::ts_match_error(event_ts, decoded_hit.ts);

    decoded_hit.full_ts = event_ts | decoded_hit.ts;

    return decoded_hit;
}

/**
 * Decode TS_MSB uplink frame. Bits configuration (3 8-bit words, MSB first):
 *   ```11xxxxxx yyyyyyzz zzzzcccc```
 * where: x,y,z - same value of ts_msb
 *        c - 4-bits CRC
 *
 * @param word 24-bit board
 * @return the hit structure
 */
constexpr auto decode_smx_ts_msb(uint32_t word) -> uint16_t
{
    // auto crc = word & 0xf; TODO add CRC checking?
    word >>= 4;

    auto ts_13_8_0 = static_cast<uint16_t>(word & 0x3f); //  [9-4] TS<13:8> #1
    word >>= 6;

    auto ts_13_8_1 = static_cast<uint16_t>(word & 0x3f); //  [15-10] TS<13:8> #2
    word >>= 6;

    auto ts_13_8_2 = static_cast<uint16_t>(word & 0x3f); //  [21-16] TS<13:8> #3
    word >>= 6;

    auto static_bit_22 = word & 0x1;                     //  [22] frame type
    word >>= 1;

    auto check_ts = (static_bit_22 == 1) and ((ts_13_8_2 == ts_13_8_1) and (ts_13_8_1 == ts_13_8_0));

    if (!check_ts) throw geri::exceptions::ts_msb_error(word);

    return ts_13_8_0 << 8;
}

} // namespace smx

namespace gbt
{

struct gbt_uplink_addr
{
    uint8_t gbt{0x0};
    uint8_t uplink{0x0};
    uint8_t unique_addr{0x0};
};

/**
 * Get Gbt address (uplink + GBT) from the word.
 */
constexpr auto get_gbt_uplink_addr(uint32_t word)
{
    gbt_uplink_addr decoded_addr;

    word >>= 24;
    decoded_addr.unique_addr = static_cast<uint8_t>(word & 0xff);
    decoded_addr.uplink = word & 0x1f; //  [28-24] uplink no
    word >>= 5;
    decoded_addr.gbt = word & 0x3;     //  [31-29] gbt no

    return decoded_addr;
}

} // namespace gbt

struct gbt_hit : gbt::gbt_uplink_addr, smx::hit
{
    explicit gbt_hit(const gbt::gbt_uplink_addr& addr) : gbt::gbt_uplink_addr{addr} {}

    auto operator=(const smx::hit& rhs) -> gbt_hit&
    {
        channel = rhs.channel;
        adc = rhs.adc;
        ts = rhs.ts;
        full_ts = rhs.full_ts;
        event_missing = rhs.event_missing;

        return *this;
    }
};

struct payload_frame
{
    uint32_t event_no{0};
    uint64_t system_ts{0};
    bool data_dropped{false};

    std::vector<gbt_hit> hits;

    payload_frame() { hits.reserve(1024L * 1024L); }
};

inline void close_file(std::FILE* fp) { std::fclose(fp); }

class file_reader
{
private:
    std::unique_ptr<FILE, decltype(&close_file)> fp;

public:
    explicit file_reader(const char* filename) : fp{fopen(filename, "rxe"), &close_file}
    {
        if (fp == nullptr) { abort(); }
    }

    auto read_word() -> uint64_t
    {
        uint64_t data{0x0};
        auto status = fread(&data, 8, 1, fp.get());

        if (status == 0) { throw std::out_of_range("END OF DATA"); }

        return data;
    }
};

template <typename T> class payload_decoder
{
private:
    T* data_reader{nullptr};

    static const uint64_t start_marker{0x579acce7};
    static const uint64_t stop_marker{0xed9acce7};

    uint64_t last_systime = 0;

    auto expect_word(uint64_t word, uint64_t expected) -> bool
    {
        if (word != expected)
        {
#ifdef __cpp_lib_print
            std::print("Expected {:#018x} got {:#018x}\n", expected, word);
#elifdef __cpp_lib_format
            std::printf("%s", std::format("Expected {:#018x} got {:#018x}\n", expected, word).c_str());
#else
            std::printf("Expected %#018lx got %#018lx\n", expected, word);
#endif
            return false;
        }
        return true;
        ;
    }

public:
    explicit payload_decoder(T* reader) : data_reader(reader) {}

    /**
     * Decode the dataframe.
     *
     * Dataframe starts with four 64-bit words:
     * 1. 0xEEEEEEEEMMMMMMMM - E - event number,  M - start marker
     * 2. 0xSSSSSSSSSSSSSSSS - s - system time
     * 3. 0x0000000000000000
     * 4. 0x000000000000000D - data dropped persist bit
     *
     * and ends up with another four 64-bit words:
     * 1. 0xEEEEEEEEMMMMMMMM - E - event number,  M - stop marker
     * 2. 0xSSSSSSSSSSSSSSSS - s - system time
     * 3. 0x0000000000000000
     * 4. 0x0000000000000000
     *
     * In between there are data words. Each 64-bit data word contains 2 32-bit words.
     * 32-bit format: 0xAASSSSSS - A gbt/uplink 8-bit address, S - SMX 24-bit words
     * and address: 0bggguuuuu - g 3-bit GBT address, u - 5-bit uplink address
     *
     * The 32-bit data words within 64-bit data words are sorted, first read the 32 LS32B,
     * then MS32B.
     */
    auto decode_frame() -> payload_frame
    {
        payload_frame payload_data;

        payload_data.event_no = [&]() -> uint32_t
        {
            while (true)
            {
                auto word = data_reader->read_word();

                if ((word & start_marker) == start_marker) { return static_cast<uint32_t>(word >> 32); }
                // else
                // {
                // std::print("Invalid data word {:#018x}\n", word);
                // }
            }
        }();

        // std::print("Detected event {:d}\n", payload_data.event_no);

        {
            auto word = data_reader->read_word();

            if (last_systime and word != last_systime)
            {
                // std::print("Invalid System Time {:#018x},  expected: {:#018x}", word, last_systime);
            }

            word = data_reader->read_word();

            payload_data.data_dropped = word & 0x1;
            // if (payload_data.data_dropped)
            // {
            //     std::print("  Data dropped persist bit detected\n");
            // }

            if (!expect_word(data_reader->read_word(), 0x0)) { throw geri::exceptions::invalid_gbt_frame(); }

            // std::print("Event {:d}   System Time {:#018x}\n", payload_data.event_no, word);
        }

        std::map<uint32_t, uint16_t> gbt_event_ts;

        while (true)
        {
            auto word = data_reader->read_word();
            if ((word & stop_marker) == stop_marker)
            {
                if (word >> 32 != payload_data.event_no)
                {
                    // std::print("Decoded wrong event number at frame end: {:#018x}\n", word);
                }
                else { break; }
            }
            else
            {
                // std::print("Full word: {:#018x}\n", word);

                uint32_t words[2] = {static_cast<uint32_t>(word & 0xffffffff), static_cast<uint32_t>(word >> 32)};

                for (const auto data_word : words)
                {
                    gbt_hit payload{gbt::get_gbt_uplink_addr(data_word)};

                    // std::print("Current word: {:08x} -- GBT: {:d}  Uplink: {:2d} -- ", data_word, payload.gbt,
                    //            payload.uplink);

                    auto word_type = smx::get_uplink_frame_type(data_word);

                    switch (word_type)
                    {
                        case smx::UPLINK_FRAME_TYPE::hit:
                        {
                            try
                            {
                                auto last_ts = gbt_event_ts[payload.unique_addr];
                                payload = smx::decode_smx_hit(data_word, last_ts);

                                // std::print("SMX data: {}\n", payload);

                                payload_data.hits.push_back(std::move(payload));
                                // if (!last_ts)
                                // std::print("hit word - event ts unknown\n");
                                // else
                                // std::print("hit word\n");
                            }
                            catch (const geri::exceptions::ts_match_error& e)
                            {
                                // std::print("ERROR: {:s}\n", e.what());
                            }
                        }
                        break;

                        case smx::UPLINK_FRAME_TYPE::ts_msb:
                        {
                            gbt_event_ts[payload.unique_addr] = smx::decode_smx_ts_msb(data_word);
                            // std::print("ts_msb word, current timestamp: {:x}\n", gbt_event_ts[payload.unique_addr]);
                        }
                        break;

                        default:
                        {
#ifdef __cpp_lib_print
                            std::print(stderr, "other word -- unsupporter yet\n");
#else
                            std::fprintf(stderr, "other word -- unsupporter yet\n");
#endif
                        }
                        break;
                    }
                }
            }
        }

        // std::print("Readout {:d} channels data\n", channels_cnt);

        {
            last_systime = data_reader->read_word();

            // std::print("New System Time: {:#018x}\n", last_systime);

            if (!expect_word(data_reader->read_word(), 0x0)) { throw geri::exceptions::invalid_gbt_frame(); }
            if (!expect_word(data_reader->read_word(), 0x0)) { throw geri::exceptions::invalid_gbt_frame(); }
        }

        payload_data.system_ts = last_systime;

        return payload_data;
    }
};

} // namespace geri

#ifdef __cpp_lib_format

template <> struct std::formatter<geri::gbt::gbt_uplink_addr>
{
    static constexpr auto parse(std::format_parse_context& ctx) { return ctx.begin(); }

    static auto format(const geri::gbt::gbt_uplink_addr& obj, std::format_context& ctx)
    {
        return std::format_to(ctx.out(), "GBT: {:d}  Uplink: {:2d}", obj.gbt, obj.uplink);
    }
};

template <> struct std::formatter<geri::smx::hit>
{
    static constexpr auto parse(std::format_parse_context& ctx) { return ctx.begin(); }

    static auto format(const geri::smx::hit& obj, std::format_context& ctx)
    {
        return std::format_to(ctx.out(), "channel: {:3d}  adc: {:3d}  full ts: {:016x}  em: {:b}", obj.channel, obj.adc,
                              obj.full_ts, obj.event_missing);
    }
};

template <> struct std::formatter<geri::gbt_hit>
{
    static constexpr auto parse(std::format_parse_context& ctx) { return ctx.begin(); }

    static auto format(const geri::gbt_hit& obj, std::format_context& ctx)
    {
        return std::format_to(ctx.out(), "{}  {}", static_cast<geri::gbt::gbt_uplink_addr>(obj),
                              static_cast<geri::smx::hit>(obj));
    }
};

#endif
