#include "gtest/gtest.h"

#include "SExp.h"
#include "src/host/util.h"

TEST(SExpTest, Empty)
{
    S::Ptr e = S::read("", "<test-stimulus>", 1);
    ASSERT_FALSE(e);
}

TEST(SExpTest, Symbols)
{
    std::vector<std::string> symbols =
    {
        "hello",
        "hello-world",
        "hello_world",
        "<",
        ">",
        "<=",
        ">=",
        "==",
        "!=",
        "+",
        "-",
        "*",
        "/",
        "%",
        "**",
        "<<",
        ">>",
        "&",
        "|",
        "^",
    };

    for (std::string const & input : symbols)
    {
        S::Ptr e = S::read(input, "<test-stimulus>", 1);
        ASSERT_TRUE(e);
        EXPECT_TRUE(e->as_symbol());
        EXPECT_EQ(input, e->as_symbol()->s);

        EXPECT_NE("", e->file);
        EXPECT_NE(0u, e->line);
    }
}

TEST(SExpTest, Ints)
{
    std::vector<std::string> ints =
    {
        "0",
        "1",
        "-1",
        "42",
        "-42",
        "1234567890",
        "-1234567890",
        "012345670",
        "-012345670",
        "0x12345678",
        "0xabcdef",
        "0XABCDEF",
        "-0x12345678",
        "-0xabcdef",
        "-0XABCDEF",
        "0x7fffffff",
        "-0x80000000",
    };

    for (std::string const & input : ints)
    {
        S::Ptr e = S::read(input, "<test-stimulus>", 1);
        ASSERT_TRUE(e);
        EXPECT_TRUE(e->as_int());
        EXPECT_EQ(stoll(input, nullptr, 0), e->as_int()->v);

        EXPECT_NE("", e->file);
        EXPECT_NE(0u, e->line);
    }
}

TEST(SExpTest, UnsignedInts)
{
    std::vector<std::string> ints =
    {
        "0u",
        "1u",
        "42u",
        "1234567890u",
        "012345670u",
        "0x12345678u",
        "0xabcdefu",
        "0XABCDEFu",
        "0x7fffffffu",
        "0xffffffffu",
    };

    for (std::string const & input : ints)
    {
        S::Ptr e = S::read(input, "<test-stimulus>", 1);
        ASSERT_TRUE(e);
        EXPECT_TRUE(e->as_int());
        EXPECT_EQ(stoull(input, nullptr, 0), static_cast<uint32_t>(e->as_int()->v));

        EXPECT_NE("", e->file);
        EXPECT_NE(0u, e->line);
    }
}

TEST(SExpTest, Strings)
{
    std::vector<std::string> strings =
    {
        "hello",
        "Hello world!",
        "some_file.txt",
        "()",
        "(())",
        "((()))",
        "(((()) ()))",
        "( (( ( )) () ) )",
        "(hello )",
        "( hello)",
        "( hello )",
        "(42 hello)",
        "(42 hello (world))",
    };

    for (std::string const & input : strings)
    {
        S::Ptr e = S::read("\"" + input + "\"", "<test-stimulus>", 1);
        ASSERT_TRUE(e);
        EXPECT_TRUE(e->as_string());
        EXPECT_EQ(input, e->as_string()->s);

        EXPECT_NE("", e->file);
        EXPECT_NE(0u, e->line);
    }

    std::string input = "\"This is a\\nstring with\\tvarious escaped\\\" characters\\\\.\"";
    std::string expected = "This is a\nstring with\tvarious escaped\" characters\\.";
    S::Ptr e = S::read(input, "<test-stimulus>", 1);
    ASSERT_TRUE(e);
    EXPECT_TRUE(e->as_string());
    EXPECT_EQ(expected, e->as_string()->s);

    EXPECT_NE("", e->file);
    EXPECT_NE(0u, e->line);
}

TEST(SExpTest, Lists)
{
    std::vector<std::string> lists =
    {
        "()",
        "(())",
        "((()))",
        "(((()) ()))",
        "( (( ( )) () ) )",
        "(hello )",
        "( hello)",
        "( hello )",
        "(42 hello)",
        "(42 hello (world))",
    };

    for (std::string const & input : lists)
    {
        S::Ptr e = S::read(input, "<test-stimulus>", 1);
        ASSERT_TRUE(e);
        EXPECT_TRUE(e->as_list());

        EXPECT_NE("", e->file);
        EXPECT_NE(0u, e->line);
    }
}

TEST(SExpTest, Whitespace)
{
    S::Ptr e = S::read("0 ", "<test-stimulus>", 1);
    ASSERT_TRUE(e);
    EXPECT_TRUE(e->as_int());
    EXPECT_EQ(0, e->as_int()->v);

    EXPECT_NE("", e->file);
    EXPECT_NE(0u, e->line);

    e.reset();

    e = S::read(" 0", "<test-stimulus>", 1);
    ASSERT_TRUE(e);
    EXPECT_TRUE(e->as_int());
    EXPECT_EQ(0, e->as_int()->v);

    EXPECT_NE("", e->file);
    EXPECT_NE(0u, e->line);

    e.reset();

    e = S::read(" 0 ", "<test-stimulus>", 1);
    ASSERT_TRUE(e);
    EXPECT_TRUE(e->as_int());
    EXPECT_EQ(0, e->as_int()->v);

    EXPECT_NE("", e->file);
    EXPECT_NE(0u, e->line);
}

TEST(SExpTest, Comment)
{
    S::Ptr e = S::read("; comment", "<test-stimulus>", 1);
    ASSERT_FALSE(e);

    e = S::read("; comment\n0", "<test-stimulus>", 1);
    ASSERT_TRUE(e);
    EXPECT_TRUE(e->as_int());
    EXPECT_EQ(0, e->as_int()->v);

    EXPECT_NE("", e->file);
    EXPECT_NE(0u, e->line);
}

TEST(SExpTest, Errors)
{
    std::vector<std::pair<std::string, std::string>> tests =
    {
        {"10000000000000000000000000000000000000000000000000000000000000000000000000000000", "<test-stimulus>:1: This integer is out of range: \"10000000000000000000000000000000000000000000000000000000000000000000000000000000\"."},
        {"-10000000000000000000000000000000000000000000000000000000000000000000000000000000", "<test-stimulus>:1: This integer is out of range: \"-10000000000000000000000000000000000000000000000000000000000000000000000000000000\"."},
        {"0x80000000", "<test-stimulus>:1: This integer is out of range: \"0x80000000\"."},
        {"-0x80000001", "<test-stimulus>:1: This integer is out of range: \"-0x80000001\"."},
        {"10000000000000000000000000000000000000000000000000000000000000000000000000000000u", "<test-stimulus>:1: This integer is out of range: \"10000000000000000000000000000000000000000000000000000000000000000000000000000000u\"."},
        {"0x100000000u", "<test-stimulus>:1: This integer is out of range: \"0x100000000u\"."},
        {"-0x1u", "<test-stimulus>:1: This integer is out of range: \"-0x1u\"."},
        {"0x1x1", "<test-stimulus>:1: Not a valid integer: \"0x1x1\"."},
        {"-3f", "<test-stimulus>:1: Not a valid integer: \"-3f\"."},
        {"-3-", "<test-stimulus>:1: Character '-' is not valid in an int."},
        {"(", "<test-stimulus>:1: Unterminated list."},
        {"((())", "<test-stimulus>:1: Unterminated list."},
        {")", "<test-stimulus>:1: Expected int, symbol, string, or list. Found ')'."},
        {"3()", "<test-stimulus>:1: Character '(' is not valid in an int."},
        {"f()", "<test-stimulus>:1: Character '(' is not valid in a symbol."},
    };

    for (auto const & t : tests)
    {
        try
        {
            S::read(t.first, "<test-stimulus>", 1);
            ASSERT_TRUE(false);
        }
        catch (S::ParseError & e)
        {
            EXPECT_EQ("<test-stimulus>", e.file);
            EXPECT_EQ(1u, e.line);
            EXPECT_EQ(t.second, e.what()) << "Stimulus: \"" << t.first << "\"";
        }
    }
}

int main(int argc, char ** argv)
{
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
