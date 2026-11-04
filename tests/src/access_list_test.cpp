/*
** EPITECH PROJECT, 2026
** r-type_tests
** File description:
** access_list_test
*/

#include "AccessList.hpp"
#include "engine/network/Endpoint.hpp"

#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <optional>
#include <stdexcept>
#include <string>

using rtype::protocol::RejectReason;
using server::AccessList;

namespace {

constexpr std::uint32_t Address = 0x0A000001; // 10.0.0.1
constexpr std::uint32_t Other = 0x0A000002;   // 10.0.0.2

// A file removed at the end of the test.
class TempFile {
  public:
    explicit TempFile(const std::string &name)
        : m_path(std::filesystem::temp_directory_path() / name)
    {
        std::filesystem::remove(m_path);
    }
    TempFile(const TempFile &) = delete;
    TempFile &operator=(const TempFile &) = delete;
    TempFile(TempFile &&) = delete;
    TempFile &operator=(TempFile &&) = delete;
    ~TempFile() { std::filesystem::remove(m_path); }

    [[nodiscard]] std::string path() const { return m_path.string(); }

    void write(const std::string &text) const { std::ofstream(m_path) << text; }

  private:
    std::filesystem::path m_path;
};

} // namespace

TEST(Endpoint, ParsesAndFormatsAnAddress)
{
    EXPECT_EQ(engine::net::parseAddress("10.0.0.1"), Address);
    EXPECT_EQ(engine::net::addressToString(Address), "10.0.0.1");
}

TEST(Endpoint, RefusesInvalidAddresses)
{
    for (const char *text :
         {"", "10.0.0", "10.0.0.1.2", "256.0.0.1", "a.b.c.d", "10.0.0.1 "}) {
        EXPECT_EQ(engine::net::parseAddress(text), std::nullopt) << text;
    }
}

TEST(AccessList, ByDefaultEveryoneMayConnect)
{
    EXPECT_EQ(AccessList{}.check(Address), std::nullopt);
}

TEST(AccessList, ABannedAddressIsRefused)
{
    AccessList access;
    access.ban(Address);

    EXPECT_EQ(access.check(Address), RejectReason::Banned);
    EXPECT_EQ(access.check(Other), std::nullopt);
    EXPECT_TRUE(access.unban(Address));
    EXPECT_FALSE(access.unban(Address));
    EXPECT_EQ(access.check(Address), std::nullopt);
}

TEST(AccessList, TheWhitelistOnlyAcceptsListedAddresses)
{
    AccessList access;
    access.allow(Address);
    EXPECT_EQ(access.check(Other), std::nullopt);

    access.setWhitelistEnabled(true);

    EXPECT_EQ(access.check(Address), std::nullopt);
    EXPECT_EQ(access.check(Other), RejectReason::NotWhitelisted);
}

TEST(AccessList, ABanWinsOverTheWhitelist)
{
    AccessList access;
    access.setWhitelistEnabled(true);
    access.allow(Address);
    access.ban(Address);

    EXPECT_EQ(access.check(Address), RejectReason::Banned);
}

TEST(AccessList, ReadsItsFilesAndKeepsThemUpToDate)
{
    const TempFile bans("rtype_test_bans.txt");
    const TempFile allowed("rtype_test_allowed.txt");
    bans.write("# comment\n10.0.0.1  # bad player\n\n");

    AccessList access(bans.path(), allowed.path());

    EXPECT_EQ(access.check(Address), RejectReason::Banned);
    EXPECT_TRUE(access.whitelistEnabled());
    access.ban(Other);
    const AccessList reloaded(bans.path(), "");
    EXPECT_EQ(reloaded.bans().size(), 2U);
    EXPECT_FALSE(reloaded.whitelistEnabled());
}

TEST(AccessList, RefusesAnInvalidFile)
{
    const TempFile bans("rtype_test_invalid_bans.txt");
    bans.write("not an address\n");

    EXPECT_THROW(AccessList(bans.path(), ""), std::runtime_error);
}
