#include <gtest/gtest.h>
#include <Calculator.hpp>
#include <cmath>
#include <StatementException.hpp>

TEST(Parsing, MinusAfter)
{
    Calculator calc;
    calc.addBasicFunctions();

    ASSERT_NO_THROW(calc.setExpression("2- 1"));
    ASSERT_DOUBLE_EQ(calc.execute(), 1);
}

TEST(Basic, Basic)
{
    Calculator calc;
    calc.addBasicFunctions();

    ASSERT_NO_THROW(calc.setExpression("atan2(0.5 0.2) + sin(0.2) / 2"));
    ASSERT_DOUBLE_EQ(calc.execute(), std::atan2(0.5, 0.2) + sin(0.2) / 2);
}

TEST(Basic, UnariMinus)
{
    Calculator calc;
    calc.addBasicFunctions();

    ASSERT_NO_THROW(calc.setExpression("-21 - -2"));
    ASSERT_DOUBLE_EQ(calc.execute(), -21 - - 2);
}

TEST(Basic, UnariPlus)
{
    Calculator calc;
    calc.addBasicFunctions();

    ASSERT_NO_THROW(calc.setExpression("12 + +2"));
    ASSERT_DOUBLE_EQ(calc.execute(), 12 + +2);
}

TEST(Errors, UnbalancedBraces1)
{
    Calculator calc;
    calc.addBasicFunctions();

    ASSERT_THROW(
        calc.setExpression("12 + 2 - (22 + 2"),
        StatementException
    );
}

TEST(Errors, UnbalancedBraces2)
{
    Calculator calc;
    calc.addBasicFunctions();

    ASSERT_THROW(
        calc.setExpression(") + 12 * 22 * sin(12)"),
        StatementException
    );
}

TEST(Errors, UnbalancedStatement)
{
    Calculator calc;
    calc.addBasicFunctions();

    ASSERT_NO_THROW(calc.setExpression("22 33 + 2"));
    ASSERT_THROW(
        calc.execute(),
        StatementException
    );
}

TEST(Variables, Basic)
{
    Calculator calc;
    calc.addBasicFunctions();

    ASSERT_NO_THROW(calc.setExpression("12 + 2 -x + y * z"));

    for (int ix = 0; ix < 100; ++ix)
    {
        calc.setVariable("x", ix);

        for (int iy = 0; iy < 100; ++iy)
        {
            calc.setVariable("y", iy);

            for (int iz = 0; iz < 100; ++iz)
            {
                calc.setVariable("z", iz);

                ASSERT_DOUBLE_EQ(calc.execute(), 12 + 2 -ix + iy * iz);
            }
        }
    }
}

int main(int argc, char** argv)
{
    testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}