// ----------------------------------------------------------------- //
//             The Speech Signal Processing Toolkit (SPTK)           //
//             developed by SPTK Working Group                       //
//             http://sp-tk.sourceforge.net/                         //
// ----------------------------------------------------------------- //
//                                                                   //
//  Copyright (c) 1984-2007  Tokyo Institute of Technology           //
//                           Interdisciplinary Graduate School of    //
//                           Science and Engineering                 //
//                                                                   //
//                1996-2021  Nagoya Institute of Technology          //
//                           Department of Computer Science          //
//                                                                   //
// All rights reserved.                                              //
//                                                                   //
// Redistribution and use in source and binary forms, with or        //
// without modification, are permitted provided that the following   //
// conditions are met:                                               //
//                                                                   //
// - Redistributions of source code must retain the above copyright  //
//   notice, this list of conditions and the following disclaimer.   //
// - Redistributions in binary form must reproduce the above         //
//   copyright notice, this list of conditions and the following     //
//   disclaimer in the documentation and/or other materials provided //
//   with the distribution.                                          //
// - Neither the name of the SPTK working group nor the names of its //
//   contributors may be used to endorse or promote products derived //
//   from this software without specific prior written permission.   //
//                                                                   //
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND            //
// CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,       //
// INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF          //
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE          //
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS //
// BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,          //
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED   //
// TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,     //
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON //
// ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,   //
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY    //
// OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE           //
// POSSIBILITY OF SUCH DAMAGE.                                       //
// ----------------------------------------------------------------- //

#include "SPTK/analysis/adaptive_generalized_cepstral_analysis.h"

#include <algorithm>  // std::fill
#include <cmath>      // std::sqrt
#include <cstddef>    // std::size_t

namespace sptk {

AdaptiveGeneralizedCepstralAnalysis::AdaptiveGeneralizedCepstralAnalysis(
    int num_order, int num_stage, double min_epsilon, double momentum,
    double forgetting_factor, double step_size_factor)
    : num_stage_(num_stage),
      min_epsilon_(min_epsilon),
      momentum_(momentum),
      forgetting_factor_(forgetting_factor),
      step_size_factor_(step_size_factor),
      generalized_cepstrum_inverse_gain_normalization_(num_order,
                                                       -1.0 / num_stage_),
      is_valid_(true) {
  if (num_stage_ <= 0 || min_epsilon_ <= 0.0 || momentum_ < 0.0 ||
      1.0 <= momentum_ || forgetting_factor_ < 0.0 ||
      1.0 <= forgetting_factor_ || step_size_factor_ <= 0.0 ||
      1.0 <= step_size_factor_ ||
      !generalized_cepstrum_inverse_gain_normalization_.IsValid()) {
    is_valid_ = false;
    return;
  }
}

bool AdaptiveGeneralizedCepstralAnalysis::Run(
    double input_signal, double* prediction_error,
    std::vector<double>* generalized_cepstrum,
    AdaptiveGeneralizedCepstralAnalysis::Buffer* buffer) const {
  // Check inputs.
  if (!is_valid_ || NULL == prediction_error || NULL == generalized_cepstrum ||
      NULL == buffer) {
    return false;
  }

  // Prepare memories.
  const int num_order(GetNumOrder());
  if (buffer->normalized_generalized_cepstrum_.size() !=
      static_cast<std::size_t>(num_order + 1)) {
    buffer->normalized_generalized_cepstrum_.resize(num_order + 1);
    std::fill(buffer->normalized_generalized_cepstrum_.begin(),
              buffer->normalized_generalized_cepstrum_.end(), 0.0);
  }
  if (buffer->d_.size() != static_cast<std::size_t>(num_order * num_stage_)) {
    buffer->d_.resize(num_order * num_stage_);
    std::fill(buffer->d_.begin(), buffer->d_.end(), 0.0);
  }
  if (buffer->gradient_.size() != static_cast<std::size_t>(num_order)) {
    buffer->gradient_.resize(num_order);
    std::fill(buffer->gradient_.begin(), buffer->gradient_.end(), 0.0);
  }

  // Store e_\gamma(n - M).
  const double last_e(buffer->d_.back());

  // Apply cascade all-zero digital filters.
  {
    const double gamma(GetGamma());
    double x(input_signal);
    for (int i(0); i < num_stage_; ++i) {
      const double* c(&buffer->normalized_generalized_cepstrum_[1]);
      double* d(&buffer->d_[num_order * i]);
      double y(0.0);
      for (int j(num_order - 1); 0 < j; --j) {
        y += c[j] * d[j];
        d[j] = d[j - 1];
      }
      y += c[0] * d[0];
      d[0] = x;
      x += y * gamma;
    }
    *prediction_error = x;
  }

  // Update epsilon.
  const double e_gamma(buffer->d_[num_order * (num_stage_ - 1)]);
  double curr_epsilon((forgetting_factor_ * buffer->prev_epsilon_) +
                      (1.0 - forgetting_factor_) * (e_gamma * e_gamma));
  if (curr_epsilon < min_epsilon_) {
    curr_epsilon = min_epsilon_;
  }

  // Update normalized generalized cepstrum.
  if (0 < num_order) {
    const double sigma(2.0 * (1.0 - momentum_) * *prediction_error);
    const double mu(step_size_factor_ / (num_order * curr_epsilon));
    const double* e(&buffer->d_[num_order * (num_stage_ - 1) + 1]);
    double* gradient(&buffer->gradient_[0]);
    double* c(&buffer->normalized_generalized_cepstrum_[1]);
    for (int i(0); i < num_order; ++i) {
      const double f(i == num_order - 1 ? last_e : e[i]);
      gradient[i] = momentum_ * gradient[i] - sigma * f;
      c[i] -= mu * gradient[i];
    }
  }

  // Update gain.
  const double curr_adjusted_error(
      (forgetting_factor_ * buffer->prev_adjusted_error_) +
      (1.0 - forgetting_factor_) * (*prediction_error * *prediction_error));
  buffer->normalized_generalized_cepstrum_[0] = std::sqrt(curr_adjusted_error);

  // Store outputs.
  buffer->prev_adjusted_error_ = curr_adjusted_error;
  buffer->prev_epsilon_ = curr_epsilon;

  if (!generalized_cepstrum_inverse_gain_normalization_.Run(
          buffer->normalized_generalized_cepstrum_, generalized_cepstrum)) {
    return false;
  }

  return true;
}

}  // namespace sptk
