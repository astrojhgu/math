#ifndef STAN_MATH_PRIM_PROB_BETA_BINOMIAL_CDF_HPP
#define STAN_MATH_PRIM_PROB_BETA_BINOMIAL_CDF_HPP

#include <stan/math/prim/meta.hpp>
#include <stan/math/prim/err.hpp>
#include <stan/math/prim/fun/beta.hpp>
#include <stan/math/prim/fun/constants.hpp>
#include <stan/math/prim/fun/digamma.hpp>
#include <stan/math/prim/fun/F32.hpp>
#include <stan/math/prim/fun/grad_F32.hpp>
#include <stan/math/prim/fun/lgamma.hpp>
#include <stan/math/prim/fun/max_size.hpp>
#include <stan/math/prim/fun/size.hpp>
#include <stan/math/prim/fun/size_zero.hpp>
#include <stan/math/prim/fun/value_of.hpp>
#include <cmath>

namespace stan {
namespace math {

/** \ingroup prob_dists
 * Returns the CDF of the Beta-Binomial distribution with given population
 * size, prior success, and prior failure parameters. Given containers of
 * matching sizes, returns the product of probabilities.
 *
 * @tparam T_n type of success parameter
 * @tparam T_N type of population size parameter
 * @tparam T_size1 type of prior success parameter
 * @tparam T_size2 type of prior failure parameter
 *
 * @param n success parameter
 * @param N population size parameter
 * @param alpha prior success parameter
 * @param beta prior failure parameter
 * @return probability or product of probabilities
 * @throw std::domain_error if N, alpha, or beta fails to be positive
 * @throw std::invalid_argument if container sizes mismatch
 */
template <typename T_n, typename T_N, typename T_size1, typename T_size2>
return_type_t<T_size1, T_size2> beta_binomial_cdf(const T_n& n, const T_N& N,
                                                  const T_size1& alpha,
                                                  const T_size2& beta) {
  static const char* function = "beta_binomial_cdf";
  using T_partials_return = partials_return_t<T_n, T_N, T_size1, T_size2>;

  if (size_zero(n, N, alpha, beta)) {
    return 1.0;
  }

  T_partials_return P(1.0);

  check_nonnegative(function, "Population size parameter", N);
  check_positive_finite(function, "First prior sample size parameter", alpha);
  check_positive_finite(function, "Second prior sample size parameter", beta);
  check_consistent_sizes(function, "Successes variable", n,
                         "Population size parameter", N,
                         "First prior sample size parameter", alpha,
                         "Second prior sample size parameter", beta);

  scalar_seq_view<T_n> n_vec(n);
  scalar_seq_view<T_N> N_vec(N);
  scalar_seq_view<T_size1> alpha_vec(alpha);
  scalar_seq_view<T_size2> beta_vec(beta);
  size_t max_size_seq_view = max_size(n, N, alpha, beta);

  using std::exp;

  operands_and_partials<T_size1, T_size2> ops_partials(alpha, beta);

  // Explicit return for extreme values
  // The gradients are technically ill-defined, but treated as zero
  for (size_t i = 0; i < size(n); i++) {
    if (value_of(n_vec[i]) < 0) {
      return ops_partials.build(0.0);
    }
  }

  for (size_t i = 0; i < max_size_seq_view; i++) {
    // Explicit results for extreme values
    // The gradients are technically ill-defined, but treated as zero
    if (value_of(n_vec[i]) >= value_of(N_vec[i])) {
      continue;
    }

    const T_partials_return n_dbl = value_of(n_vec[i]);
    const T_partials_return N_dbl = value_of(N_vec[i]);
    const T_partials_return alpha_dbl = value_of(alpha_vec[i]);
    const T_partials_return beta_dbl = value_of(beta_vec[i]);

    const T_partials_return mu = alpha_dbl + n_dbl + 1;
    const T_partials_return nu = beta_dbl + N_dbl - n_dbl - 1;

    const T_partials_return F
        = F32((T_partials_return)1, mu, -N_dbl + n_dbl + 1, n_dbl + 2, 1 - nu,
              (T_partials_return)1);

    T_partials_return C = lgamma(nu) - lgamma(N_dbl - n_dbl);
    C += lgamma(mu) - lgamma(n_dbl + 2);
    C += lgamma(N_dbl + 2) - lgamma(N_dbl + alpha_dbl + beta_dbl);
    C = exp(C);

    C *= F / stan::math::beta(alpha_dbl, beta_dbl);
    C /= N_dbl + 1;

    const T_partials_return Pi = 1 - C;

    P *= Pi;

    T_partials_return dF[6];
    T_partials_return digammaOne = 0;
    T_partials_return digammaTwo = 0;

    if (!is_constant_all<T_size1, T_size2>::value) {
      digammaOne = digamma(mu + nu);
      digammaTwo = digamma(alpha_dbl + beta_dbl);
      grad_F32(dF, (T_partials_return)1, mu, -N_dbl + n_dbl + 1, n_dbl + 2,
               1 - nu, (T_partials_return)1);
    }
    if (!is_constant_all<T_size1>::value) {
      const T_partials_return g = -C
                                  * (digamma(mu) - digammaOne + dF[1] / F
                                     - digamma(alpha_dbl) + digammaTwo);
      ops_partials.edge1_.partials_[i] += g / Pi;
    }
    if (!is_constant_all<T_size2>::value) {
      const T_partials_return g = -C
                                  * (digamma(nu) - digammaOne - dF[4] / F
                                     - digamma(beta_dbl) + digammaTwo);
      ops_partials.edge2_.partials_[i] += g / Pi;
    }
  }

  if (!is_constant_all<T_size1>::value) {
    for (size_t i = 0; i < size(alpha); ++i) {
      ops_partials.edge1_.partials_[i] *= P;
    }
  }
  if (!is_constant_all<T_size2>::value) {
    for (size_t i = 0; i < size(beta); ++i) {
      ops_partials.edge2_.partials_[i] *= P;
    }
  }

  return ops_partials.build(P);
}

}  // namespace math
}  // namespace stan
#endif
