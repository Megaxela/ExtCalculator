#include <gtest/gtest.h>
#include <Calculator.hpp>
#include <cmath>
#include <StatementException.hpp>
#include <ParsingException.hpp>

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

    ASSERT_NO_THROW(calc.setExpression("atan2(0.5, 0.2) + sin(0.2) / 2"));
    ASSERT_DOUBLE_EQ(calc.execute(), std::atan2(0.5, 0.2) + sin(0.2) / 2);
}

TEST(Basic, MultipleBraces)
{
    Calculator calc;
    calc.addBasicFunctions();

    ASSERT_NO_THROW(calc.setExpression("(1/(12+1)+2/(13+2)+3/(14+3))"));
    ASSERT_DOUBLE_EQ(calc.execute(), (1.0/(12+1)+2.0/(13+2)+3.0/(14+3)));
}

TEST(Basic, CompressedCalculation)
{
    Calculator calc;
    calc.addBasicFunctions();

    ASSERT_NO_THROW(calc.setExpression("-2-2"));
    ASSERT_DOUBLE_EQ(calc.execute(), -2-2);
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

TEST(Basic, Factorial)
{
    Calculator calc;
    calc.addBasicFunctions();

    ASSERT_NO_THROW(calc.setExpression("5!"));
    ASSERT_DOUBLE_EQ(calc.execute(), 120);

    ASSERT_NO_THROW(calc.setExpression("5.2!"));
    ASSERT_DOUBLE_EQ(calc.execute(), 169.406099461722999);
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

TEST(Errors, UnbalancedBraces3)
{
    Calculator calc;
    calc.addBasicFunctions();

    ASSERT_THROW(
        calc.setExpression("12 * 22 * sin(12) + {12 + 2]"),
        StatementException
    );
}

TEST(Errors, UnbalancedStatement)
{
    Calculator calc;
    calc.addBasicFunctions();

    ASSERT_THROW(
        calc.setExpression("22 33 + 2"),
        ParsingException
    );
}

TEST(Errors, WrongArgumentsNumber)
{
    Calculator calc;
    calc.addBasicFunctions();

    ASSERT_THROW(
        calc.setExpression("sin(22, 11)"),
        StatementException
    );

    ASSERT_THROW(
        calc.setExpression("atan2(11)"),
        StatementException
    );
}

TEST(Errors, WrongArgumentsNumber2)
{
    Calculator calc;
    calc.addBasicFunctions();

    ASSERT_THROW(
        calc.setExpression("(12 22)!"),
        ParsingException
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

TEST(Logic, Comparison)
{
    Calculator calc;
    calc.addLogicFunctions();

    // <
    ASSERT_NO_THROW(calc.setExpression("1 < 2"));
    ASSERT_DOUBLE_EQ(calc.execute(), 1);

    ASSERT_NO_THROW(calc.setExpression("2 < 1"));
    ASSERT_DOUBLE_EQ(calc.execute(), 0);

    // >
    ASSERT_NO_THROW(calc.setExpression("2 > 1"));
    ASSERT_DOUBLE_EQ(calc.execute(), 1);

    ASSERT_NO_THROW(calc.setExpression("1 > 2"));
    ASSERT_DOUBLE_EQ(calc.execute(), 0);

    // >=
    ASSERT_NO_THROW(calc.setExpression("1 >= 2"));
    ASSERT_DOUBLE_EQ(calc.execute(), 0);

    ASSERT_NO_THROW(calc.setExpression("2 >= 2"));
    ASSERT_DOUBLE_EQ(calc.execute(), 1);

    ASSERT_NO_THROW(calc.setExpression("2 >= 1"));
    ASSERT_DOUBLE_EQ(calc.execute(), 1);

    // <=
    ASSERT_NO_THROW(calc.setExpression("2 <= 1"));
    ASSERT_DOUBLE_EQ(calc.execute(), 0);

    ASSERT_NO_THROW(calc.setExpression("2 <= 2"));
    ASSERT_DOUBLE_EQ(calc.execute(), 1);

    ASSERT_NO_THROW(calc.setExpression("1 <= 2"));
    ASSERT_DOUBLE_EQ(calc.execute(), 1);

    // ==
    ASSERT_NO_THROW(calc.setExpression("1 == 2"));
    ASSERT_DOUBLE_EQ(calc.execute(), 0);

    ASSERT_NO_THROW(calc.setExpression("1 == 1"));
    ASSERT_DOUBLE_EQ(calc.execute(), 1);

    // !=
    ASSERT_NO_THROW(calc.setExpression("1 != 2"));
    ASSERT_DOUBLE_EQ(calc.execute(), 1);

    ASSERT_NO_THROW(calc.setExpression("1 != 1"));
    ASSERT_DOUBLE_EQ(calc.execute(), 0);

    // if
    ASSERT_NO_THROW(calc.setExpression("if (1) {12} {13}"));
    ASSERT_DOUBLE_EQ(calc.execute(), 12);

    ASSERT_NO_THROW(calc.setExpression("if (0) {12} {13}"));
    ASSERT_DOUBLE_EQ(calc.execute(), 13);
}

TEST(Constants, Base)
{
    Calculator calc;
    calc.addConstants();

    ASSERT_NO_THROW(calc.setExpression("Pi / 2"));
    ASSERT_DOUBLE_EQ(calc.execute(), M_PI / 2);

    ASSERT_NO_THROW(calc.setExpression("e"));
    ASSERT_DOUBLE_EQ(calc.execute(), M_E);
}

int main(int argc, char** argv)
{
    testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}