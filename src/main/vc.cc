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

#include <getopt.h>  // getopt_long_only

#include <fstream>   // std::ifstream
#include <iomanip>   // std::setw
#include <iostream>  // std::cerr, std::cin, std::cout, std::endl, etc.
#include <sstream>   // std::ostringstream
#include <vector>    // std::vector

#include "SPTK/math/gaussian_mixture_model_based_conversion.h"
#include "SPTK/utils/sptk_utils.h"

namespace {

enum LongOptions {
  kMagic = 1000,
};

const int kDefaultNumOrder(25);
const int kDefaultNumMixture(16);
const bool kDefaultFullCovarianceFlag(false);

void PrintUsage(std::ostream* stream) {
  // clang-format off
  *stream << std::endl;
  *stream << " vc - GMM-based voice conversion" << std::endl;
  *stream << std::endl;
  *stream << "  usage:" << std::endl;
  *stream << "       vc [ options ] gmmfile [ infile ] > stdout" << std::endl;
  *stream << "  options:" << std::endl;
  *stream << "       -l l          : length of source vector (   int)[" << std::setw(5) << std::right << kDefaultNumOrder + 1 << "][ 1 <= l <=   ]" << std::endl;  // NOLINT
  *stream << "       -m m          : order of source vector  (   int)[" << std::setw(5) << std::right << "l-1"                << "][ 0 <= m <=   ]" << std::endl;  // NOLINT
  *stream << "       -L L          : length of target vector (   int)[" << std::setw(5) << std::right << "l"                  << "][ 1 <= L <=   ]" << std::endl;  // NOLINT
  *stream << "       -M M          : order of target vector  (   int)[" << std::setw(5) << std::right << "L-1"                << "][ 0 <= M <=   ]" << std::endl;  // NOLINT
  *stream << "       -k k          : number of mixtures      (   int)[" << std::setw(5) << std::right << kDefaultNumMixture   << "][ 1 <= k <=   ]" << std::endl;  // NOLINT
  *stream << "       -f            : use full covariance or  (  bool)[" << std::setw(5) << std::right << sptk::ConvertBooleanToString(kDefaultFullCovarianceFlag) << "]" << std::endl;  // NOLINT
  *stream << "                       block covariance" << std::endl;
  *stream << "       -d d1 d2 ...  : delta coefficients      (double)[" << std::setw(5) << std::right << "N/A"                << "]" << std::endl;  // NOLINT
  *stream << "       -D D          : filename of double type (string)[" << std::setw(5) << std::right << "N/A"                << "]" << std::endl;  // NOLINT
  *stream << "                       delta coefficients" << std::endl;
  *stream << "       -r r1 (r2)    : width of regression     (   int)[" << std::setw(5) << std::right << "N/A"                << "]" << std::endl;  // NOLINT
  *stream << "                       coefficients" << std::endl;
  *stream << "       -magic magic  : magic number            (double)[" << std::setw(5) << std::right << "N/A"                << "]" << std::endl;  // NOLINT
  *stream << "       -h            : print this message" << std::endl;
  *stream << "  gmmfile:" << std::endl;
  *stream << "       GMM parameters                          (double)" << std::endl;  // NOLINT
  *stream << "  infile:" << std::endl;
  *stream << "       source static+dynamic vector sequence   (double)[stdin]" << std::endl;  // NOLINT
  *stream << "  stdout:" << std::endl;
  *stream << "       target static vector sequence           (double)" << std::endl;  // NOLINT
  *stream << std::endl;
  *stream << " SPTK: version " << sptk::kVersion << std::endl;
  *stream << std::endl;
  // clang-format on
}

}  // namespace

/**
 * @a vc [ @e option ] @e gmmfile [ @e infile ]
 *
 * - @b -l @e int
 *   - length of source vector @f$(1 \le M_1 + 1)@f$
 * - @b -m @e int
 *   - order of source vector @f$(0 \le M_1)@f$
 * - @b -L @e int
 *   - length of target vector @f$(1 \le M_2 + 1)@f$
 * - @b -M @e int
 *   - order of target vector @f$(0 \le M_2)@f$
 * - @b -k @e int
 *   - number of mixtures @f$(1 \le K)@f$
 * - @b -f @e bool
 *   - use full or block covariance instead of diagonal one
 * - @b -d @e double+
 *   - delta coefficients
 * - @b -D @e string
 *   - filename of double-type delta coefficients
 * - @b -r @e int+
 *   - width of 1st (and 2nd) regression coefficients
 * - @b -magic @e double
 *   - magic number
 * - @b gmmfile @e str
 *   - double-type GMM parameters
 * - @b infile @e str
 *   - double-type source static+dynamic vector sequence
 * - @b stdout
 *   - double-type target static vector sequence
 *
 * In the following example, the converted 4-th order vectors corresponding
 * @c data.source are obtained using the trained 2-mixture GMM @c data.gmm.
 *
 * @code{.sh}
 *   delta -l 5 -d -0.5 0.0 0.5 data.source | \
 *     vc -k 2 -l 5 data.gmm > data.target
 * @endcode
 *
 * @param[in] argc Number of arguments.
 * @param[in] argv Argument vector.
 * @return 0 on success, 1 on failure.
 */
int main(int argc, char* argv[]) {
  int num_source_order(kDefaultNumOrder);
  int num_target_order(kDefaultNumOrder);
  bool is_num_target_order_specified(false);
  int num_mixture(kDefaultNumMixture);
  bool full_covariance_flag(kDefaultFullCovarianceFlag);
  std::vector<std::vector<double> > window_coefficients;
  bool is_regression_specified(false);
  double magic_number(0.0);
  bool is_magic_number_specified(false);

  const struct option long_options[] = {
      {"magic", required_argument, NULL, kMagic}, {0, 0, 0, 0},
  };

  for (;;) {
    const int option_char(
        getopt_long_only(argc, argv, "l:m:L:M:k:fd:D:r:h", long_options, NULL));
    if (-1 == option_char) break;

    switch (option_char) {
      case 'l': {
        if (!sptk::ConvertStringToInteger(optarg, &num_source_order) ||
            num_source_order <= 0) {
          std::ostringstream error_message;
          error_message
              << "The argument for the -l option must be a positive integer";
          sptk::PrintErrorMessage("vc", error_message);
          return 1;
        }
        --num_source_order;
        break;
      }
      case 'm': {
        if (!sptk::ConvertStringToInteger(optarg, &num_source_order) ||
            num_source_order < 0) {
          std::ostringstream error_message;
          error_message << "The argument for the -m option must be a "
                        << "non-negative integer";
          sptk::PrintErrorMessage("vc", error_message);
          return 1;
        }
        break;
      }
      case 'L': {
        if (!sptk::ConvertStringToInteger(optarg, &num_target_order) ||
            num_target_order <= 0) {
          std::ostringstream error_message;
          error_message
              << "The argument for the -L option must be a positive integer";
          sptk::PrintErrorMessage("vc", error_message);
          return 1;
        }
        --num_target_order;
        is_num_target_order_specified = true;
        break;
      }
      case 'M': {
        if (!sptk::ConvertStringToInteger(optarg, &num_target_order) ||
            num_target_order < 0) {
          std::ostringstream error_message;
          error_message << "The argument for the -M option must be a "
                        << "non-negative integer";
          sptk::PrintErrorMessage("vc", error_message);
          return 1;
        }
        is_num_target_order_specified = true;
        break;
      }
      case 'k': {
        if (!sptk::ConvertStringToInteger(optarg, &num_mixture) ||
            num_mixture <= 0) {
          std::ostringstream error_message;
          error_message
              << "The argument for the -k option must be a positive integer";
          sptk::PrintErrorMessage("vc", error_message);
          return 1;
        }
        break;
      }
      case 'f': {
        full_covariance_flag = true;
        break;
      }
      case 'd': {
        if (is_regression_specified) {
          std::ostringstream error_message;
          error_message << "-d and -r options cannot be specified at the same";
          sptk::PrintErrorMessage("vc", error_message);
          return 1;
        }

        std::vector<double> coefficients;
        double coefficient;
        if (!sptk::ConvertStringToDouble(optarg, &coefficient)) {
          std::ostringstream error_message;
          error_message << "The argument for the -d option must be numeric";
          sptk::PrintErrorMessage("vc", error_message);
          return 1;
        }
        coefficients.push_back(coefficient);
        while (optind < argc &&
               sptk::ConvertStringToDouble(argv[optind], &coefficient)) {
          coefficients.push_back(coefficient);
          ++optind;
        }
        window_coefficients.push_back(coefficients);
        break;
      }
      case 'D': {
        if (is_regression_specified) {
          std::ostringstream error_message;
          error_message << "-D and -r options cannot be specified at the same";
          sptk::PrintErrorMessage("vc", error_message);
          return 1;
        }

        std::ifstream ifs;
        ifs.open(optarg, std::ios::in | std::ios::binary);
        if (ifs.fail()) {
          std::ostringstream error_message;
          error_message << "Cannot open file " << optarg;
          sptk::PrintErrorMessage("vc", error_message);
          return 1;
        }
        std::vector<double> coefficients;
        double coefficient;
        while (sptk::ReadStream(&coefficient, &ifs)) {
          coefficients.push_back(coefficient);
        }
        window_coefficients.push_back(coefficients);
        break;
      }
      case 'r': {
        if (is_regression_specified) {
          std::ostringstream error_message;
          error_message << "-r option cannot be specified multiple times";
          sptk::PrintErrorMessage("vc", error_message);
          return 1;
        }

        int n;
        // Set first order coefficients.
        {
          if (!sptk::ConvertStringToInteger(optarg, &n) || n <= 0) {
            std::ostringstream error_message;
            error_message
                << "The argument for the -r option must be positive integer(s)";
            sptk::PrintErrorMessage("vc", error_message);
            return 1;
          }

          std::vector<double> coefficients(2 * n + 1);
          const int a1(n * (n + 1) * (2 * n + 1) / 3);
          const double norm(1.0 / a1);
          for (int j(-n), i(0); j <= n; ++j, ++i) {
            coefficients[i] = j * norm;
          }
          window_coefficients.push_back(coefficients);
        }

        // Set second order coefficients.
        if (optind < argc && sptk::ConvertStringToInteger(argv[optind], &n)) {
          if (n <= 0) {
            std::ostringstream error_message;
            error_message
                << "The argument for the -r option must be positive integer(s)";
            sptk::PrintErrorMessage("vc", error_message);
            return 1;
          }
          std::vector<double> coefficients(2 * n + 1);
          const int a0(2 * n + 1);
          const int a1(a0 * n * (n + 1) / 3);
          const int a2(a1 * (3 * n * n + 3 * n - 1) / 5);
          const double norm(2.0 / (a2 * a0 - a1 * a1));
          for (int j(-n), i(0); j <= n; ++j, ++i) {
            coefficients[i] = (a0 * j * j - a1) * norm;
          }
          window_coefficients.push_back(coefficients);
          ++optind;
        }
        is_regression_specified = true;
        break;
      }
      case kMagic: {
        if (!sptk::ConvertStringToDouble(optarg, &magic_number)) {
          std::ostringstream error_message;
          error_message
              << "The argument for the -magic option must be a number";
          sptk::PrintErrorMessage("vc", error_message);
          return 1;
        }
        is_magic_number_specified = true;
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

  if (!is_num_target_order_specified) {
    num_target_order = num_source_order;
  }

  // Get input file names.
  const char* gmm_file;
  const char* input_file;
  const int num_input_files(argc - optind);
  if (2 == num_input_files) {
    gmm_file = argv[argc - 2];
    input_file = argv[argc - 1];
  } else if (1 == num_input_files) {
    gmm_file = argv[argc - 1];
    input_file = NULL;
  } else {
    std::ostringstream error_message;
    error_message << "Just two input files, gmmfile and infile, are required";
    sptk::PrintErrorMessage("vc", error_message);
    return 1;
  }

  // Load GMM.
  std::vector<double> weights(num_mixture);
  std::vector<std::vector<double> > mean_vectors(num_mixture);
  std::vector<sptk::SymmetricMatrix> covariance_matrices(num_mixture);
  {
    std::ifstream ifs;
    ifs.open(gmm_file, std::ios::in | std::ios::binary);
    if (ifs.fail()) {
      std::ostringstream error_message;
      error_message << "Cannot open file " << gmm_file;
      sptk::PrintErrorMessage("vc", error_message);
      return 1;
    }
    std::istream& input_stream(ifs);

    const int read_size((window_coefficients.size() + 1) *
                        (num_source_order + num_target_order + 2));
    for (int k(0); k < num_mixture; ++k) {
      if (!sptk::ReadStream(&(weights[k]), &input_stream)) {
        std::ostringstream error_message;
        error_message << "Failed to load mixture weight";
        sptk::PrintErrorMessage("vc", error_message);
        return 1;
      }

      if (!sptk::ReadStream(false, 0, 0, read_size, &mean_vectors[k],
                            &input_stream, NULL)) {
        std::ostringstream error_message;
        error_message << "Failed to load mean vector";
        sptk::PrintErrorMessage("vc", error_message);
        return 1;
      }

      covariance_matrices[k].Resize(read_size);
      if (full_covariance_flag) {
        if (!sptk::ReadStream(&covariance_matrices[k], &input_stream)) {
          std::ostringstream error_message;
          error_message << "Failed to load covariance matrix";
          sptk::PrintErrorMessage("vc", error_message);
          return 1;
        }
      } else {
        std::vector<double> variance;
        if (!sptk::ReadStream(false, 0, 0, read_size, &variance, &input_stream,
                              NULL)) {
          std::ostringstream error_message;
          error_message << "Failed to load diagonal covariance vector";
          sptk::PrintErrorMessage("vc", error_message);
          return 1;
        }
        covariance_matrices[k].SetDiagonal(variance);
      }
    }
  }

  // Read input vectors.
  std::vector<std::vector<double> > source_vectors;
  {
    std::ifstream ifs;
    ifs.open(input_file, std::ios::in | std::ios::binary);
    if (ifs.fail() && NULL != input_file) {
      std::ostringstream error_message;
      error_message << "Cannot open file " << input_file;
      sptk::PrintErrorMessage("vc", error_message);
      return 1;
    }
    std::istream& input_stream(ifs.fail() ? std::cin : ifs);

    const int read_size((window_coefficients.size() + 1) *
                        (num_source_order + 1));
    std::vector<double> tmp;
    while (
        sptk::ReadStream(false, 0, 0, read_size, &tmp, &input_stream, NULL)) {
      source_vectors.push_back(tmp);
    }
  }

  // Perform voice conversion.
  std::vector<std::vector<double> > target_vectors;
  {
    sptk::GaussianMixtureModelBasedConversion conversion(
        num_source_order, num_target_order, window_coefficients, weights,
        mean_vectors, covariance_matrices, is_magic_number_specified,
        magic_number);
    if (!conversion.IsValid()) {
      std::ostringstream error_message;
      error_message
          << "Failed to initialize GaussianMixtureModelBasedConversion";
      sptk::PrintErrorMessage("vc", error_message);
      return 1;
    }
    if (!conversion.Run(source_vectors, &target_vectors)) {
      std::ostringstream error_message;
      error_message << "Failed to perform voice conversion";
      sptk::PrintErrorMessage("vc", error_message);
      return 1;
    }
  }

  // Write output vectors.
  {
    const int sequence_length(static_cast<int>(target_vectors.size()));
    for (int t(0); t < sequence_length; ++t) {
      if (!sptk::WriteStream(0, num_target_order + 1, target_vectors[t],
                             &std::cout, NULL)) {
        std::ostringstream error_message;
        error_message << "Failed to write target vectors";
        sptk::PrintErrorMessage("vc", error_message);
        return 1;
      }
    }
  }

  return 0;
}
