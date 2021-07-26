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
//                1996-2020  Nagoya Institute of Technology          //
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

#include "SPTK/compression/uniform_quantization.h"

#include <cmath>  // std::floor, std::pow, std::round

namespace sptk {

UniformQuantization::UniformQuantization(double absolute_maximum_value,
                                         int num_bit,
                                         QuantizationType quantization_type)
    : absolute_maximum_value_(absolute_maximum_value),
      num_bit_(num_bit),
      quantization_type_(quantization_type),
      is_valid_(true) {
  if (absolute_maximum_value_ <= 0.0 || num_bit_ <= 0) {
    is_valid_ = false;
    return;
  }

  switch (quantization_type_) {
    case kMidRise: {
      quantization_levels_ = static_cast<int>(std::pow(2.0, num_bit_));
      break;
    }
    case kMidTread: {
      quantization_levels_ = static_cast<int>(std::pow(2.0, num_bit_)) - 1;
      break;
    }
    default: {
      is_valid_ = false;
      return;
    }
  }

  inverse_step_size_ = quantization_levels_ / (2.0 * absolute_maximum_value_);
}

bool UniformQuantization::Run(double input, int* output) const {
  // Check inputs.
  if (!is_valid_ || NULL == output) {
    return false;
  }

  int index;
  switch (quantization_type_) {
    case kMidRise: {
      index = static_cast<int>(std::floor(input * inverse_step_size_) +
                               quantization_levels_ / 2);
      break;
    }
    case kMidTread: {
      index = static_cast<int>(std::round(input * inverse_step_size_) +
                               (quantization_levels_ - 1) / 2);
      break;
    }
    default: { return false; }
  }

  // Clip index.
  if (index < 0) {
    index = 0;
  } else if (quantization_levels_ <= index) {
    index = quantization_levels_ - 1;
  }

  *output = index;

  return true;
}

}  // namespace sptk
