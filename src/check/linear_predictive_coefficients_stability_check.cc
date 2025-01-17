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

#include "SPTK/check/linear_predictive_coefficients_stability_check.h"

#include <cmath>    // std::fabs
#include <cstddef>  // std::size_t

namespace {

const double kMinimumMargin(1e-16);

}  // namespace

namespace sptk {

LinearPredictiveCoefficientsStabilityCheck::
    LinearPredictiveCoefficientsStabilityCheck(int num_order, double margin)
    : num_order_(num_order),
      margin_(margin),
      linear_predictive_coefficients_to_parcor_coefficients_(num_order_, 1.0),
      parcor_coefficients_to_linear_predictive_coefficients_(num_order_),
      is_valid_(true) {
  if (num_order_ < 0 || margin_ < kMinimumMargin || 1.0 <= margin_ ||
      !linear_predictive_coefficients_to_parcor_coefficients_.IsValid() ||
      !parcor_coefficients_to_linear_predictive_coefficients_.IsValid()) {
    is_valid_ = false;
    return;
  }
}

bool LinearPredictiveCoefficientsStabilityCheck::Run(
    const std::vector<double>& linear_predictive_coefficients,
    std::vector<double>* modified_linear_predictive_coefficients,
    bool* is_stable,
    LinearPredictiveCoefficientsStabilityCheck::Buffer* buffer) const {
  // Check inputs.
  if (!is_valid_ ||
      linear_predictive_coefficients.size() !=
          static_cast<std::size_t>(num_order_ + 1) ||
      NULL == is_stable || NULL == buffer) {
    return false;
  }

  // Prepare memories.
  if (NULL != modified_linear_predictive_coefficients &&
      modified_linear_predictive_coefficients->size() !=
          static_cast<std::size_t>(num_order_ + 1)) {
    modified_linear_predictive_coefficients->resize(num_order_ + 1);
  }

  // Handle a special case.
  *is_stable = true;
  if (0 == num_order_) {
    if (NULL != modified_linear_predictive_coefficients) {
      (*modified_linear_predictive_coefficients)[0] =
          linear_predictive_coefficients[0];
    }
    return true;
  }

  // Check stability.
  if (!linear_predictive_coefficients_to_parcor_coefficients_.Run(
          linear_predictive_coefficients, &buffer->parcor_coefficients_,
          is_stable, &buffer->conversion_buffer_)) {
    return false;
  }

  // Perform modification.
  if (NULL != modified_linear_predictive_coefficients) {
    double* parcor_coefficients(&buffer->parcor_coefficients_[0]);
    const double bound(1.0 - margin_);
    for (int m(1); m <= num_order_; ++m) {
      if (bound < std::fabs(parcor_coefficients[m])) {
        parcor_coefficients[m] =
            (0.0 < parcor_coefficients[m]) ? bound : -bound;
      }
    }
    if (!parcor_coefficients_to_linear_predictive_coefficients_.Run(
            buffer->parcor_coefficients_,
            modified_linear_predictive_coefficients,
            &buffer->reconversion_buffer_)) {
      return false;
    }
  }

  return true;
}

bool LinearPredictiveCoefficientsStabilityCheck::Run(
    std::vector<double>* input_and_output, bool* is_stable,
    LinearPredictiveCoefficientsStabilityCheck::Buffer* buffer) const {
  if (NULL == input_and_output) return false;
  return Run(*input_and_output, input_and_output, is_stable, buffer);
}

}  // namespace sptk
