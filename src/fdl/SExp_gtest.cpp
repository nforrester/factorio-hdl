#include "gtest/gtest.h"

#include "SExp.h"

#include <fstream>

TEST(SExpTest, Empty)
{
    std::unique_ptr<S::Exp> e = S::read("", "<test-stimulus>", 1);
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
        std::unique_ptr<S::Exp> e = S::read(input, "<test-stimulus>", 1);
        ASSERT_TRUE(e);
        EXPECT_TRUE(e->as_symbol());
        EXPECT_EQ(input, e->as_symbol()->s);
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
        std::unique_ptr<S::Exp> e = S::read(input, "<test-stimulus>", 1);
        ASSERT_TRUE(e);
        EXPECT_TRUE(e->as_int());
        EXPECT_EQ(stoll(input, nullptr, 0), e->as_int()->v);
    }
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
        std::unique_ptr<S::Exp> e = S::read(input, "<test-stimulus>", 1);
        ASSERT_TRUE(e);
        EXPECT_TRUE(e->as_list());
    }
}

TEST(SExpTest, Whitespace)
{
    std::unique_ptr<S::Exp> e = S::read("0 ", "<test-stimulus>", 1);
    ASSERT_TRUE(e);
    EXPECT_TRUE(e->as_int());
    EXPECT_EQ(0, e->as_int()->v);

    e.reset();

    e = S::read(" 0", "<test-stimulus>", 1);
    ASSERT_TRUE(e);
    EXPECT_TRUE(e->as_int());
    EXPECT_EQ(0, e->as_int()->v);

    e.reset();

    e = S::read(" 0 ", "<test-stimulus>", 1);
    ASSERT_TRUE(e);
    EXPECT_TRUE(e->as_int());
    EXPECT_EQ(0, e->as_int()->v);
}

TEST(SExpTest, Comment)
{
    std::unique_ptr<S::Exp> e = S::read("; comment", "<test-stimulus>", 1);
    ASSERT_FALSE(e);

    e = S::read("; comment\n0", "<test-stimulus>", 1);
    ASSERT_TRUE(e);
    EXPECT_TRUE(e->as_int());
    EXPECT_EQ(0, e->as_int()->v);
}

TEST(SExpTest, Errors)
{
    std::vector<std::pair<std::string, std::string>> tests =
    {
        {"10000000000000000000000000000000000000000000000000000000000000000000000000000000", "<test-stimulus>:1: This integer is too long: \"10000000000000000000000000000000000000000000000000000000000000000000000000000000\"."},
        {"-10000000000000000000000000000000000000000000000000000000000000000000000000000000", "<test-stimulus>:1: This integer is too long: \"-10000000000000000000000000000000000000000000000000000000000000000000000000000000\"."},
        {"0x80000000", "<test-stimulus>:1: This integer is too long: \"0x80000000\"."},
        {"-0x80000001", "<test-stimulus>:1: This integer is too long: \"-0x80000001\"."},
        {"0x1x1", "<test-stimulus>:1: Not a valid integer: \"0x1x1\"."},
        {"-3f", "<test-stimulus>:1: Not a valid integer: \"-3f\"."},
        {"-3-", "<test-stimulus>:1: Character '-' is not valid in an int."},
        {"(", "<test-stimulus>:1: Unterminated list."},
        {"((())", "<test-stimulus>:1: Unterminated list."},
        {")", "<test-stimulus>:1: Expected int, symbol, or list. Found ')'."},
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

TEST(SExpTest, BigFile)
{
    std::string filename = "src/fdl/syntax-exploration.fdl";

    std::string buffer;
    {
        std::ifstream f(filename);
        ASSERT_TRUE(f.good());
        buffer = std::string { std::istreambuf_iterator<char>(f), std::istreambuf_iterator<char>() };
    }

    std::string_view input(buffer.data(), buffer.size());
    size_t line = 1;

    ASSERT_GT(input.size(), 0u);
    while (input.size() > 0)
    {
        std::unique_ptr<S::Exp> e = S::read(input, filename, line);
        if (input.size() > 0)
        {
            ASSERT_TRUE(e);
        }
        else
        {
            ASSERT_FALSE(e);
        }
    }

    ASSERT_LT(1u, line);
}

int main(int argc, char ** argv)
{
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
