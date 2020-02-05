#include "gtest/gtest.h"

#include "Blueprint.h"
#include "src/util.h"
#include "json.hpp"

using json = nlohmann::json;

TEST(BlueprintTest, BackAndForth)
{
    std::string const blueprint_string = "0eNqlVNFu2zAM/Bc+FnZR2Wm7GXvZ2r8oCkOxmYaALRkyHTQI9O+jpCLNGidp1xcDEsnj8XjWDpbdhIMjw1DtgBprRqiedjDSi9FduOPtgFABMfaQgdF9OGlHvO6Rqckb2y/JaLYOfAZkWnyFSvnsIkaLDbXo5gEK/5wBGiYmTIziYVubqV+ikw4XuGQw2FGqrQkEBPHm+jaDLVS5cIOWHDYpWGQgU7OzXb3Etd6QFEvFO2ot4TYijSGwIjdyfTTbhhxPcrOnlTLy32GoEQPG54v+hCI7oNOJI1xJip14mL7Q+QG893E4k2aN9FX4vDhEc6gqtVFySS9OxkvBItdMxPGoJN2HNX9YTLHnEczE2vDZteTqfStze1hRx+hOmPKs6I2dgqvz2wuuPLuENxD149CZX1FVfU618txvcSRa0kwc/a+VF/MSviF+z8cPSY20UekdDv2gXSRYwa//MOhjghy2dVS5Xjnb12QEA6qV7kb03xD6wM0O24/JixNrWOw5krMmb9Y48sxTEuVXJ9jNtCuPufnwwMUHsTp4gzPYiNljp/vyRv28K1R5f+f9XynO7c0=";

    std::string const raw_json = blueprint_string_to_raw_json(blueprint_string);

    std::string const expected =
        "{" "\"blueprint" "\":{" "\"icons" "\":[{" "\"signal" "\":{" "\"type" "\":" "\"item" "\","
        "\"name" "\":" "\"arithmetic-combinator" "\"}," "\"index" "\":1},{" "\"signal" "\":{"
        "\"type" "\":" "\"item" "\"," "\"name" "\":" "\"decider-combinator" "\"}," "\"index"
        "\":2}]," "\"entities" "\":[{" "\"entity_number" "\":1," "\"name" "\":"
        "\"arithmetic-combinator" "\"," "\"position" "\":{" "\"x" "\":0.5," "\"y" "\":-1},"
        "\"direction" "\":2," "\"control_behavior" "\":{" "\"arithmetic_conditions" "\":{"
        "\"first_signal" "\":{" "\"type" "\":" "\"virtual" "\"," "\"name" "\":" "\"signal-A" "\"},"
        "\"second_signal" "\":{" "\"type" "\":" "\"virtual" "\"," "\"name" "\":" "\"signal-B" "\"},"
        "\"operation" "\":" "\"*" "\"," "\"output_signal" "\":{" "\"type" "\":" "\"virtual" "\","
        "\"name" "\":" "\"signal-C" "\"}}}," "\"connections" "\":{" "\"1" "\":{" "\"green" "\":[{"
        "\"entity_id" "\":2}]}," "\"2" "\":{" "\"green" "\":[{" "\"entity_id" "\":3," "\"circuit_id"
        "\":1}]}}},{" "\"entity_number" "\":2," "\"name" "\":" "\"constant-combinator" "\","
        "\"position" "\":{" "\"x" "\":-1," "\"y" "\":-1}," "\"control_behavior" "\":{" "\"filters"
        "\":[{" "\"signal" "\":{" "\"type" "\":" "\"virtual" "\"," "\"name" "\":" "\"signal-A" "\"},"
        "\"count" "\":-5," "\"index" "\":1},{" "\"signal" "\":{" "\"type" "\":" "\"virtual" "\","
        "\"name" "\":" "\"signal-B" "\"}," "\"count" "\":18," "\"index" "\":2}]}," "\"connections"
        "\":{" "\"1" "\":{" "\"green" "\":[{" "\"entity_id" "\":1," "\"circuit_id" "\":1}]}}},{"
        "\"entity_number" "\":3," "\"name" "\":" "\"decider-combinator" "\"," "\"position" "\":{"
        "\"x" "\":1," "\"y" "\":0.5}," "\"direction" "\":4," "\"control_behavior" "\":{"
        "\"decider_conditions" "\":{" "\"first_signal" "\":{" "\"type" "\":" "\"virtual" "\","
        "\"name" "\":" "\"signal-C" "\"}," "\"constant" "\":0," "\"comparator" "\":" "\"<" "\","
        "\"output_signal" "\":{" "\"type" "\":" "\"virtual" "\"," "\"name" "\":" "\"signal-D"
        "\"}," "\"copy_count_from_input" "\":false}}," "\"connections" "\":{" "\"1" "\":{" "\"green"
        "\":[{" "\"entity_id" "\":1," "\"circuit_id" "\":2}]}," "\"2" "\":{" "\"red" "\":[{"
        "\"entity_id" "\":4}]}}},{" "\"entity_number" "\":4," "\"name" "\":" "\"iron-chest" "\","
        "\"position" "\":{" "\"x" "\":0," "\"y" "\":1}," "\"connections" "\":{" "\"1" "\":{" "\"red"
        "\":[{" "\"entity_id" "\":3," "\"circuit_id" "\":2}]}}}]," "\"item" "\":" "\"blueprint" "\","
        "\"version" "\":73019621376}}";

    ASSERT_EQ(expected, raw_json);

    std::string const blueprint_string2 = raw_json_to_blueprint_string(raw_json);

    ASSERT_EQ(blueprint_string, blueprint_string2);

    std::string raw_json2 = json::parse(raw_json).dump();
    ASSERT_NE(raw_json, raw_json2);

    debug(0) << json::parse(raw_json).dump(4) << "\n";
}
TEST(BlueprintTest, Parse)
{
    std::string const blueprint_string = "0eNqlVNFu2zAM/Bc+FnZR2Wm7GXvZ2r8oCkOxmYaALRkyHTQI9O+jpCLNGidp1xcDEsnj8XjWDpbdhIMjw1DtgBprRqiedjDSi9FduOPtgFABMfaQgdF9OGlHvO6Rqckb2y/JaLYOfAZkWnyFSvnsIkaLDbXo5gEK/5wBGiYmTIziYVubqV+ikw4XuGQw2FGqrQkEBPHm+jaDLVS5cIOWHDYpWGQgU7OzXb3Etd6QFEvFO2ot4TYijSGwIjdyfTTbhhxPcrOnlTLy32GoEQPG54v+hCI7oNOJI1xJip14mL7Q+QG893E4k2aN9FX4vDhEc6gqtVFySS9OxkvBItdMxPGoJN2HNX9YTLHnEczE2vDZteTqfStze1hRx+hOmPKs6I2dgqvz2wuuPLuENxD149CZX1FVfU618txvcSRa0kwc/a+VF/MSviF+z8cPSY20UekdDv2gXSRYwa//MOhjghy2dVS5Xjnb12QEA6qV7kb03xD6wM0O24/JixNrWOw5krMmb9Y48sxTEuVXJ9jNtCuPufnwwMUHsTp4gzPYiNljp/vyRv28K1R5f+f9XynO7c0=";

    Blueprint::Blueprint blueprint(blueprint_string);
    debug(0) << blueprint.to_json().dump(4) << "\n";
    Blueprint::Blueprint check(blueprint.to_blueprint_string());
    debug(0) << "Regenerated: " << blueprint.to_blueprint_string() << "\n";
    // TODO EXPECT_TRUE(check == blueprint);
}

int main(int argc, char ** argv)
{
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
