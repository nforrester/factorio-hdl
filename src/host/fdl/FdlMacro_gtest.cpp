#include "gtest/gtest.h"

#include "FdlMacro.h"

TEST(MacroTest, Test)
{
    std::string const fdl =
        "(some unrelated stuff)\n"
        "(defmacro test-macro \"src/host/fdl/FdlMacro_gtest.scm\")\n"
        "(defmacro test-macro2 \"src/host/fdl/FdlMacro_gtest.scm\")\n"
        "(more unrelated things)\n"
        "(test-macro 3)\n"
        "(test-macro many)\n"
        "(begin (x) (y))\n"
        "(test-macro2)\n"
        "(blah blah blah)\n";

    S::PtrV ast = S::consume(fdl, "<test-stimulus>", 1);

    Fdl::expand_all_macros(ast, "fake.scm");

    std::string expected =
        "("
        "(some unrelated stuff) "
        "(more unrelated things) "
        "(hello 3 worlds) "
        "(hello many worlds) "
        "(x) "
        "(y) "
        "(stuff) "
        "(more-stuff) "
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
