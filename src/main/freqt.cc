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

#include <getopt.h>  // getopt_long

#include <fstream>   // std::ifstream
#include <iomanip>   // std::setw
#include <iostream>  // std::cerr, std::cin, std::cout, std::endl, etc.
#include <sstream>   // std::ostringstream
#include <vector>    // std::vector

#include "SPTK/math/frequency_transform.h"
#include "SPTK/utils/sptk_utils.h"

namespace {

const int kDefaultNumInputOrder(25);
const int kDefaultNumOutputOrder(25);
const double kDefaultInputAlpha(0.0);
const double kDefaultOutputAlpha(0.35);

void PrintUsage(std::ostream* stream) {
  // clang-format off
  *stream << std::endl;
  *stream << " freqt - frequency transform" << std::endl;
  *stream << std::endl;
  *stream << "  usage:" << std::endl;
  *stream << "       freqt [ options ] [ infile ] > stdout" << std::endl;
  *stream << "  options:" << std::endl;
  *stream << "       -m m  : order of minimum phase sequence      (   int)[" << std::setw(5) << std::right << kDefaultNumInputOrder  << "][    0 <= m <=     ]" << std::endl;  // NOLINT
  *stream << "       -M M  : order of warped sequence             (   int)[" << std::setw(5) << std::right << kDefaultNumOutputOrder << "][    0 <= M <=     ]" << std::endl;  // NOLINT
  *stream << "       -a a  : all-pass constant of input sequence  (double)[" << std::setw(5) << std::right << kDefaultInputAlpha     << "][ -1.0 <  a <  1.0 ]" << std::endl;  // NOLINT
  *stream << "       -A A  : all-pass constant of output sequence (double)[" << std::setw(5) << std::right << kDefaultOutputAlpha    << "][ -1.0 <  A <  1.0 ]" << std::endl;  // NOLINT
  *stream << "       -h    : print this message" << std::endl;
  *stream << "  infile:" << std::endl;
  *stream << "       minimum phase sequence                       (double)[stdin]" << std::endl;  // NOLINT
  *stream << "  stdout:" << std::endl;
  *stream << "       warped sequence                              (double)" << std::endl;  // NOLINT
  *stream << std::endl;
  *stream << " SPTK: version " << sptk::kVersion << std::endl;
  *stream << std::endl;
  // clang-format on
}

}  // namespace

/**
 * @a freqt [ @e option ] [ @e infile ]
 *
 * - @b -m @e int
 *   - order of minimum phase sequence @f$(0 \le M_1)@f$
 * - @b -M @e int
 *   - order of warped sequence @f$(0 \le M_2)@f$
 * - @b -a @e double
 *   - all-pass constant of input sequence @f$(|\alpha_1|<1)@f$
 * - @b -A @e double
 *   - all-pass constant of output sequence @f$(|\alpha_2|<1)@f$
 * - @b infile @e str
 *   - double-type minimum phase sequence
 * - @b stdout
 *   - double-type warped sequence
 *
 * The below example converts LPC coefficients into LPC mel-cepstral
 * coefficients:
 *
 * @code{.sh}
 *   lpc2c < data.lpc | freqt -A 0.42 > data.lpcmc
 * @endcode
 *
 * The converted LPC mel-cepstral coefficients can be reverted if @f$M_2@f$
 * is sufficiently greater than @f$M_1@f$:
 *
 * @code{.sh}
 *   freqt -A -0.42 < data.lpcmc > data.lpc
 * @endcode
 *
 * @param[in] argc Number of arguments.
 * @param[in] argv Argument vector.
 * @return 0 on success, 1 on failure.
 */
int main(int argc, char* argv[]) {
  int num_input_order(kDefaultNumInputOrder);
  int num_output_order(kDefaultNumOutputOrder);
  double input_alpha(kDefaultInputAlpha);
  double output_alpha(kDefaultOutputAlpha);

  for (;;) {
    const int option_char(getopt_long(argc, argv, "m:M:a:A:h", NULL, NULL));
    if (-1 == option_char) break;

    switch (option_char) {
      case 'm': {
        if (!sptk::ConvertStringToInteger(optarg, &num_input_order) ||
            num_input_order < 0) {
          std::ostringstream error_message;
          error_message << "The argument for the -m option must be a "
                        << "non-negative integer";
          sptk::PrintErrorMessage("freqt", error_message);
          return 1;
        }
        break;
      }
      case 'M': {
        if (!sptk::ConvertStringToInteger(optarg, &num_output_order) ||
            num_output_order < 0) {
          std::ostringstream error_message;
          error_message << "The argument for the -M option must be a "
                        << "non-negative integer";
          sptk::PrintErrorMessage("freqt", error_message);
          return 1;
        }
        break;
      }
      case 'a': {
        if (!sptk::ConvertStringToDouble(optarg, &input_alpha) ||
            !sptk::IsValidAlpha(input_alpha)) {
          std::ostringstream error_message;
          error_message
              << "The argument for the -a option must be in (-1.0, 1.0)";
          sptk::PrintErrorMessage("freqt", error_message);
          return 1;
        }
        break;
      }
      case 'A': {
        if (!sptk::ConvertStringToDouble(optarg, &output_alpha) ||
            !sptk::IsValidAlpha(output_alpha)) {
          std::ostringstream error_message;
          error_message
              << "The argument for the -A option must be in (-1.0, 1.0)";
          sptk::PrintErrorMessage("freqt", error_message);
          return 1;
        }
        break;
      }
      case 'h': {
        PrintUsage(&std::cout);
        return 0;
      }
      default: {
        PrintUsage(&std::cerr);
        return 1;
      }
    }
  }

  const int num_input_files(argc - optind);
  if (1 < num_input_files) {
    std::ostringstream error_message;
    error_message << "Too many input files";
    sptk::PrintErrorMessage("freqt", error_message);
    return 1;
  }
  const char* input_file(0 == num_input_files ? NULL : argv[optind]);

  std::ifstream ifs;
  ifs.open(input_file, std::ios::in | std::ios::binary);
  if (ifs.fail() && NULL != input_file) {
    std::ostringstream error_message;
    error_message << "Cannot open file " << input_file;
    sptk::PrintErrorMessage("freqt", error_message);
    return 1;
  }
  std::istream& input_stream(ifs.fail() ? std::cin : ifs);

  const double alpha((output_alpha - input_alpha) /
                     (1.0 - input_alpha * output_alpha));
  sptk::FrequencyTransform frequency_transform(num_input_order,
                                               num_output_order, alpha);
  sptk::FrequencyTransform::Buffer buffer;
  if (!frequency_transform.IsValid()) {
    std::ostringstream error_message;
    error_message << "Failed to initialize FrequencyTransform";
    sptk::PrintErrorMessage("freqt", error_message);
    return 1;
  }

  const int input_length(num_input_order + 1);
  const int output_length(num_output_order + 1);
  std::vector<double> minimum_phase_sequence(input_length);
  std::vector<double> warped_sequence(output_length);

  while (sptk::ReadStream(false, 0, 0, input_length, &minimum_phase_sequence,
                          &input_stream, NULL)) {
    if (!frequency_transform.Run(minimum_phase_sequence, &warped_sequence,
                                 &buffer)) {
      std::ostringstream error_message;
      error_message << "Failed to run frequency transform";
      sptk::PrintErrorMessage("freqt", error_message);
      return 1;
    }

    if (!sptk::WriteStream(0, output_length, warped_sequence, &std::cout,
                           NULL)) {
      std::ostringstream error_message;
      error_message << "Failed to write warped sequence";
      sptk::PrintErrorMessage("freqt", error_message);
      return 1;
    }
  }

  return 0;
}
