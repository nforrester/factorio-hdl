#include "gtest/gtest.h"

#include "JsonDigester.h"
#include "src/host/util.h"

#include "json.hpp"

using json = nlohmann::json;

TEST(JsonDigesterTest, EndToEnd)
{
    struct S
    {
        int x;
        std::string y;
        double z = std::numeric_limits<double>::quiet_NaN();
        std::vector<S> v;

        S(json const & j)
        {
            JsonDigester<S> d;
            d.require("x", [](json const & j, S & s){ s.x = j; });
            d.require("y", [](json const & j, S & s){ s.y = j; });
            d.optional("z", [](json const & j, S & s){ s.z = j; });
            d.optional("v", [](json const & j, S & s){
                    assert(j.is_array());
                    for (json const & jj : j)
                    {
                        s.v.emplace_back(jj);
                    }
                });
            d.digest(j, *this);
        }
    };

    json j1 = R"(
            {
                "x": 4,
                "y": "hello"
            }
        )"_json;
    S s1(j1);
    EXPECT_EQ(4, s1.x);
    EXPECT_EQ("hello", s1.y);
    EXPECT_TRUE(std::isnan(s1.z));
    EXPECT_TRUE(s1.v.empty());

    json j2 = R"(
            {
                "v": [{"x": 2, "y": "no"}],
                "x": 5,
                "z": 3.3,
                "y": "yes"
            }
        )"_json;
    S s2(j2);
    EXPECT_EQ(5, s2.x);
    EXPECT_EQ("yes", s2.y);
    EXPECT_EQ(3.3, s2.z);
    EXPECT_EQ(1u, s2.v.size());
    EXPECT_EQ(2, s2.v.front().x);
    EXPECT_EQ("no", s2.v.front().y);

    json j3 = R"(
            {
                "x": 4,
                "y": "hello",
                "a": 1
            }
        )"_json;
    try
    {
        S s3(j3);
        ASSERT_TRUE(false);
    }
    catch (std::runtime_error & e)
    {
        EXPECT_EQ(std::string("Unhandled key \"a\""), e.what());
    }
}

int main(int argc, char ** argv)
{
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
