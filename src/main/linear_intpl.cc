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

#include <cfloat>    // DBL_MAX
#include <fstream>   // std::ifstream
#include <iomanip>   // std::setw
#include <iostream>  // std::cerr, std::cin, std::cout, std::endl, etc.
#include <sstream>   // std::ostringstream
#include <vector>    // std::vector

#include "SPTK/utils/sptk_utils.h"

namespace {

const int kDefaultOutputLength(256);

void PrintUsage(std::ostream* stream) {
  // clang-format off
  *stream << std::endl;
  *stream << " linear_intpl - linear interpolation of data" << std::endl;
  *stream << std::endl;
  *stream << "  usage:" << std::endl;
  *stream << "       linear_intpl [ options ] [ infile ] > stdout" << std::endl;
  *stream << "  options:" << std::endl;
  *stream << "       -l l  : output length             (   int)[" << std::setw(5) << std::right << kDefaultOutputLength << "][ 1 <= l <=   ]" << std::endl;  // NOLINT
  *stream << "       -m m  : output order              (   int)[" << std::setw(5) << std::right << "l-1"                << "][ 0 <= m <=   ]" << std::endl;  // NOLINT
  *stream << "       -s s  : minimum value of x-axis   (double)[" << std::setw(5) << std::right << "BOS"                << "][   <= s <  e ]" << std::endl;  // NOLINT
  *stream << "       -e e  : maximum value of x-axis   (double)[" << std::setw(5) << std::right << "EOS"                << "][ s <  e <=   ]" << std::endl;  // NOLINT
  *stream << "       -h    : print this message" << std::endl;
  *stream << "  infile:" << std::endl;
  *stream << "       2-dimensional data sequence       (double)[stdin]" << std::endl;  // NOLINT
  *stream << "  stdout:" << std::endl;
  *stream << "       linear interpolated data sequence (double)" << std::endl;
  *stream << "  notice:" << std::endl;
  *stream << "       x-axis values in input data must be in ascending order" << std::endl;  // NOLINT
  *stream << "       if -s is not given, its value is set to beggning of input data";  // NOLINT
  *stream << "       if -e is not given, its value is set to end of input data";
  *stream << std::endl;
  *stream << " SPTK: version " << sptk::kVersion << std::endl;
  *stream << std::endl;
  // clang-format on
}

}  // namespace

/**
 * @a linear_intpl [ @e option ] [ @e infile ]
 *
 * - @b -l @e int
 *   - output length @f$(1 \le L)@f$
 * - @b -m @e int
 *   - number of interpolation points @f$(0 \le L-1)@f$
 * - @b -s @e double
 *   - minimum value of x-axis @f$(x_{min})@f$
 * - @b -e @e double
 *   - maximum value of x-axis @f$(x_{max})@f$
 * - @b infile @e str
 *   - double-type 2D data sequence
 * - @b stdout
 *   - double-type linear interpolated 1D data sequence
 *
 * The input of this command is a 2-dimensional data sequence:
 * @f[
 *   \begin{array}{ccccc}
 *     x_0, & y_0, & x_1, & y_1, & \ldots
 *   \end{array}
 * @f]
 * The output is
 * @f[
 *   \begin{array}{cccc}
 *     f(x_{min}), & f(x_{min}+t), & f(x_{min}+2t), & \ldots, & f(x_{max}),
 *   \end{array}
 * @f]
 * where @f$t@f$ is the interval:
 * @f[
 *   t = \frac{x_{max} - x_{min}}{L - 1},
 * @f]
 * and @f$f(\cdot)@f$ is a linear interpolation function:
 * @f[
 *   f(x) = ax + b.
 * @f]
 * The slope @f$a@f$ and the intercept @f$b@f$ are calculated from the two
 * points that sandwitch @f$x@f$.
 *
 * @code{.sh}
 *   # x: 0, 2, 3, 5
 *   # y: 2, 2, 0, 1
 *   echo 0 2 2 2 3 0 5 1 | x2x +ad | linear_intpl -m 10 | x2x +da
 *   # 2, 2, 2, 2, 2, 1, 0, 0.25, 0.5, 0.75, 1
 * @endcode
 *
 * @param[in] argc Number of arguments.
 * @param[in] argv Argument vector.
 * @return 0 on success, 1 on failure.
 */
int main(int argc, char* argv[]) {
  int output_length(kDefaultOutputLength);
  double minimum_x(-DBL_MAX);
  double maximum_x(DBL_MAX);

  for (;;) {
    const int option_char(getopt_long(argc, argv, "l:m:s:e:h", NULL, NULL));
    if (-1 == option_char) break;

    switch (option_char) {
      case 'l': {
        if (!sptk::ConvertStringToInteger(optarg, &output_length) ||
            output_length <= 0) {
          std::ostringstream error_message;
          error_message
              << "The argument for the -l option must be a positive integer";
          sptk::PrintErrorMessage("linear_intpl", error_message);
          return 1;
        }
        break;
      }
      case 'm': {
        if (!sptk::ConvertStringToInteger(optarg, &output_length) ||
            output_length < 0) {
          std::ostringstream error_message;
          error_message << "The argument for the -m option must be a "
                        << "non-negative integer";
          sptk::PrintErrorMessage("linear_intpl", error_message);
          return 1;
        }
        ++output_length;
        break;
      }
      case 's': {
        if (!sptk::ConvertStringToDouble(optarg, &minimum_x)) {
          std::ostringstream error_message;
          error_message << "The argument for the -s option must be numeric";
          sptk::PrintErrorMessage("linear_intpl", error_message);
          return 1;
        }
        break;
      }
      case 'e': {
        if (!sptk::ConvertStringToDouble(optarg, &maximum_x)) {
          std::ostringstream error_message;
          error_message << "The argument for the -e option must be numeric";
          sptk::PrintErrorMessage("linear_intpl", error_message);
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
    sptk::PrintErrorMessage("linear_intpl", error_message);
    return 1;
  }
  const char* input_file(0 == num_input_files ? NULL : argv[optind]);

  std::ifstream ifs;
  ifs.open(input_file, std::ios::in | std::ios::binary);
  if (ifs.fail() && NULL != input_file) {
    std::ostringstream error_message;
    error_message << "Cannot open file " << input_file;
    sptk::PrintErrorMessage("linear_intpl", error_message);
    return 1;
  }
  std::istream& input_stream(ifs.fail() ? std::cin : ifs);

  // Read all data in advance.
  // This is due to estimate maximum_x if -e is not given.
  std::vector<double> data_x;
  std::vector<double> data_y;
  {
    double tmp_x;
    double tmp_y;
    while (sptk::ReadStream(&tmp_x, &input_stream) &&
           sptk::ReadStream(&tmp_y, &input_stream)) {
      data_x.push_back(tmp_x);
      data_y.push_back(tmp_y);
    }
  }

  // Check x-axis values.
  const int input_length(data_x.size());
  for (int i(1); i < input_length; ++i) {
    if (data_x[i] <= data_x[i - 1]) {
      std::ostringstream error_message;
      error_message << "X-axis values in input data must be in ascending order";
      sptk::PrintErrorMessage("linear_intpl", error_message);
      return 1;
    }
  }

  if (-DBL_MAX == minimum_x) {
    minimum_x = data_x.front();
  } else {
    if (minimum_x < data_x.front()) {
      std::ostringstream error_message;
      error_message << "Minimum value must be equal to or greater than "
                    << "minimum values of x-axis in input data";
      sptk::PrintErrorMessage("linear_intpl", error_message);
      return 1;
    }
    if (data_x.back() <= minimum_x) {
      std::ostringstream error_message;
      error_message << "Minimum value must be less than maximum values of "
                       "x-axis in input data";
      sptk::PrintErrorMessage("linear_intpl", error_message);
      return 1;
    }
  }

  if (DBL_MAX == maximum_x) {
    maximum_x = data_x.back();
  } else {
    if (data_x.back() < maximum_x) {
      std::ostringstream error_message;
      error_message << "Maximum value must be equal to or less than "
                    << "maximum values of x-axis in input data";
      sptk::PrintErrorMessage("linear_interpl", error_message);
      return 1;
    }
    if (maximum_x <= data_x.front()) {
      std::ostringstream error_message;
      error_message << "Maximum value must be greater than minimum values of "
                       "x-axis in input data";
      sptk::PrintErrorMessage("linear_intpl", error_message);
      return 1;
    }
  }

  if (maximum_x <= minimum_x) {
    std::ostringstream error_message;
    error_message << "Maximum of x must be greater than minimum of x";
    sptk::PrintErrorMessage("linear_intpl", error_message);
    return 1;
  }

  int start_index(0);
  while (data_x[start_index + 1] < minimum_x) {
    ++start_index;
  }

  int output_count(0);
  double x(minimum_x);
  const double interval((1 == output_length) ? 0.0 : ((maximum_x - minimum_x) /
                                                      (output_length - 1)));
  for (int index(start_index), next_index(index + 1); next_index < input_length;
       index = next_index, ++next_index) {
    if (x <= data_x[next_index]) {
      const double diff_x(data_x[index] - data_x[next_index]);
      const double diff_y(data_y[index] - data_y[next_index]);
      const double intercept((data_x[index] * data_y[next_index] -
                              data_x[next_index] * data_y[index]) /
                             diff_x);
      const double slope(diff_y / diff_x);
      // Note x += interval does not work well.
      for (; output_count < output_length && x <= data_x[next_index];
           ++output_count, x = minimum_x + interval * output_count) {
        const double y(slope * x + intercept);
        if (!sptk::WriteStream(y, &std::cout)) {
          std::ostringstream error_message;
          error_message << "Failed to write interpolated data";
          sptk::PrintErrorMessage("linear_intpl", error_message);
          return 1;
        }
      }
    }
  }

  return 0;
}
