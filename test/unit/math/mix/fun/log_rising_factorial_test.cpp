#include <test/unit/math/test_ad.hpp>
#include <limits>

TEST(mathMixScalFun, logRisingFactorial) {
  auto f = [](const auto& x1, const auto& x2) {
    return stan::math::log_rising_factorial(x1, x2);
  };
  stan::test::expect_ad(f, -3.0, -2);    // throws
  stan::test::expect_ad(f, -3.0, 2);     // throws
  stan::test::expect_ad(f, 1.0, -3.0);   // throws
  stan::test::expect_ad(f, -3.0, -3.0);  // throws

  stan::test::expect_ad(f, 1.0, 4.0);
  stan::test::expect_ad(f, 4.0, 1.0);
  stan::test::expect_ad(f, 4.0, 3.0);
  stan::test::expect_ad(f, 4.0, 5.0);
  stan::test::expect_ad(f, 5.0, 4.0);

  stan::test::expect_ad(f, 1, 4.0);
  stan::test::expect_ad(f, 1.0, 4);
  stan::test::expect_ad(f, 1, 4);

  double nan = std::numeric_limits<double>::quiet_NaN();
  stan::test::expect_ad(f, 2.0, nan);
  stan::test::expect_ad(f, nan, 4.0);
  stan::test::expect_ad(f, nan, nan);
}
