/////////////////////////////////////////////////////////////////////////
// $Id$
/////////////////////////////////////////////////////////////////////////
//
//   Copyright (c) 2013-2018 Stanislav Shwartsman
//          Written by Stanislav Shwartsman [sshwarts at sourceforge net]
//
//  This library is free software; you can redistribute it and/or
//  modify it under the terms of the GNU Lesser General Public
//  License as published by the Free Software Foundation; either
//  version 2 of the License, or (at your option) any later version.
//
//  This library is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with this library; if not, write to the Free Software
//  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA B 02110-1301 USA
//
/////////////////////////////////////////////////////////////////////////

#define NEED_CPU_REG_SHORTCUTS 1
#include "bochs.h"
#include "cpu.h"
#define LOG_THIS BX_CPU_THIS_PTR

#if BX_SUPPORT_EVEX

extern float_status_t mxcsr_to_softfloat_status_word(bx_mxcsr_t mxcsr);

#include "simd_int.h"
#include "simd_pfp.h"

//////////////////////////////
// AVX-512 FMA Instructions //
//////////////////////////////

#define EVEX_FMA_SCALAR_SINGLE(HANDLER, func)                                 \
  void BX_CPP_AttrRegparmN(1) BX_CPU_C:: HANDLER (bxInstruction_c *i)         \
  {                                                                           \
    if (BX_SCALAR_ELEMENT_MASK(i->opmask())) {                                \
      float32 op1 = BX_READ_XMM_REG_LO_DWORD(i->src1());                      \
      float32 op2 = BX_READ_XMM_REG_LO_DWORD(i->src2());                      \
      float32 op3 = BX_READ_XMM_REG_LO_DWORD(i->src3());                      \
                                                                              \
      float_status_t status = mxcsr_to_softfloat_status_word(MXCSR);          \
      softfloat_status_word_rc_override(status, i);                           \
      op1 = (func)(op1, op2, op3, status);                                    \
      check_exceptionsSSE(get_exception_flags(status));                       \
                                                                              \
      BX_WRITE_XMM_REG_LO_DWORD(i->dst(), op1);                               \
    }                                                                         \
    else {                                                                    \
      if (i->isZeroMasking())                                                 \
        BX_WRITE_XMM_REG_LO_DWORD(i->dst(), 0);                               \
    }                                                                         \
                                                                              \
    BX_CLEAR_AVX_HIGH128(i->dst());                                           \
    BX_NEXT_INSTR(i);                                                         \
  }

EVEX_FMA_SCALAR_SINGLE(VFMADDSS_MASK_VpsHssWssR, float32_fmadd)
EVEX_FMA_SCALAR_SINGLE(VFMSUBSS_MASK_VpsHssWssR, float32_fmsub)
EVEX_FMA_SCALAR_SINGLE(VFNMADDSS_MASK_VpsHssWssR, float32_fnmadd)
EVEX_FMA_SCALAR_SINGLE(VFNMSUBSS_MASK_VpsHssWssR, float32_fnmsub)

#define EVEX_FMA_SCALAR_DOUBLE(HANDLER, func)                                 \
  void BX_CPP_AttrRegparmN(1) BX_CPU_C:: HANDLER (bxInstruction_c *i)         \
  {                                                                           \
    if (BX_SCALAR_ELEMENT_MASK(i->opmask())) {                                \
      float64 op1 = BX_READ_XMM_REG_LO_QWORD(i->src1());                      \
      float64 op2 = BX_READ_XMM_REG_LO_QWORD(i->src2());                      \
      float64 op3 = BX_READ_XMM_REG_LO_QWORD(i->src3());                      \
                                                                              \
      float_status_t status = mxcsr_to_softfloat_status_word(MXCSR);          \
      softfloat_status_word_rc_override(status, i);                           \
      op1 = (func)(op1, op2, op3, status);                                    \
      check_exceptionsSSE(get_exception_flags(status));                       \
                                                                              \
      BX_WRITE_XMM_REG_LO_QWORD(i->dst(), op1);                               \
    }                                                                         \
    else {                                                                    \
      if (i->isZeroMasking())                                                 \
        BX_WRITE_XMM_REG_LO_QWORD(i->dst(), 0);                               \
    }                                                                         \
                                                                              \
    BX_CLEAR_AVX_HIGH128(i->dst());                                           \
    BX_NEXT_INSTR(i);                                                         \
  }

EVEX_FMA_SCALAR_DOUBLE(VFMADDSD_MASK_VpdHsdWsdR, float64_fmadd)
EVEX_FMA_SCALAR_DOUBLE(VFMSUBSD_MASK_VpdHsdWsdR, float64_fmsub)
EVEX_FMA_SCALAR_DOUBLE(VFNMADDSD_MASK_VpdHsdWsdR, float64_fnmadd)
EVEX_FMA_SCALAR_DOUBLE(VFNMSUBSD_MASK_VpdHsdWsdR, float64_fnmsub)

#endif
