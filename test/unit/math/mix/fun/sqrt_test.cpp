
#include <stan/math/mix.hpp>
#include <gtest/gtest.h>
#include <test/unit/math/rev/fun/util.hpp>
#include <test/unit/math/mix/fun/nan_util.hpp>
#include <test/unit/math/prim/vectorize/prim_scalar_unary_test.hpp>
#include <test/unit/math/rev/vectorize/rev_scalar_unary_test.hpp>
#include <test/unit/math/fwd/vectorize/fwd_scalar_unary_test.hpp>
#include <test/unit/math/mix/vectorize/mix_scalar_unary_test.hpp>
#include <stan/math/prim/fun/sqrt.hpp>
#include <test/unit/math/prim/vectorize/vector_builder.hpp>

#include <vector>








TEST(AgradFwdSqrt, FvarVar_1stDeriv) {
  using stan::math::fvar;
  using stan::math::var;
  using std::sqrt;

  fvar<var> x(1.5, 1.3);
  fvar<var> a = sqrt(x);

  EXPECT_FLOAT_EQ(sqrt(1.5), a.val_.val());
  EXPECT_FLOAT_EQ(1.3 * 0.5 / sqrt(1.5), a.d_.val());

  AVEC y = createAVEC(x.val_);
  VEC g;
  a.val_.grad(y, g);
  EXPECT_FLOAT_EQ(0.5 / sqrt(1.5), g[0]);
}
TEST(AgradFwdSqrt, FvarVar_2ndDeriv) {
  using stan::math::fvar;
  using stan::math::var;
  using std::sqrt;

  fvar<var> x(1.5, 1.3);
  fvar<var> a = sqrt(x);

  AVEC y = createAVEC(x.val_);
  VEC g;
  a.d_.grad(y, g);
  EXPECT_FLOAT_EQ(0.5 * -0.5 * 1.3 / sqrt(1.5) / 1.5, g[0]);
}

TEST(AgradFwdSqrt, FvarFvarVar_1stDeriv) {
  using stan::math::fvar;
  using stan::math::var;
  using std::sqrt;

  fvar<fvar<var> > x;
  x.val_.val_ = 1.5;
  x.val_.d_ = 2.0;

  fvar<fvar<var> > a = sqrt(x);

  EXPECT_FLOAT_EQ(sqrt(1.5), a.val_.val_.val());
  EXPECT_FLOAT_EQ(2.0 * 0.5 / sqrt(1.5), a.val_.d_.val());
  EXPECT_FLOAT_EQ(0, a.d_.val_.val());
  EXPECT_FLOAT_EQ(0, a.d_.d_.val());

  AVEC p = createAVEC(x.val_.val_);
  VEC g;
  a.val_.val_.grad(p, g);
  EXPECT_FLOAT_EQ(0.5 / sqrt(1.5), g[0]);

  fvar<fvar<var> > y;
  y.val_.val_ = 1.5;
  y.d_.val_ = 2.0;

  fvar<fvar<var> > b = sqrt(y);
  EXPECT_FLOAT_EQ(sqrt(1.5), b.val_.val_.val());
  EXPECT_FLOAT_EQ(0, b.val_.d_.val());
  EXPECT_FLOAT_EQ(2.0 * 0.5 / sqrt(1.5), b.d_.val_.val());
  EXPECT_FLOAT_EQ(0, b.d_.d_.val());

  AVEC q = createAVEC(y.val_.val_);
  VEC r;
  b.val_.val_.grad(q, r);
  EXPECT_FLOAT_EQ(0.5 / sqrt(1.5), r[0]);
}
TEST(AgradFwdSqrt, FvarFvarVar_2ndDeriv) {
  using stan::math::fvar;
  using stan::math::var;
  using std::sqrt;

  fvar<fvar<var> > x;
  x.val_.val_ = 1.5;
  x.val_.d_ = 2.0;

  fvar<fvar<var> > a = sqrt(x);

  AVEC p = createAVEC(x.val_.val_);
  VEC g;
  a.val_.d_.grad(p, g);
  EXPECT_FLOAT_EQ(0.5 * 2.0 * -0.5 / 1.5 / sqrt(1.5), g[0]);

  fvar<fvar<var> > y;
  y.val_.val_ = 1.5;
  y.d_.val_ = 2.0;

  fvar<fvar<var> > b = sqrt(y);

  AVEC q = createAVEC(y.val_.val_);
  VEC r;
  b.d_.val_.grad(q, r);
  EXPECT_FLOAT_EQ(0.5 * -2.0 * 0.5 / 1.5 / sqrt(1.5), r[0]);
}
TEST(AgradFwdSqrt, FvarFvarVar_3rdDeriv) {
  using stan::math::fvar;
  using stan::math::var;
  using std::sqrt;

  fvar<fvar<var> > x;
  x.val_.val_ = 1.5;
  x.val_.d_ = 1.0;
  x.d_.val_ = 1.0;

  fvar<fvar<var> > a = sqrt(x);

  AVEC p = createAVEC(x.val_.val_);
  VEC g;
  a.d_.d_.grad(p, g);
  EXPECT_FLOAT_EQ(0.1360827634879543387887380041503, g[0]);
}

struct sqrt_fun {
  template <typename T0>
  inline T0 operator()(const T0& arg1) const {
    return sqrt(arg1);
  }
};

TEST(AgradFwdSqrt, sqrt_NaN) {
  sqrt_fun sqrt_;
  test_nan_mix(sqrt_, false);
}










/**
 * This is the structure for testing vectorized sqrt (defined in the
 * testing framework).
 */
struct sqrt_test {
  /**
   * Redefinition of function brought in from stan::math.  The reason
   * to do this is that it wraps it up in this static template class.
   *
   * This is the version that's being tested.
   *
   * WARNING:  assumes that the scalar values for all instantiations
   * (prim, rev, fwd, mix) ***have already been tested***.
   *
   * @tparam R Return type.
   * @tparam T Argument type.
   */
  template <typename R, typename T>
  static R apply(const T& x) {
    using stan::math::sqrt;
    return sqrt(x);
  }

  /**
   * This defines the truth against which we're testing.
   *
   * Because this is *not an independent test*, this function just
   * delegates to the actual function defined in stan::math.
   *
   * Redundant definition of function from stan::math to apply to an
   * integer and return a double.
   *
   * This function delegates to apply(), defined above, directly.
   *
   * WARNING:  this is *not an independent test*.
   */
  static double apply_base(int x) { return apply<double>(x); }

  /**
   * This is the generic version of the integer version defined
   * above.  For every other type, the return type is the same as the
   * reference type.
   *
   * WARNING:  this is *not an independent test of the underlying function*.
   */
  template <typename T>
  static T apply_base(const T& x) {
    return apply<T>(x);
  }

  /**
   * Return sequence of valid double-valued inputs.
   */
  static std::vector<double> valid_inputs() {
    return test::math::vector_builder<double>()
        .add(1.3)
        .add(10.7)
        .add(-5.2)
        .add(0)
        .build();
  }

  /**
   * Return sequence of invalid double-valued inputs.
   */
  static std::vector<double> invalid_inputs() { return std::vector<double>(); }

  /**
   * Return sequence of valid integer inputs.
   */
  static std::vector<int> int_valid_inputs() {
    return test::math::vector_builder<int>()
        .add(0)
        .add(-6)
        .add(7)
        .add(36)
        .build();
  }

  /**
   * Return sequence of invalid integer inputs.
   */
  static std::vector<int> int_invalid_inputs() { return std::vector<int>(); }
};

INSTANTIATE_TYPED_TEST_CASE_P(, prim_scalar_unary_test, sqrt_test);
INSTANTIATE_TYPED_TEST_CASE_P(, rev_scalar_unary_test, sqrt_test);
INSTANTIATE_TYPED_TEST_CASE_P(, fwd_scalar_unary_test, sqrt_test);
INSTANTIATE_TYPED_TEST_CASE_P(, mix_scalar_unary_test, sqrt_test);
