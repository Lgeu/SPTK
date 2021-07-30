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

#ifndef SPTK_UTILS_DATA_SYMMETRIZING_H_
#define SPTK_UTILS_DATA_SYMMETRIZING_H_

#include <vector>  // std::vector

#include "SPTK/utils/sptk_utils.h"

namespace sptk {

/**
 * Symmetrize/desymmetrize data sequence.
 *
 * The input and output takes one of the following forms.
 *
 * Type 0:
 * @f[
 *   \begin{array}{cccc}
 *     x(0), & x(1), & \ldots, & x(L/2).
 *   \end{array}
 * @f]
 * Type 1:
 * @f[
 *   \begin{array}{ccccccc}
 *     x(0), & x(1), & \ldots, & x(L/2), & x(L/2-1), & \ldots, & x(1).
 *   \end{array}
 * @f]
 * Type 2:
 * @f[
 *   \begin{array}{cccccccc}
 *     \frac{1}{2} x(L/2), & x(L/2-1), & \ldots, & x(0),
 *     & x(1), & \ldots, & x(L/2-1), & \frac{1}{2} x(L/2).
 *   \end{array}
 * @f]
 * Type 3:
 * @f[
 *   \begin{array}{cccccccc}
 *     x(L/2), & x(L/2-1), & \ldots, & x(0),
 *     & x(1), & \ldots, & x(L/2-1), & x(L/2),
 *   \end{array}
 * @f]
 * where @f$L@f$ must be even.
 */
class DataSymmetrizing {
 public:
  /**
   * Input and output format.
   */
  enum InputOutputFormats {
    kStandard = 0,
    kSymmetricForApplyingFourierTransform,
    kSymmetricForPreservingFrequencyResponse,
    kSymmetricForPlottingFrequencyResponse,
    kNumInputOutputFormats
  };

  /**
   * @param[in] fft_length FFT length, @f$L@f$.
   * @param[in] input_format Input format.
   * @param[in] output_format Output format.
   */
  DataSymmetrizing(int fft_length, InputOutputFormats input_format,
                   InputOutputFormats output_format);

  virtual ~DataSymmetrizing() {
  }

  /**
   * @return FFT length.
   */
  int GetFftLength() const {
    return fft_length_;
  }

  /**
   * @return Input format.
   */
  InputOutputFormats GetInputFormat() const {
    return input_format_;
  }

  /**
   * @return Output format.
   */
  InputOutputFormats GetOutputFormat() const {
    return output_format_;
  }

  /**
   * @return Expected input length.
   */
  int GetInputLength() const {
    return input_length_;
  }

  /**
   * @return Expected output length.
   */
  int GetOutputLength() const {
    return output_length_;
  }

  /**
   * @return True if this object is valid.
   */
  bool IsValid() const {
    return is_valid_;
  }

  /**
   * @param[in] data_sequence Input data.
   * @param[out] symmetrized_data_sequence Output data.
   * @return True on success, false on failure.
   */
  bool Run(const std::vector<double>& data_sequence,
           std::vector<double>* symmetrized_data_sequence) const;

 private:
  const int fft_length_;
  const InputOutputFormats input_format_;
  const InputOutputFormats output_format_;
  const int input_length_;
  const int output_length_;

  bool is_valid_;

  DISALLOW_COPY_AND_ASSIGN(DataSymmetrizing);
};

}  // namespace sptk

#endif  // SPTK_UTILS_DATA_SYMMETRIZING_H_
