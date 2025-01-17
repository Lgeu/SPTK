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

#ifndef SPTK_CONVERSION_MEL_GENERALIZED_CEPSTRUM_TO_MEL_GENERALIZED_CEPSTRUM_H_
#define SPTK_CONVERSION_MEL_GENERALIZED_CEPSTRUM_TO_MEL_GENERALIZED_CEPSTRUM_H_

#include <vector>  // std::vector

#include "SPTK/math/frequency_transform.h"
#include "SPTK/utils/sptk_utils.h"

namespace sptk {

/**
 * Convert mel-generalized cepstrum to mel-generalized cepstrum.
 *
 * The input and output are in one of the following forms:
 *
 * (1) gain normalized + gamma multiplied
 * @f[
 *   \begin{array}{cccc}
 *     K_{\alpha,\gamma}, & \gamma c'_{\alpha,\gamma}(1), & \ldots, &
 *     \gamma c'_{\alpha,\gamma}(M),
 *   \end{array}
 * @f]
 * (2) gain normalized
 * @f[
 *   \begin{array}{cccc}
 *     K_{\alpha,\gamma}, & c'_{\alpha,\gamma}(1), & \ldots, &
 *     c'_{\alpha,\gamma}(M),
 *   \end{array}
 * @f]
 * (3) gain unnormalized + gamma multiplied
 * @f[
 *   \begin{array}{cccc}
 *     c_{\alpha,\gamma}(0), & \gamma c_{\alpha,\gamma}(1), & \ldots, &
 *     \gamma c_{\alpha,\gamma}(M),
 *   \end{array}
 * @f]
 * (4) gain unnormalized
 * @f[
 *   \begin{array}{cccc}
 *     c_{\alpha,\gamma}(0), & c_{\alpha,\gamma}(1), & \ldots, &
 *     c_{\alpha,\gamma}(M),
 *   \end{array}
 * @f]
 * where @f$M@f$ is the order of cepstrum. The conversion process is as follows:
 *
 * 1. Convert input to gain unnormalized form.
 * @f[
 *   \begin{array}{cccc}
 *     c_{\alpha_1,\gamma_1}(0), & c_{\alpha_1,\gamma_1}(1), & \ldots, &
 *     c_{\alpha_1,\gamma_1}(M_1),
 *   \end{array}
 * @f]
 * 2. Perform frequency transform.
 * @f[
 *   \begin{array}{cccc}
 *     c_{\alpha_2,\gamma_1}(0), & c_{\alpha_2,\gamma_1}(1), & \ldots, &
 *     c_{\alpha_2,\gamma_1}(M_2),
 *   \end{array}
 * @f]
 * 3. Perform gain normalization.
 * @f[
 *   \begin{array}{cccc}
 *     K_{\alpha_2,\gamma_1}, & c'_{\alpha_2,\gamma_1}(1), & \ldots, &
 *     c'_{\alpha_2,\gamma_1}(M_2),
 *   \end{array}
 * @f]
 * 4. Calculate
 * @f[
 *   \begin{array}{cccc}
 *     K_{\alpha_2,\gamma_2}, & c'_{\alpha_2,\gamma_2}(1), & \ldots, &
 *     c'_{\alpha_2,\gamma_2}(M_2),
 *   \end{array}
 * @f]
 * using the generalized log transformation:
 * @f[
 *   c'_{\alpha_2,\gamma_2}(m) =
 *     c'_{\alpha_2,\gamma_1}(m) + \sum_{k=1}^{m-1} \frac{k}{m} \left\{
 *       \gamma_2 c_{\alpha_2,\gamma_1}(k) \, c'_{\alpha_2,\gamma_2}(m-k)
 *       -\gamma_1 c'_{\alpha_2,\gamma_2}(k) \, c_{\alpha_2,\gamma_1}(m-k)
 *     \right\}.
 * @f]
 * 5. Convert the output to one of the four form.
 */
class MelGeneralizedCepstrumToMelGeneralizedCepstrum {
 public:
  /**
   * Buffer for MelGeneralizedCepstrumToMelGeneralizedCepstrum class.
   */
  class Buffer {
   public:
    Buffer() {
    }

    virtual ~Buffer() {
    }

   private:
    FrequencyTransform::Buffer frequency_transform_buffer_;
    std::vector<double> temporary_mel_generalized_cepstrum_;

    friend class MelGeneralizedCepstrumToMelGeneralizedCepstrum;
    DISALLOW_COPY_AND_ASSIGN(Buffer);
  };

  /**
   * Module interface.
   */
  class ModuleInterface {
   public:
    virtual ~ModuleInterface() {
    }

    /**
     * @return True if this object is valid.
     */
    virtual bool IsValid() const = 0;

    /**
     * @param[in] input Input.
     * @param[out] output Output.
     * @param[out] frequency_transform_buffer Buffer for frequency transform.
     * @return True on success, false on failure.
     */
    virtual bool Run(
        const std::vector<double>& input, std::vector<double>* output,
        FrequencyTransform::Buffer* frequency_transform_buffer) const = 0;
  };

  /**
   * @param[in] num_input_order Order of input, @f$M_1@f$.
   * @param[in] input_alpha Input alpha, @f$\alpha_1@f$.
   * @param[in] input_gamma Input gamma, @f$\gamma_1@f$.
   * @param[in] is_normalized_input Input gain-normalized flag.
   * @param[in] is_multiplied_input Input gamma-multiplied flag.
   * @param[in] num_output_order Order of output, @f$M_2@f$.
   * @param[in] output_alpha Output alpha, @f$\alpha_2@f$.
   * @param[in] output_gamma Output gamma, @f$\gamma_2@f$.
   * @param[in] is_normalized_output Output gain-normalized flag.
   * @param[in] is_multiplied_output Output gamma-multiplied flag.
   */
  MelGeneralizedCepstrumToMelGeneralizedCepstrum(
      int num_input_order, double input_alpha, double input_gamma,
      bool is_normalized_input, bool is_multiplied_input, int num_output_order,
      double output_alpha, double output_gamma, bool is_normalized_output,
      bool is_multiplied_output);

  virtual ~MelGeneralizedCepstrumToMelGeneralizedCepstrum() {
    for (std::vector<MelGeneralizedCepstrumToMelGeneralizedCepstrum::
                         ModuleInterface*>::iterator itr(modules_.begin());
         itr != modules_.end(); ++itr) {
      delete (*itr);
    }
  }

  /**
   * @return Order of input.
   */
  int GetNumInputOrder() const {
    return num_input_order_;
  }

  /**
   * @return Input alpha.
   */
  double GetInputAlpha() const {
    return input_alpha_;
  }

  /**
   * @return Input gamma.
   */
  double GetInputGamma() const {
    return input_gamma_;
  }

  /**
   * @return True if input is gain-normalized.
   */
  bool IsNormalizedInput() const {
    return is_normalized_input_;
  }

  /**
   * @return True if input is muliplied by gamma.
   */
  bool IsMultipliedInput() const {
    return is_multiplied_input_;
  }

  /**
   * @return Order of output.
   */
  int GetNumOutputOrder() const {
    return num_output_order_;
  }

  /**
   * @return Output alpha.
   */
  double GetOutputAlpha() const {
    return output_alpha_;
  }

  /**
   * @return Output gamma.
   */
  double GetOutputGamma() const {
    return output_gamma_;
  }

  /**
   * @return True if output is gain-normalized.
   */
  bool IsNormalizedOutput() const {
    return is_normalized_output_;
  }

  /**
   * @return True if output is muliplied by gamma.
   */
  bool IsMultipliedOutput() const {
    return is_multiplied_output_;
  }

  /**
   * @return True if this object is valid.
   */
  bool IsValid() const {
    return is_valid_;
  }

  /**
   * @param[in] input @f$M_1@f$-th order mel-generalized cepstrum.
   * @param[out] output @f$M_2@f$-th order mel-generalized cepstrum.
   * @param[out] buffer Buffer.
   * @return True on success, false on failure.
   */
  bool Run(
      const std::vector<double>& input, std::vector<double>* output,
      MelGeneralizedCepstrumToMelGeneralizedCepstrum::Buffer* buffer) const;

 private:
  const int num_input_order_;
  const double input_alpha_;
  const double input_gamma_;
  const bool is_normalized_input_;
  const bool is_multiplied_input_;

  const int num_output_order_;
  const double output_alpha_;
  const double output_gamma_;
  const bool is_normalized_output_;
  const bool is_multiplied_output_;

  double alpha_transform_;

  std::vector<MelGeneralizedCepstrumToMelGeneralizedCepstrum::ModuleInterface*>
      modules_;

  bool is_valid_;

  DISALLOW_COPY_AND_ASSIGN(MelGeneralizedCepstrumToMelGeneralizedCepstrum);
};

}  // namespace sptk

#endif  // SPTK_CONVERSION_MEL_GENERALIZED_CEPSTRUM_TO_MEL_GENERALIZED_CEPSTRUM_H_
