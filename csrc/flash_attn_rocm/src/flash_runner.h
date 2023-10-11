// BSD 3 Clause
// Copyright 2023 Advanced Micro Devices, Inc.
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
// 1. Redistributions of source code must retain the above copyright notice,
// this list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright notice,
// this list of conditions and the following disclaimer in the documentation
// and/or other materials provided with the distribution.
// 3. Neither the name of the copyright holder nor the names of its contributors
// may be used to endorse or promote products derived from this software without
// specific prior written permission. THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT
// HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
// INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
// FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
// COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
// INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
// OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
// EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#pragma once

#include <memory>
#include <cstdlib>

#include "params.h"
#include "flash_fwd_runner_gfx9x.h"
#include "flash_bwd_runner_gfx9x.h"

#include "static_switch.h"

class FlashRunner {
 public:
  // constructor
  explicit FlashRunner()
  : pimpl_fwd_runner_(std::make_unique<fwd_device_gemm::FlashFwdRunner>()),
    pimpl_bwd_runner_(std::make_unique<bwd_device_gemm::FlashBwdRunner>()) {}

  void RunFwd(FlashFwdParams &params, hipStream_t &stream) {
    HEADDIM_SWITCH(params.d, [&] {
      BF16_SWITCH(params.is_bf16, [&] {
        BOOL_SWITCH(params.is_mnko_padding, kIsPadding, [&] {
          BOOL_SWITCH(params.is_causal, kIsCausal, [&] {
            BOOL_SWITCH(params.cu_seqlens_q, kIsGrouped, [&] {
              pimpl_fwd_runner_->Run<kIsGrouped, kHeadDim, T, kIsPadding, kIsCausal>(params, stream);
            });
          });
        });
      });
    });
  }

  void RunBwd(FlashBwdParams &params, hipStream_t &stream) {
    HEADDIM_SWITCH(params.d, [&] {
      BF16_SWITCH(params.is_bf16, [&] {
        BOOL_SWITCH(params.is_mnko_padding, kIsPadding, [&] {
          BOOL_SWITCH(params.is_causal, kIsCausal, [&] {
            BOOL_SWITCH(params.cu_seqlens_q, kIsGrouped, [&] {
              pimpl_bwd_runner_->Run<kIsGrouped, kHeadDim, T, kIsPadding, kIsCausal>(params, stream);
            });
          });
        });
      });
    });
  }

 private:
  std::unique_ptr<fwd_device_gemm::FlashFwdRunner> pimpl_fwd_runner_;
  std::unique_ptr<bwd_device_gemm::FlashBwdRunner> pimpl_bwd_runner_;
}; // class FlashRunner