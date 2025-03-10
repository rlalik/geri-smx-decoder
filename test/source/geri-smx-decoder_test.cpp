#include <gtest/gtest.h>

#include "geri-smx-decoder/geri-smx-decoder.hpp"

TEST(TestGeriSmx, UplinkFrameType)
{
    ASSERT_EQ(geri::smx::get_uplink_frame_type(0x000000), geri::smx::UPLINK_FRAME_TYPE::dummy_hit);
    ASSERT_EQ(geri::smx::get_uplink_frame_type(0x00f800), geri::smx::UPLINK_FRAME_TYPE::hit);

    ASSERT_EQ(geri::smx::get_uplink_frame_type(0xc00000), geri::smx::UPLINK_FRAME_TYPE::ts_msb);

    ASSERT_EQ(geri::smx::get_uplink_frame_type(0xa00000), geri::smx::UPLINK_FRAME_TYPE::rdata_ack);

    ASSERT_EQ(geri::smx::get_uplink_frame_type(0x880000), geri::smx::UPLINK_FRAME_TYPE::ack);
    ASSERT_EQ(geri::smx::get_uplink_frame_type(0x900000), geri::smx::UPLINK_FRAME_TYPE::nack);
    ASSERT_EQ(geri::smx::get_uplink_frame_type(0x980000), geri::smx::UPLINK_FRAME_TYPE::alert_ack);

    ASSERT_EQ(geri::smx::get_uplink_frame_type(0x800000), geri::smx::UPLINK_FRAME_TYPE::seq_error);
}

TEST(TestGeriSmx, HitDecodingEmptyTS)
{
    // 0b0'0000'0001'0010'0011'0100'0101;
    auto word = 0x012345;

    ASSERT_EQ(geri::smx::get_uplink_frame_type(word), geri::smx::UPLINK_FRAME_TYPE::hit);

    auto res = geri::smx::decode_smx_hit(word, 0x0);

    ASSERT_EQ(res.event_missing, true);
    ASSERT_EQ(res.ts, 0x1a2);
    ASSERT_EQ(res.adc, 0x4);
    ASSERT_EQ(res.channel, 0x1);
}

TEST(TestGeriSmx, HitDecodingValidTS)
{
    // 0b0'0000'0001'0010'0011'0100'0101;
    auto word = 0x012345;

    ASSERT_EQ(geri::smx::get_uplink_frame_type(word), geri::smx::UPLINK_FRAME_TYPE::hit);

    auto res = geri::smx::decode_smx_hit(word, 0x1 << 8);

    ASSERT_EQ(res.event_missing, true);
    ASSERT_EQ(res.ts, 0x1a2);
    ASSERT_EQ(res.adc, 0x4);
    ASSERT_EQ(res.channel, 0x1);
}

TEST(TestGeriSmx, HitDecodingMismatch)
{
    try
    {
        geri::smx::decode_smx_hit(0x012345, 0x8);
    }
    catch (geri::exceptions::ts_match_error e)
    {
        // ASSERT_EQ(e.what(), "");
    }
}

TEST(TestGeriSmx, TsMsbDecoding)
{
    // 0b0'1101'1001'0110'0101'1001'0000; // fake CRC
    // full_ts = 0b0'011001'00000000
    auto word = 0xd96590;

    ASSERT_EQ(geri::smx::get_uplink_frame_type(word), geri::smx::UPLINK_FRAME_TYPE::ts_msb);
    ASSERT_EQ(geri::smx::decode_smx_ts_msb(word), 0b011001'00000000);
}

TEST(TestGeri, GbtFrameStruct)
{
    geri::payload_frame frame;
    ASSERT_EQ(frame.hits.capacity(), 1024 * 1024);
}
