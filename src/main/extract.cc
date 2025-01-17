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

#include "SPTK/utils/sptk_utils.h"

namespace {

const int kDefaultVectorLength(26);
const int kDefaultCodebookIndex(0);

void PrintUsage(std::ostream* stream) {
  // clang-format off
  *stream << std::endl;
  *stream << " extract - extract vector" << std::endl;
  *stream << std::endl;
  *stream << "  usage:" << std::endl;
  *stream << "       extract [ options ] idxfile [ infile ] > stdout" << std::endl;  // NOLINT
  *stream << "  options:" << std::endl;
  *stream << "       -l l  : length of vector   (   int)[" << std::setw(5) << std::right << kDefaultVectorLength  << "][ 1 <= l <=   ]" << std::endl;  // NOLINT
  *stream << "       -m m  : order of vector    (   int)[" << std::setw(5) << std::right << "l-1"                 << "][ 0 <= m <=   ]" << std::endl;  // NOLINT
  *stream << "       -i i  : codebook index     (   int)[" << std::setw(5) << std::right << kDefaultCodebookIndex << "][ 0 <= i <=   ]" << std::endl;  // NOLINT
  *stream << "       -h    : print this message" << std::endl;
  *stream << "  idxfile:" << std::endl;
  *stream << "       index sequence             (   int)" << std::endl;
  *stream << "  infile:" << std::endl;
  *stream << "       data sequence              (double)[stdin]" << std::endl;
  *stream << "  stdout:" << std::endl;
  *stream << "       extracted data sequence    (double)" << std::endl;
  *stream << std::endl;
  *stream << " SPTK: version " << sptk::kVersion << std::endl;
  *stream << std::endl;
  // clang-format on
}

}  // namespace

/**
 * @a extract [ @e option ] idxfile [ @e infile ]
 *
 * - @b -l @e int
 *   - length of vector @f$(1 \le M + 1)@f$
 * - @b -m @e int
 *   - order of vector @f$(0 \le M)@f$
 * - @b -i @e int
 *   - codebook index @f$(0 \le j)@f$
 * - @b infile @e str
 *   - double-type data sequence
 * - @b stdout
 *   - double-type extracted data sequence
 *
 * The command takes two input sequence:
 * @f[
 *   \begin{array}{cccc}
 *     \boldsymbol{x}(0), & \boldsymbol{x}(1), & \boldsymbol{x}(2), & \ldots,
 *   \end{array}
 * @f]
 * and
 * @f[
 *   \begin{array}{cccc}
 *     i(0), & i(1), & i(2), & \ldots,
 *   \end{array}
 * @f]
 * where @f$\boldsymbol{x}(t)@f$ is an @f$(M+1)@f$-dimensional vector.
 * If @f$i(t)@f$ is the same as the given codebook index @f$j@f$,
 * this command outputs @f$\boldsymbol{x}(t)@f$. An example of the output is
 * @f[
 *   \begin{array}{cccc}
 *     \boldsymbol{x}(1), & \boldsymbol{x}(4), & \boldsymbol{x}(5), & \ldots
 *   \end{array}
 * @f]
 *
 * The below example extracts 10th order vectors indexed as 0 from @c data.d.
 *
 * @code{.sh}
 *   extract -l 10 -i 0 data.idx < data.d > data.ext
 * @endcode
 *
 * @param[in] argc Number of arguments.
 * @param[in] argv Argument vector.
 * @return 0 on success, 1 on failure.
 */
int main(int argc, char* argv[]) {
  int vector_length(kDefaultVectorLength);
  int codebook_index(kDefaultCodebookIndex);

  for (;;) {
    const int option_char(getopt_long(argc, argv, "l:m:i:h", NULL, NULL));
    if (-1 == option_char) break;

    switch (option_char) {
      case 'l': {
        if (!sptk::ConvertStringToInteger(optarg, &vector_length) ||
            vector_length <= 0) {
          std::ostringstream error_message;
          error_message
              << "The argument for the -l option must be a positive integer";
          sptk::PrintErrorMessage("extract", error_message);
          return 1;
        }
        break;
      }
      case 'm': {
        if (!sptk::ConvertStringToInteger(optarg, &vector_length) ||
            vector_length < 0) {
          std::ostringstream error_message;
          error_message << "The argument for the -m option must be a "
                        << "non-negative integer";
          sptk::PrintErrorMessage("extract", error_message);
          return 1;
        }
        ++vector_length;
        break;
      }
      case 'i': {
        if (!sptk::ConvertStringToInteger(optarg, &codebook_index) ||
            codebook_index < 0) {
          std::ostringstream error_message;
          error_message << "The argument for the -i option must be a "
                        << "non-negative integer";
          sptk::PrintErrorMessage("extract", error_message);
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

  // Get input file names.
  const char* index_file;
  const char* input_file;
  const int num_input_files(argc - optind);
  if (2 == num_input_files) {
    index_file = argv[argc - 2];
    input_file = argv[argc - 1];
  } else if (1 == num_input_files) {
    index_file = argv[argc - 1];
    input_file = NULL;
  } else {
    std::ostringstream error_message;
    error_message << "Just two input files, idxfile and infile, are required";
    sptk::PrintErrorMessage("extract", error_message);
    return 1;
  }

  // Open stream for reading index sequence.
  std::ifstream ifs1;
  ifs1.open(index_file, std::ios::in | std::ios::binary);
  if (ifs1.fail()) {
    std::ostringstream error_message;
    error_message << "Cannot open file " << index_file;
    sptk::PrintErrorMessage("extract", error_message);
    return 1;
  }
  std::istream& stream_for_index(ifs1);

  // Open stream for reading input sequence.
  std::ifstream ifs2;
  ifs2.open(input_file, std::ios::in | std::ios::binary);
  if (ifs2.fail() && NULL != input_file) {
    std::ostringstream error_message;
    error_message << "Cannot open file " << input_file;
    sptk::PrintErrorMessage("extract", error_message);
    return 1;
  }
  std::istream& stream_for_input(ifs2.fail() ? std::cin : ifs2);

  int index;
  std::vector<double> input_vector(vector_length);

  while (sptk::ReadStream(&index, &stream_for_index) &&
         sptk::ReadStream(false, 0, 0, vector_length, &input_vector,
                          &stream_for_input, NULL)) {
    if (index == codebook_index) {
      if (!sptk::WriteStream(0, vector_length, input_vector, &std::cout,
                             NULL)) {
        std::ostringstream error_message;
        error_message << "Failed to write extracted vector";
        sptk::PrintErrorMessage("extract", error_message);
        return 1;
      }
    }
  }

  return 0;
}
