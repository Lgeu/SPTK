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
#include <iostream>  // std::cerr, std::cin, std::cout, std::endl, etc.
#include <sstream>   // std::ostringstream

#include "SPTK/compression/huffman_decoding.h"
#include "SPTK/utils/sptk_utils.h"

namespace {

void PrintUsage(std::ostream* stream) {
  // clang-format off
  *stream << std::endl;
  *stream << " huffman_decode - Huffman decoding" << std::endl;
  *stream << std::endl;
  *stream << "  usage:" << std::endl;
  *stream << "       huffman_decode [ options ] cbfile [ infile ] > stdout" << std::endl;  // NOLINT
  *stream << "  options:" << std::endl;
  *stream << "       -h    : print this message" << std::endl;
  *stream << "  cbfile:" << std::endl;
  *stream << "       codebook                   (string)" << std::endl;
  *stream << "  infile:" << std::endl;
  *stream << "       codeword sequence          (  bool)[stdin]" << std::endl;
  *stream << "  stdout:" << std::endl;
  *stream << "       symbol sequence            (   int)" << std::endl;
  *stream << std::endl;
  *stream << " SPTK: version " << sptk::kVersion << std::endl;
  *stream << std::endl;
  // clang-format on
}

}  // namespace

/**
 * @a huffman_encode @e cbfile [ @e infile ]
 *
 * - @b cbfile @e str
 *   - ascii codebook
 * - @b infile @e str
 *   - bool-type codeword sequence
 * - @b stdout
 *   - int-type symbol sequence
 *
 * The below example encodes @c data.i and decodes it.
 *
 * @code{.sh}
 *   huffman_encode cbfile < data.i | huffman_decode cbfile > data.i2
 *   # data.i and data.i2 should be identical
 * @endcode
 *
 * @param[in] argc Number of arguments.
 * @param[in] argv Argument vector.
 * @return 0 on success, 1 on failure.
 */
int main(int argc, char* argv[]) {
  for (;;) {
    const int option_char(getopt_long(argc, argv, "h", NULL, NULL));
    if (-1 == option_char) break;

    switch (option_char) {
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

  const char* codebook_file;
  const char* input_file;
  const int num_input_files(argc - optind);
  if (2 == num_input_files) {
    codebook_file = argv[argc - 2];
    input_file = argv[argc - 1];
  } else if (1 == num_input_files) {
    codebook_file = argv[argc - 1];
    input_file = NULL;
  } else {
    std::ostringstream error_message;
    error_message << "Just two input files, cbfile and infile, are required";
    sptk::PrintErrorMessage("huffman_decode", error_message);
    return 1;
  }

  std::ifstream ifs1;
  ifs1.open(codebook_file, std::ios::in);
  if (ifs1.fail()) {
    std::ostringstream error_message;
    error_message << "Cannot open file " << codebook_file;
    sptk::PrintErrorMessage("huffman_decode", error_message);
    return 1;
  }
  sptk::HuffmanDecoding huffman_decoding(&ifs1);
  if (!huffman_decoding.IsValid()) {
    std::ostringstream error_message;
    error_message << "Failed to initialize HuffmanDecoding";
    sptk::PrintErrorMessage("huffman_decode", error_message);
    return 1;
  }

  std::ifstream ifs2;
  ifs2.open(input_file, std::ios::in | std::ios::binary);
  if (ifs2.fail() && NULL != input_file) {
    std::ostringstream error_message;
    error_message << "Cannot open file " << input_file;
    sptk::PrintErrorMessage("huffman_decode", error_message);
    return 1;
  }
  std::istream& input_stream(ifs2.fail() ? std::cin : ifs2);

  bool input;
  int output;
  bool is_leaf;

  while (sptk::ReadStream(&input, &input_stream)) {
    if (!huffman_decoding.Get(input, &output, &is_leaf)) {
      std::ostringstream error_message;
      error_message << "Failed to decode";
      sptk::PrintErrorMessage("huffman_decode", error_message);
      return 1;
    }

    if (is_leaf && !sptk::WriteStream(output, &std::cout)) {
      std::ostringstream error_message;
      error_message << "Failed to write decoded data";
      sptk::PrintErrorMessage("huffman_decode", error_message);
      return 1;
    }
  }

  return 0;
}
