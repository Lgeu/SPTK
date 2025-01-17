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

#include "SPTK/math/toeplitz_plus_hankel_system_solver.h"

#include <cstddef>  // std::size_t

namespace {

void PutBar(int i, const std::vector<double>& x, std::vector<double>* y) {
  (*y)[0] = x[i];
  (*y)[1] = x[x.size() - 1 - i];
}

}  // namespace

namespace sptk {

ToeplitzPlusHankelSystemSolver::ToeplitzPlusHankelSystemSolver(
    int num_order, bool coefficients_modification)
    : num_order_(num_order),
      coefficients_modification_(coefficients_modification),
      is_valid_(true) {
  if (num_order_ < 0) {
    is_valid_ = false;
    return;
  }
}

bool ToeplitzPlusHankelSystemSolver::Run(
    const std::vector<double>& toeplitz_coefficient_vector,
    const std::vector<double>& hankel_coefficient_vector,
    const std::vector<double>& constant_vector,
    std::vector<double>* solution_vector,
    ToeplitzPlusHankelSystemSolver::Buffer* buffer) const {
  // Check inputs.
  const int length(num_order_ + 1);
  if (!is_valid_ ||
      toeplitz_coefficient_vector.size() !=
          static_cast<std::size_t>(2 * length - 1) ||
      hankel_coefficient_vector.size() !=
          static_cast<std::size_t>(2 * length - 1) ||
      constant_vector.size() != static_cast<std::size_t>(length) ||
      NULL == solution_vector || NULL == buffer) {
    return false;
  }

  // Prepare memories.
  if (solution_vector->size() != static_cast<std::size_t>(length)) {
    solution_vector->resize(length);
  }
  if (buffer->r_.size() != static_cast<std::size_t>(length)) {
    buffer->r_.resize(length);
  }
  if (buffer->x_.size() != static_cast<std::size_t>(length)) {
    buffer->x_.resize(length);
  }
  if (buffer->prev_x_.size() != static_cast<std::size_t>(length)) {
    buffer->prev_x_.resize(length);
  }
  if (buffer->p_.size() != static_cast<std::size_t>(length)) {
    buffer->p_.resize(length);
    for (int i(0); i < length; ++i) {
      buffer->p_[i].resize(2);
    }
  }

  // Step 0)
  {
    // Set R.
    const double* t(&(toeplitz_coefficient_vector[0]));
    const double* h(&(hankel_coefficient_vector[0]));
    for (int i(0); i < length; ++i) {
      buffer->r_[i][0][0] = t[num_order_ + i];
      buffer->r_[i][1][1] = t[num_order_ - i];
      buffer->r_[i][0][1] = h[num_order_ + i];
      buffer->r_[i][1][0] = h[num_order_ - i];
    }

    if (coefficients_modification_) {
      const double d0(t[num_order_]);
      for (int i(0); i < length; i += 2) {
        buffer->r_[i][0][0] += d0;
        buffer->r_[i][1][1] += d0;
      }
      for (int i((0 == num_order_ % 2) ? 0 : 1); i < length; i += 2) {
        buffer->r_[i][0][1] -= d0;
        buffer->r_[i][1][0] -= d0;
      }
    }
  }

  // Step 1)
  {
    // Set X_0.
    buffer->x_[0].FillDiagonal(1.0);

    // Set p_0.
    PutBar(0, constant_vector, &buffer->bar_);
    if (!buffer->r_[0].Invert(&buffer->inv_) ||
        !sptk::Matrix2D::Multiply(buffer->inv_, buffer->bar_, &buffer->p_[0])) {
      return false;
    }

    // Set V_x.
    buffer->vx_ = buffer->r_[0];
  }

  // Step 2)
  for (int i(1); i < length; ++i) {
    // a) Calculate E_x.
    buffer->ex_.Fill(0.0);
    for (int j(0); j < i; ++j) {
      if (!sptk::Matrix2D::Multiply(buffer->r_[i - j], buffer->x_[j],
                                    &buffer->tmp_matrix_) ||
          !sptk::Matrix2D::Add(buffer->tmp_matrix_, &buffer->ex_)) {
        return false;
      }
    }

    // b) Calculate \bar{e}_p.
    buffer->ep_[0] = 0.0;
    buffer->ep_[1] = 0.0;
    for (int j(0); j < i; ++j) {
      if (!sptk::Matrix2D::Multiply(buffer->r_[i - j], buffer->p_[j],
                                    &buffer->tmp_vector_)) {
        return false;
      }
      buffer->ep_[0] += buffer->tmp_vector_[0];
      buffer->ep_[1] += buffer->tmp_vector_[1];
    }

    // c) Calculate B_x.
    if (!buffer->vx_.CrossTranspose(&buffer->tau_) ||
        !buffer->tau_.Invert(&buffer->inv_) ||
        !sptk::Matrix2D::Multiply(buffer->inv_, buffer->ex_, &buffer->bx_)) {
      return false;
    }

    // d) Update X.
    for (int j(1); j < i; ++j) {
      if (!buffer->prev_x_[i - j].CrossTranspose(&buffer->tau_) ||
          !sptk::Matrix2D::Multiply(buffer->tau_, buffer->bx_,
                                    &buffer->tmp_matrix_) ||
          !sptk::Matrix2D::Subtract(buffer->tmp_matrix_, &buffer->x_[j])) {
        return false;
      }
    }
    buffer->x_[i].Negate(buffer->bx_);
    for (int j(1); j <= i; ++j) {
      buffer->prev_x_[j] = buffer->x_[j];
    }

    // d) Update V_x.
    if (!buffer->ex_.CrossTranspose(&buffer->tau_) ||
        !sptk::Matrix2D::Multiply(buffer->tau_, buffer->bx_,
                                  &buffer->tmp_matrix_) ||
        !sptk::Matrix2D::Subtract(buffer->tmp_matrix_, &buffer->vx_)) {
      return false;
    }

    // e) Calculate \bar{g}.
    if (!buffer->vx_.CrossTranspose(&buffer->tau_) ||
        !buffer->tau_.Invert(&buffer->inv_)) {
      return false;
    }
    PutBar(i, constant_vector, &buffer->bar_);
    buffer->tmp_vector_[0] = buffer->bar_[0] - buffer->ep_[0];
    buffer->tmp_vector_[1] = buffer->bar_[1] - buffer->ep_[1];
    if (!sptk::Matrix2D::Multiply(buffer->inv_, buffer->tmp_vector_,
                                  &buffer->g_)) {
      return false;
    }

    // f) Update \bar{p}.
    for (int j(0); j < i; ++j) {
      if (!buffer->x_[i - j].CrossTranspose(&buffer->tau_) ||
          !sptk::Matrix2D::Multiply(buffer->tau_, buffer->g_,
                                    &buffer->tmp_vector_)) {
        return false;
      }
      buffer->p_[j][0] += buffer->tmp_vector_[0];
      buffer->p_[j][1] += buffer->tmp_vector_[1];
    }
    buffer->p_[i] = buffer->g_;
  }

  // Step 3)
  {
    double* a(&((*solution_vector)[0]));
    for (int i(0); i < length; ++i) {
      a[i] = buffer->p_[i][0];
    }
  }

  return true;
}

}  // namespace sptk
