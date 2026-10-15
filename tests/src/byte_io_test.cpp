/*
** EPITECH PROJECT, 2026
** r-type_tests
** File description:
** byte_io_test
*/

#include "common/protocol/ByteReader.hpp"
#include "common/protocol/ByteWriter.hpp"

#include <gtest/gtest.h>

#include <cstdint>
#include <vector>

using rtype::protocol::ByteReader;
using rtype::protocol::ByteWriter;
using rtype::protocol::MaxPacketSize;

namespace {

std::vector<std::uint8_t> bytesOf(const ByteWriter &writer)
{
    return {writer.data().begin(), writer.data().end()};
}

} // namespace

TEST(ByteWriter, WritesLittleEndian)
{
    ByteWriter writer;
    writer.u8(0xAB);
    writer.u16(0x1234);
    writer.u32(0xAABBCCDD);
    EXPECT_TRUE(writer.ok());
    EXPECT_EQ(bytesOf(writer), (std::vector<std::uint8_t>{
                                   0xAB, 0x34, 0x12, 0xDD, 0xCC, 0xBB, 0xAA}));
}

TEST(ByteWriter, WritesFloatsAsIeee754)
{
    ByteWriter writer;
    writer.f32(1.0F);
    EXPECT_EQ(bytesOf(writer),
              (std::vector<std::uint8_t>{0x00, 0x00, 0x80, 0x3F}));
}

TEST(ByteWriter, StopsAtMaxPacketSize)
{
    ByteWriter writer;
    for (std::size_t i = 0; i < MaxPacketSize / 2; ++i) {
        writer.u16(1);
    }
    EXPECT_TRUE(writer.ok());
    EXPECT_EQ(writer.size(), MaxPacketSize);
    writer.u8(1);
    EXPECT_FALSE(writer.ok());
    EXPECT_EQ(writer.size(), MaxPacketSize);
}

TEST(ByteWriter, StaysFailedAfterAWriteThatDidNotFit)
{
    ByteWriter writer;
    for (std::size_t i = 0; i < MaxPacketSize - 2; ++i) {
        writer.u8(0);
    }
    writer.u32(1); // does not fit
    writer.u8(1);  // would fit, but must not leave a hole
    EXPECT_FALSE(writer.ok());
    EXPECT_EQ(writer.size(), MaxPacketSize - 2);
}

TEST(ByteWriter, PatchesOnlyBytesAlreadyWritten)
{
    ByteWriter writer;
    writer.u16(0);
    writer.u8(0);
    writer.patchU16(0, 0x1234);
    EXPECT_TRUE(writer.ok());
    EXPECT_EQ(bytesOf(writer), (std::vector<std::uint8_t>{0x34, 0x12, 0x00}));
    writer.patchU16(2, 0x1234);
    EXPECT_FALSE(writer.ok());
}

TEST(ByteReader, ReadsLittleEndian)
{
    const std::vector<std::uint8_t> bytes{
        0xAB, 0x34, 0x12, 0xDD, 0xCC, 0xBB, 0xAA,
    };
    ByteReader reader(bytes);
    EXPECT_EQ(reader.u8(), 0xAB);
    EXPECT_EQ(reader.u16(), 0x1234);
    EXPECT_EQ(reader.u32(), 0xAABBCCDDU);
    EXPECT_TRUE(reader.ok());
    EXPECT_EQ(reader.remaining(), 0U);
}

TEST(ByteReader, ReadsWhatTheWriterWrote)
{
    ByteWriter writer;
    writer.f32(-12.5F);
    writer.f32(1920.0F);
    ByteReader reader(writer.data());
    EXPECT_EQ(reader.f32(), -12.5F);
    EXPECT_EQ(reader.f32(), 1920.0F);
    EXPECT_TRUE(reader.ok());
}

TEST(ByteReader, FailsForGoodPastTheEnd)
{
    const std::vector<std::uint8_t> bytes{0x01, 0x02, 0x03};
    ByteReader reader(bytes);
    EXPECT_EQ(reader.u16(), 0x0201);
    EXPECT_EQ(reader.u16(), 0); // only one byte left
    EXPECT_FALSE(reader.ok());
    EXPECT_EQ(reader.u8(), 0); // that byte stays unread
    EXPECT_FALSE(reader.ok());
}

TEST(ByteReader, EmptyInput)
{
    ByteReader reader({});
    EXPECT_EQ(reader.remaining(), 0U);
    EXPECT_EQ(reader.u8(), 0);
    EXPECT_FALSE(reader.ok());
}
