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

#include <cmath>     // std::cos, std::sin
#include <iomanip>   // std::setw
#include <iostream>  // std::cerr, std::cout, std::endl, etc.
#include <sstream>   // std::ostringstream

#include "SPTK/utils/sptk_utils.h"

namespace {

const int kMagicNumberForInfinity(-1);
const double kDefaultPeriod(10.0);
const double kDefaultAmplitude(1.0);
const bool kDefaultCosineWaveFlag(false);

void PrintUsage(std::ostream* stream) {
  // clang-format off
  *stream << std::endl;
  *stream << " sin - generate sinusoidal sequence" << std::endl;
  *stream << std::endl;
  *stream << "  usage:" << std::endl;
  *stream << "       sin [ options ] > stdout" << std::endl;
  *stream << "  options:" << std::endl;
  *stream << "       -l l  : output length      (   int)[" << std::setw(5) << std::right << "INF"             << "][   1 <= l <=   ]" << std::endl;  // NOLINT
  *stream << "       -m m  : output order       (   int)[" << std::setw(5) << std::right << "l-1"             << "][   0 <= m <=   ]" << std::endl;  // NOLINT
  *stream << "       -p p  : period             (double)[" << std::setw(5) << std::right << kDefaultPeriod    << "][ 0.0 <  p <=   ]" << std::endl;  // NOLINT
  *stream << "       -a a  : amplitude          (double)[" << std::setw(5) << std::right << kDefaultAmplitude << "][     <= a <=   ]" << std::endl;  // NOLINT
  *stream << "       -C    : cosine wave        (  bool)[" << std::setw(5) << std::right << sptk::ConvertBooleanToString(kDefaultCosineWaveFlag) << "]" << std::endl;  // NOLINT
  *stream << "       -h    : print this message" << std::endl;
  *stream << "  stdout:" << std::endl;
  *stream << "       sinusoidal sequence        (double)" << std::endl;
  *stream << std::endl;
  *stream << " SPTK: version " << sptk::kVersion << std::endl;
  *stream << std::endl;
  // clang-format on
}

}  // namespace

/**
 * @a sin [ @e option ]
 *
 * - @b -l @e int
 *   - output length @f$(1 \le L)@f$
 * - @b -m @e int
 *   - output order @f$(0 \le L - 1)@f$
 * - @b -p @e double
 *   - period @f$(0 < P)@f$
 * - @b -a @e double
 *   - amplitude @f$(A)@f$
 * - @b -C @e bool
 *   - generate cosine wave
 * - @b stdout
 *   - double-type sinusoidal sequence
 *
 * The output of this command is
 * @f[
 *   \begin{array}{cccc}
 *     x(0), & x(1), & \ldots, & x(L-1),
 *   \end{array}
 * @f]
 * where
 * @f[
 *   x(l) = A \sin \left( \frac{2\pi l}{P} \right).
 * @f]
 * If @f$L@f$ is not given, an infinite sinusoidal sequence is generated.
 *
 * @param[in] argc Number of arguments.
 * @param[in] argv Argument vector.
 * @return 0 on success, 1 on failure.
 */
int main(int argc, char* argv[]) {
  int output_length(kMagicNumberForInfinity);
  double period(kDefaultPeriod);
  double amplitude(kDefaultAmplitude);
  bool cosine_wave(kDefaultCosineWaveFlag);

  for (;;) {
    const int option_char(getopt_long(argc, argv, "l:m:p:a:Ch", NULL, NULL));
    if (-1 == option_char) break;

    switch (option_char) {
      case 'l': {
        if (!sptk::ConvertStringToInteger(optarg, &output_length) ||
            output_length <= 0) {
          std::ostringstream error_message;
          error_message
              << "The argument for the -l option must be a positive integer";
          sptk::PrintErrorMessage("sin", error_message);
          return 1;
        }
        break;
      }
      case 'm': {
        if (!sptk::ConvertStringToInteger(optarg, &output_length) ||
            output_length < 0) {
          std::ostringstream error_message;
          error_message << "The argument for the -m option must be a "
                           "non-negative integer";
          sptk::PrintErrorMessage("sin", error_message);
          return 1;
        }
        ++output_length;
        break;
      }
      case 'p': {
        if (!sptk::ConvertStringToDouble(optarg, &period) || period <= 0.0) {
          std::ostringstream error_message;
          error_message
              << "The argument for the -p option must be a positive number";
          sptk::PrintErrorMessage("sin", error_message);
          return 1;
        }
        break;
      }
      case 'a': {
        if (!sptk::ConvertStringToDouble(optarg, &amplitude)) {
          std::ostringstream error_message;
          error_message << "The argument for the -a option must be numeric";
          sptk::PrintErrorMessage("sin", error_message);
          return 1;
        }
        break;
      }
      case 'C': {
        cosine_wave = true;
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

  if (0 != argc - optind) {
    std::ostringstream error_message;
    error_message << "Input file is not required";
    sptk::PrintErrorMessage("sin", error_message);
    return 1;
  }

  const double omega(sptk::kTwoPi / period);
  for (int i(0); kMagicNumberForInfinity == output_length || i < output_length;
       ++i) {
    const double output(cosine_wave ? amplitude * std::cos(omega * i)
                                    : amplitude * std::sin(omega * i));
    if (!sptk::WriteStream(output, &std::cout)) {
      std::ostringstream error_message;
      error_message << "Failed to write sinusoidal sequence";
      sptk::PrintErrorMessage("sin", error_message);
      return 1;
    }
  }

  return 0;
}
