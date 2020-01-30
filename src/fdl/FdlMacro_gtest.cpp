#include "gtest/gtest.h"

#include "FdlMacro.h"

TEST(MacroTest, Test)
{
    std::string const fdl =
        "(some unrelated stuff)\n"
        "(defmacro test-macro \"src/fdl/FdlMacro_gtest.scm\")\n"
        "(more unrelated things)\n"
        "(test-macro 3)\n"
        "(test-macro many)\n"
        "(blah blah blah)\n";

    S::PtrV ast = S::consume(fdl, "<test-stimulus>", 1);

    Fdl::expand_all_macros(ast);

    std::string expected =
        "("
        "(some unrelated stuff) "
        "(more unrelated things) "
        "(hello 3 worlds) "
        "(hello many worlds) "
        "(blah blah blah)"
        ")";

    S::List l;
    l.l = std::move(ast);
    EXPECT_EQ(expected, l.write());
}

int main(int argc, char ** argv)
{
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
