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

#include "SPTK/conversion/all_pole_to_all_zero_digital_filter_coefficients.h"

#include <algorithm>  // std::transform
#include <cstddef>    // std::size_t

namespace sptk {

AllPoleToAllZeroDigitalFilterCoefficients::
    AllPoleToAllZeroDigitalFilterCoefficients(int num_order)
    : num_order_(num_order), is_valid_(true) {
  if (num_order_ < 0) {
    is_valid_ = false;
    return;
  }
}

bool AllPoleToAllZeroDigitalFilterCoefficients::Run(
    const std::vector<double>& input_filter_coefficients,
    std::vector<double>* output_filter_coefficients) const {
  // Check inputs.
  const int length(num_order_ + 1);
  if (!is_valid_ ||
      input_filter_coefficients.size() != static_cast<std::size_t>(length) ||
      0.0 == input_filter_coefficients[0] ||
      NULL == output_filter_coefficients) {
    return false;
  }

  // Prepare memories.
  if (output_filter_coefficients->size() != static_cast<std::size_t>(length)) {
    output_filter_coefficients->resize(length);
  }

  // Perform conversion.
  const double z(1.0 / input_filter_coefficients[0]);
  (*output_filter_coefficients)[0] = z;
  std::transform(
      input_filter_coefficients.begin() + 1, input_filter_coefficients.end(),
      output_filter_coefficients->begin() + 1, [z](double x) { return x * z; });

  return true;
}

bool AllPoleToAllZeroDigitalFilterCoefficients::Run(
    std::vector<double>* input_and_output) const {
  if (NULL == input_and_output) return false;
  std::vector<double> input(*input_and_output);
  return Run(input, input_and_output);
}

}  // namespace sptk
