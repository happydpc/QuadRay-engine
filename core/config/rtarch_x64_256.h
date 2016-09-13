/******************************************************************************/
/* Copyright (c) 2013-2016 VectorChief (at github, bitbucket, sourceforge)    */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#ifndef RT_RTARCH_X64_256_H
#define RT_RTARCH_X64_256_H

#if   RT_ADDRESS == 32

#include "rtarch_x32.h"

#elif RT_ADDRESS == 64

#include "rtarch_x64.h"

#else  /* RT_ADDRESS */

#error "unsupported address size, check RT_ADDRESS in makefiles"

#endif /* RT_ADDRESS */

#define RT_SIMD_REGS        16
#define RT_SIMD_WIDTH       4
#define RT_SIMD_ALIGN       32
#define RT_SIMD_SET(s, v)   s[0]=s[1]=s[2]=s[3]=v

#if defined (RT_SIMD_CODE)

#undef  sregs_sa
#undef  sregs_la
#undef  mxcsr_ld

#if defined (RT_256) && (RT_256 != 0)

/******************************************************************************/
/*********************************   LEGEND   *********************************/
/******************************************************************************/

/*
 * rtarch_x64_256.h: Implementation of x86_64 fp64 AVX(1,2) instructions.
 *
 * This file is a part of the unified SIMD assembler framework (rtarch.h)
 * designed to be compatible with different processor architectures,
 * while maintaining strictly defined common API.
 *
 * Recommended naming scheme for instructions:
 *
 * cmdp*_ri - applies [cmd] to [p]acked: [r]egister from [i]mmediate
 * cmdp*_rr - applies [cmd] to [p]acked: [r]egister from [r]egister
 *
 * cmdp*_rm - applies [cmd] to [p]acked: [r]egister from [m]emory
 * cmdp*_ld - applies [cmd] to [p]acked: as above
 *
 * cmdpx_** - applies [cmd] to [p]acked unsigned integer args, [x] - default
 * cmdpn_** - applies [cmd] to [p]acked   signed integer args, [n] - negatable
 * cmdps_** - applies [cmd] to [p]acked floating point   args, [s] - scalable
 *
 * The cmdp*_** instructions are intended for SPMD programming model
 * and can be configured to work with 32/64-bit data-elements (int, fp).
 * In this model data-paths are fixed-width, BASE and SIMD data-elements are
 * width-compatible, code-path divergence is handled via CHECK_MASK macro.
 */

/******************************************************************************/
/********************************   INTERNAL   ********************************/
/******************************************************************************/

/* 3-byte VEX prefix with full customization */
#define VEX(rxg, rxm, ren, len, pfx, aux)                                   \
        EMITB(0xC4)                                                         \
        EMITB((1 - (rxg)) << 7 | 1 << 6 | (1 - (rxm)) << 5 | (aux))         \
        EMITB((len) << 2 | (0x0F - (ren)) << 3 | (pfx))

/* selector for full register (3rd operand, 4-bits-wide) */
#define REN(reg, mod, sib)  reg

/******************************************************************************/
/********************************   EXTERNAL   ********************************/
/******************************************************************************/

/* registers    REG,  MOD,  SIB */

#define Xmm0    0x00, 0x03, EMPTY
#define Xmm1    0x01, 0x03, EMPTY
#define Xmm2    0x02, 0x03, EMPTY
#define Xmm3    0x03, 0x03, EMPTY
#define Xmm4    0x04, 0x03, EMPTY
#define Xmm5    0x05, 0x03, EMPTY
#define Xmm6    0x06, 0x03, EMPTY
#define Xmm7    0x07, 0x03, EMPTY
#define Xmm8    0x08, 0x03, EMPTY
#define Xmm9    0x09, 0x03, EMPTY
#define XmmA    0x0A, 0x03, EMPTY
#define XmmB    0x0B, 0x03, EMPTY
#define XmmC    0x0C, 0x03, EMPTY
#define XmmD    0x0D, 0x03, EMPTY
#define XmmE    0x0E, 0x03, EMPTY
#define XmmF    0x0F, 0x03, EMPTY

/******************************************************************************/
/**********************************   AVX   ***********************************/
/******************************************************************************/

/**************************   packed generic (AVX1)   *************************/

/* mov */

#define movpx_rr(RG, RM)                                                    \
        VEX(RXB(RG), RXB(RM),     0x0, 1, 1, 1) EMITB(0x28)                 \
        MRM(REG(RG), MOD(RM), REG(RM))

#define movpx_ld(RG, RM, DP)                                                \
    ADR VEX(RXB(RG), RXB(RM),     0x0, 1, 1, 1) EMITB(0x28)                 \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

#define movpx_st(RG, RM, DP)                                                \
    ADR VEX(RXB(RG), RXB(RM),     0x0, 1, 1, 1) EMITB(0x29)                 \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

#define adrpx_ld(RG, RM, DP) /* RG is a BASE reg, DP is SIMD-aligned */     \
    ADR REW(RXB(RG), RXB(RM)) EMITB(0x8D)                                   \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

/* and */

#define andpx_rr(RG, RM)                                                    \
        VEX(RXB(RG), RXB(RM), REN(RG), 1, 1, 1) EMITB(0x54)                 \
        MRM(REG(RG), MOD(RM), REG(RM))

#define andpx_ld(RG, RM, DP)                                                \
    ADR VEX(RXB(RG), RXB(RM), REN(RG), 1, 1, 1) EMITB(0x54)                 \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

/* ann */

#define annpx_rr(RG, RM)                                                    \
        VEX(RXB(RG), RXB(RM), REN(RG), 1, 1, 1) EMITB(0x55)                 \
        MRM(REG(RG), MOD(RM), REG(RM))

#define annpx_ld(RG, RM, DP)                                                \
    ADR VEX(RXB(RG), RXB(RM), REN(RG), 1, 1, 1) EMITB(0x55)                 \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

/* orr */

#define orrpx_rr(RG, RM)                                                    \
        VEX(RXB(RG), RXB(RM), REN(RG), 1, 1, 1) EMITB(0x56)                 \
        MRM(REG(RG), MOD(RM), REG(RM))

#define orrpx_ld(RG, RM, DP)                                                \
    ADR VEX(RXB(RG), RXB(RM), REN(RG), 1, 1, 1) EMITB(0x56)                 \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

/* xor */

#define xorpx_rr(RG, RM)                                                    \
        VEX(RXB(RG), RXB(RM), REN(RG), 1, 1, 1) EMITB(0x57)                 \
        MRM(REG(RG), MOD(RM), REG(RM))

#define xorpx_ld(RG, RM, DP)                                                \
    ADR VEX(RXB(RG), RXB(RM), REN(RG), 1, 1, 1) EMITB(0x57)                 \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

/**************   packed double precision floating point (AVX1)   *************/

/* add */

#define addps_rr(RG, RM)                                                    \
        VEX(RXB(RG), RXB(RM), REN(RG), 1, 1, 1) EMITB(0x58)                 \
        MRM(REG(RG), MOD(RM), REG(RM))

#define addps_ld(RG, RM, DP)                                                \
    ADR VEX(RXB(RG), RXB(RM), REN(RG), 1, 1, 1) EMITB(0x58)                 \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

/* sub */

#define subps_rr(RG, RM)                                                    \
        VEX(RXB(RG), RXB(RM), REN(RG), 1, 1, 1) EMITB(0x5C)                 \
        MRM(REG(RG), MOD(RM), REG(RM))

#define subps_ld(RG, RM, DP)                                                \
    ADR VEX(RXB(RG), RXB(RM), REN(RG), 1, 1, 1) EMITB(0x5C)                 \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

/* mul */

#define mulps_rr(RG, RM)                                                    \
        VEX(RXB(RG), RXB(RM), REN(RG), 1, 1, 1) EMITB(0x59)                 \
        MRM(REG(RG), MOD(RM), REG(RM))

#define mulps_ld(RG, RM, DP)                                                \
    ADR VEX(RXB(RG), RXB(RM), REN(RG), 1, 1, 1) EMITB(0x59)                 \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

/* div */

#define divps_rr(RG, RM)                                                    \
        VEX(RXB(RG), RXB(RM), REN(RG), 1, 1, 1) EMITB(0x5E)                 \
        MRM(REG(RG), MOD(RM), REG(RM))

#define divps_ld(RG, RM, DP)                                                \
    ADR VEX(RXB(RG), RXB(RM), REN(RG), 1, 1, 1) EMITB(0x5E)                 \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

/* sqr */

#define sqrps_rr(RG, RM)                                                    \
        VEX(RXB(RG), RXB(RM),     0x0, 1, 1, 1) EMITB(0x51)                 \
        MRM(REG(RG), MOD(RM), REG(RM))

#define sqrps_ld(RG, RM, DP)                                                \
    ADR VEX(RXB(RG), RXB(RM),     0x0, 1, 1, 1) EMITB(0x51)                 \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

/* cbr */

        /* cbe, cbs, cbr defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* rcp
 * accuracy/behavior may vary across supported targets, use accordingly */

#if RT_SIMD_COMPAT_RCP == 0

#define rceps_rr(RG, RM)                                                    \
        movpx_st(W(RM), Mebp, inf_SCR01(0))                                 \
        movpx_ld(W(RG), Mebp, inf_GPC01)                                    \
        divps_ld(W(RG), Mebp, inf_SCR01(0))

#define rcsps_rr(RG, RM) /* destroys RM */

#endif /* RT_SIMD_COMPAT_RCP */

        /* rcp defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* rsq
 * accuracy/behavior may vary across supported targets, use accordingly */

#if RT_SIMD_COMPAT_RSQ == 0

#define rseps_rr(RG, RM)                                                    \
        sqrps_rr(W(RG), W(RM))                                              \
        movpx_st(W(RG), Mebp, inf_SCR01(0))                                 \
        movpx_ld(W(RG), Mebp, inf_GPC01)                                    \
        divps_ld(W(RG), Mebp, inf_SCR01(0))

#define rssps_rr(RG, RM) /* destroys RM */

#endif /* RT_SIMD_COMPAT_RSQ */

        /* rsq defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* min */

#define minps_rr(RG, RM)                                                    \
        VEX(RXB(RG), RXB(RM), REN(RG), 1, 1, 1) EMITB(0x5D)                 \
        MRM(REG(RG), MOD(RM), REG(RM))

#define minps_ld(RG, RM, DP)                                                \
    ADR VEX(RXB(RG), RXB(RM), REN(RG), 1, 1, 1) EMITB(0x5D)                 \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

/* max */

#define maxps_rr(RG, RM)                                                    \
        VEX(RXB(RG), RXB(RM), REN(RG), 1, 1, 1) EMITB(0x5F)                 \
        MRM(REG(RG), MOD(RM), REG(RM))

#define maxps_ld(RG, RM, DP)                                                \
    ADR VEX(RXB(RG), RXB(RM), REN(RG), 1, 1, 1) EMITB(0x5F)                 \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

/* cmp */

#define ceqps_rr(RG, RM)                                                    \
        VEX(RXB(RG), RXB(RM), REN(RG), 1, 1, 1) EMITB(0xC2)                 \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x00))

#define ceqps_ld(RG, RM, DP)                                                \
    ADR VEX(RXB(RG), RXB(RM), REN(RG), 1, 1, 1) EMITB(0xC2)                 \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMITB(0x00))

#define cneps_rr(RG, RM)                                                    \
        VEX(RXB(RG), RXB(RM), REN(RG), 1, 1, 1) EMITB(0xC2)                 \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x04))

#define cneps_ld(RG, RM, DP)                                                \
    ADR VEX(RXB(RG), RXB(RM), REN(RG), 1, 1, 1) EMITB(0xC2)                 \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMITB(0x04))

#define cltps_rr(RG, RM)                                                    \
        VEX(RXB(RG), RXB(RM), REN(RG), 1, 1, 1) EMITB(0xC2)                 \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x01))

#define cltps_ld(RG, RM, DP)                                                \
    ADR VEX(RXB(RG), RXB(RM), REN(RG), 1, 1, 1) EMITB(0xC2)                 \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMITB(0x01))

#define cleps_rr(RG, RM)                                                    \
        VEX(RXB(RG), RXB(RM), REN(RG), 1, 1, 1) EMITB(0xC2)                 \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x02))

#define cleps_ld(RG, RM, DP)                                                \
    ADR VEX(RXB(RG), RXB(RM), REN(RG), 1, 1, 1) EMITB(0xC2)                 \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMITB(0x02))

#define cgtps_rr(RG, RM)                                                    \
        VEX(RXB(RG), RXB(RM), REN(RG), 1, 1, 1) EMITB(0xC2)                 \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x06))

#define cgtps_ld(RG, RM, DP)                                                \
    ADR VEX(RXB(RG), RXB(RM), REN(RG), 1, 1, 1) EMITB(0xC2)                 \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMITB(0x06))

#define cgeps_rr(RG, RM)                                                    \
        VEX(RXB(RG), RXB(RM), REN(RG), 1, 1, 1) EMITB(0xC2)                 \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x05))

#define cgeps_ld(RG, RM, DP)                                                \
    ADR VEX(RXB(RG), RXB(RM), REN(RG), 1, 1, 1) EMITB(0xC2)                 \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMITB(0x05))

/**************************   packed integer (x87)   **************************/

#define fpuxs_ld(RM, DP) /* not portable, do not use outside */             \
    ADR REX(0,       RXB(RM)) EMITB(0xDD)                                   \
        MRM(0x00,    MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

#define fpuxs_st(RM, DP) /* not portable, do not use outside */             \
    ADR REX(0,       RXB(RM)) EMITB(0xDD)                                   \
        MRM(0x03,    MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

#define fpuxn_ld(RM, DP) /* not portable, do not use outside */             \
    ADR REX(0,       RXB(RM)) EMITB(0xDF)                                   \
        MRM(0x05,    MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

#define fpuxn_st(RM, DP) /* not portable, do not use outside */             \
    ADR REX(0,       RXB(RM)) EMITB(0xDF)                                   \
        MRM(0x07,    MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

#define fpuxt_st(RM, DP) /* not portable, do not use outside */             \
    ADR REX(0,       RXB(RM)) EMITB(0xDD)                                   \
        MRM(0x01,    MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

#define fpucw_ld(RM, DP) /* not portable, do not use outside */             \
    ADR REX(0,       RXB(RM)) EMITB(0xD9)                                   \
        MRM(0x05,    MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

#define fpucw_st(RM, DP) /* not portable, do not use outside */             \
FWT ADR REX(0,       RXB(RM)) EMITB(0xD9)                                   \
        MRM(0x07,    MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

/* cvz (fp-to-signed-int)
 * rounding mode is encoded directly (can be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 64-bit signed int range */

#define rnzps_rr(RG, RM)     /* round towards zero */                       \
        VEX(RXB(RG), RXB(RM),     0x0, 1, 1, 3) EMITB(0x09)                 \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x03))

#define rnzps_ld(RG, RM, DP) /* round towards zero */                       \
    ADR VEX(RXB(RG), RXB(RM),     0x0, 1, 1, 3) EMITB(0x09)                 \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMITB(0x03))

#define cvzps_rr(RG, RM)     /* round towards zero */                       \
        movpx_st(W(RM), Mebp, inf_SCR01(0))                                 \
        fpuxs_ld(Mebp,  inf_SCR01(0x00))                                    \
        fpuxt_st(Mebp,  inf_SCR01(0x00))                                    \
        fpuxs_ld(Mebp,  inf_SCR01(0x08))                                    \
        fpuxt_st(Mebp,  inf_SCR01(0x08))                                    \
        fpuxs_ld(Mebp,  inf_SCR01(0x10))                                    \
        fpuxt_st(Mebp,  inf_SCR01(0x10))                                    \
        fpuxs_ld(Mebp,  inf_SCR01(0x18))                                    \
        fpuxt_st(Mebp,  inf_SCR01(0x18))                                    \
        movpx_ld(W(RG), Mebp, inf_SCR01(0))

#define cvzps_ld(RG, RM, DP) /* round towards zero */                       \
        movpx_ld(W(RG), W(RM), W(DP))                                       \
        cvzps_rr(W(RG), W(RG))

/* cvp (fp-to-signed-int)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 64-bit signed int range */

#define rnpps_rr(RG, RM)     /* round towards +inf */                       \
        VEX(RXB(RG), RXB(RM),     0x0, 1, 1, 3) EMITB(0x09)                 \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x02))

#define rnpps_ld(RG, RM, DP) /* round towards +inf */                       \
    ADR VEX(RXB(RG), RXB(RM),     0x0, 1, 1, 3) EMITB(0x09)                 \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMITB(0x02))

#define cvpps_rr(RG, RM)     /* round towards +inf */                       \
        rnpps_rr(W(RG), W(RM))                                              \
        cvzps_rr(W(RG), W(RG))

#define cvpps_ld(RG, RM, DP) /* round towards +inf */                       \
        rnpps_ld(W(RG), W(RM), W(DP))                                       \
        cvzps_rr(W(RG), W(RG))

/* cvm (fp-to-signed-int)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 64-bit signed int range */

#define rnmps_rr(RG, RM)     /* round towards -inf */                       \
        VEX(RXB(RG), RXB(RM),     0x0, 1, 1, 3) EMITB(0x09)                 \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x01))

#define rnmps_ld(RG, RM, DP) /* round towards -inf */                       \
    ADR VEX(RXB(RG), RXB(RM),     0x0, 1, 1, 3) EMITB(0x09)                 \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMITB(0x01))

#define cvmps_rr(RG, RM)     /* round towards -inf */                       \
        rnmps_rr(W(RG), W(RM))                                              \
        cvzps_rr(W(RG), W(RG))

#define cvmps_ld(RG, RM, DP) /* round towards -inf */                       \
        rnmps_ld(W(RG), W(RM), W(DP))                                       \
        cvzps_rr(W(RG), W(RG))

/* cvn (fp-to-signed-int)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 64-bit signed int range */

#define rnnps_rr(RG, RM)     /* round towards near */                       \
        VEX(RXB(RG), RXB(RM),     0x0, 1, 1, 3) EMITB(0x09)                 \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x00))

#define rnnps_ld(RG, RM, DP) /* round towards near */                       \
    ADR VEX(RXB(RG), RXB(RM),     0x0, 1, 1, 3) EMITB(0x09)                 \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMITB(0x00))

#define cvnps_rr(RG, RM)     /* round towards near */                       \
        movpx_st(W(RM), Mebp, inf_SCR01(0))                                 \
        fpuxs_ld(Mebp,  inf_SCR01(0x00))                                    \
        fpuxn_st(Mebp,  inf_SCR01(0x00))                                    \
        fpuxs_ld(Mebp,  inf_SCR01(0x08))                                    \
        fpuxn_st(Mebp,  inf_SCR01(0x08))                                    \
        fpuxs_ld(Mebp,  inf_SCR01(0x10))                                    \
        fpuxn_st(Mebp,  inf_SCR01(0x10))                                    \
        fpuxs_ld(Mebp,  inf_SCR01(0x18))                                    \
        fpuxn_st(Mebp,  inf_SCR01(0x18))                                    \
        movpx_ld(W(RG), Mebp, inf_SCR01(0))

#define cvnps_ld(RG, RM, DP) /* round towards near */                       \
        movpx_ld(W(RG), W(RM), W(DP))                                       \
        cvnps_rr(W(RG), W(RG))

/* cvn (signed-int-to-fp)
 * rounding mode encoded directly (cannot be used in FCTRL blocks) */

#define cvnpn_rr(RG, RM)     /* round towards near */                       \
        movpx_st(W(RM), Mebp, inf_SCR01(0))                                 \
        fpuxn_ld(Mebp,  inf_SCR01(0x00))                                    \
        fpuxs_st(Mebp,  inf_SCR01(0x00))                                    \
        fpuxn_ld(Mebp,  inf_SCR01(0x08))                                    \
        fpuxs_st(Mebp,  inf_SCR01(0x08))                                    \
        fpuxn_ld(Mebp,  inf_SCR01(0x10))                                    \
        fpuxs_st(Mebp,  inf_SCR01(0x10))                                    \
        fpuxn_ld(Mebp,  inf_SCR01(0x18))                                    \
        fpuxs_st(Mebp,  inf_SCR01(0x18))                                    \
        movpx_ld(W(RG), Mebp, inf_SCR01(0))

#define cvnpn_ld(RG, RM, DP) /* round towards near */                       \
        movpx_ld(W(RG), W(RM), W(DP))                                       \
        cvnpn_rr(W(RG), W(RG))

/**************************   packed integer (AVX1)   *************************/

#if (RT_256 < 2)

#define prmpx_rr(RG, RM, IM) /* not portable, do not use outside */         \
        VEX(RXB(RG), RXB(RM), REN(RG), 1, 1, 3) EMITB(0x46)                 \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IM)))

#define movlx_ld(RG, RM, DP) /* not portable, do not use outside */         \
    ADR VEX(RXB(RG), RXB(RM),     0x0, 0, 1, 1) EMITB(0x28)                 \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

#define movlx_st(RG, RM, DP) /* not portable, do not use outside */         \
    ADR VEX(RXB(RG), RXB(RM),     0x0, 0, 1, 1) EMITB(0x29)                 \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

/* add */

#define addlx_rr(RG, RM)     /* not portable, do not use outside */         \
        VEX(RXB(RG), RXB(RM), REN(RG), 0, 1, 1) EMITB(0xD4)                 \
        MRM(REG(RG), MOD(RM), REG(RM))

#define addpx_rr(RG, RM)                                                    \
        movpx_st(W(RG), Mebp, inf_SCR01(0))                                 \
        addlx_rr(W(RG), W(RM))                                              \
        movlx_st(W(RG), Mebp, inf_SCR01(0x00))                              \
        movlx_ld(W(RG), Mebp, inf_SCR01(0x10))                              \
        prmpx_rr(W(RM), W(RM), IB(1))                                       \
        addlx_rr(W(RG), W(RM))                                              \
        prmpx_rr(W(RM), W(RM), IB(1))                                       \
        movlx_st(W(RG), Mebp, inf_SCR01(0x10))                              \
        movpx_ld(W(RG), Mebp, inf_SCR01(0))

#define addlx_ld(RG, RM, DP) /* not portable, do not use outside */         \
    ADR VEX(RXB(RG), RXB(RM), REN(RG), 0, 1, 1) EMITB(0xD4)                 \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

#define addpx_ld(RG, RM, DP)                                                \
        movpx_st(W(RG), Mebp, inf_SCR01(0))                                 \
        movpx_ld(W(RG), W(RM), W(DP))                                       \
        movpx_st(W(RG), Mebp, inf_SCR02(0))                                 \
        movlx_ld(W(RG), Mebp, inf_SCR01(0x00))                              \
        addlx_ld(W(RG), Mebp, inf_SCR02(0x00))                              \
        movlx_st(W(RG), Mebp, inf_SCR01(0x00))                              \
        movlx_ld(W(RG), Mebp, inf_SCR01(0x10))                              \
        addlx_ld(W(RG), Mebp, inf_SCR02(0x10))                              \
        movlx_st(W(RG), Mebp, inf_SCR01(0x10))                              \
        movpx_ld(W(RG), Mebp, inf_SCR01(0))

/* sub */

#define sublx_rr(RG, RM)     /* not portable, do not use outside */         \
        VEX(RXB(RG), RXB(RM), REN(RG), 0, 1, 1) EMITB(0xFB)                 \
        MRM(REG(RG), MOD(RM), REG(RM))

#define subpx_rr(RG, RM)                                                    \
        movpx_st(W(RG), Mebp, inf_SCR01(0))                                 \
        sublx_rr(W(RG), W(RM))                                              \
        movlx_st(W(RG), Mebp, inf_SCR01(0x00))                              \
        movlx_ld(W(RG), Mebp, inf_SCR01(0x10))                              \
        prmpx_rr(W(RM), W(RM), IB(1))                                       \
        sublx_rr(W(RG), W(RM))                                              \
        prmpx_rr(W(RM), W(RM), IB(1))                                       \
        movlx_st(W(RG), Mebp, inf_SCR01(0x10))                              \
        movpx_ld(W(RG), Mebp, inf_SCR01(0))

#define sublx_ld(RG, RM, DP) /* not portable, do not use outside */         \
    ADR VEX(RXB(RG), RXB(RM), REN(RG), 0, 1, 1) EMITB(0xFB)                 \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

#define subpx_ld(RG, RM, DP)                                                \
        movpx_st(W(RG), Mebp, inf_SCR01(0))                                 \
        movpx_ld(W(RG), W(RM), W(DP))                                       \
        movpx_st(W(RG), Mebp, inf_SCR02(0))                                 \
        movlx_ld(W(RG), Mebp, inf_SCR01(0x00))                              \
        sublx_ld(W(RG), Mebp, inf_SCR02(0x00))                              \
        movlx_st(W(RG), Mebp, inf_SCR01(0x00))                              \
        movlx_ld(W(RG), Mebp, inf_SCR01(0x10))                              \
        sublx_ld(W(RG), Mebp, inf_SCR02(0x10))                              \
        movlx_st(W(RG), Mebp, inf_SCR01(0x10))                              \
        movpx_ld(W(RG), Mebp, inf_SCR01(0))

/* shl */

#define shllx_ri(RM, IM)     /* not portable, do not use outside */         \
        VEX(0,       RXB(RM), REN(RM), 0, 1, 1) EMITB(0x73)                 \
        MRM(0x06,    MOD(RM), REG(RM))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IM) & 0x3F))

#define shlpx_ri(RM, IM)                                                    \
        movpx_st(W(RM), Mebp, inf_SCR01(0))                                 \
        shllx_ri(W(RM), W(IM))                                              \
        movlx_st(W(RM), Mebp, inf_SCR01(0x00))                              \
        movlx_ld(W(RM), Mebp, inf_SCR01(0x10))                              \
        shllx_ri(W(RM), W(IM))                                              \
        movlx_st(W(RM), Mebp, inf_SCR01(0x10))                              \
        movpx_ld(W(RM), Mebp, inf_SCR01(0))

#define shllx_ld(RG, RM, DP) /* not portable, do not use outside */         \
    ADR VEX(RXB(RG), RXB(RM), REN(RG), 0, 1, 1) EMITB(0xF3)                 \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

#define shlpx_ld(RG, RM, DP) /* loads SIMD, uses 1 elem at given address */ \
        movpx_st(W(RG), Mebp, inf_SCR01(0))                                 \
        shllx_ld(W(RG), W(RM), W(DP))                                       \
        movlx_st(W(RG), Mebp, inf_SCR01(0x00))                              \
        movlx_ld(W(RG), Mebp, inf_SCR01(0x10))                              \
        shllx_ld(W(RG), W(RM), W(DP))                                       \
        movlx_st(W(RG), Mebp, inf_SCR01(0x10))                              \
        movpx_ld(W(RG), Mebp, inf_SCR01(0))

/* shr */

#define shrlx_ri(RM, IM)     /* not portable, do not use outside */         \
        VEX(0,       RXB(RM), REN(RM), 0, 1, 1) EMITB(0x73)                 \
        MRM(0x02,    MOD(RM), REG(RM))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IM) & 0x3F))

#define shrpx_ri(RM, IM)                                                    \
        movpx_st(W(RM), Mebp, inf_SCR01(0))                                 \
        shrlx_ri(W(RM), W(IM))                                              \
        movlx_st(W(RM), Mebp, inf_SCR01(0x00))                              \
        movlx_ld(W(RM), Mebp, inf_SCR01(0x10))                              \
        shrlx_ri(W(RM), W(IM))                                              \
        movlx_st(W(RM), Mebp, inf_SCR01(0x10))                              \
        movpx_ld(W(RM), Mebp, inf_SCR01(0))

#define shrlx_ld(RG, RM, DP) /* not portable, do not use outside */         \
    ADR VEX(RXB(RG), RXB(RM), REN(RG), 0, 1, 1) EMITB(0xD3)                 \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

#define shrpx_ld(RG, RM, DP) /* loads SIMD, uses 1 elem at given address */ \
        movpx_st(W(RG), Mebp, inf_SCR01(0))                                 \
        shrlx_ld(W(RG), W(RM), W(DP))                                       \
        movlx_st(W(RG), Mebp, inf_SCR01(0x00))                              \
        movlx_ld(W(RG), Mebp, inf_SCR01(0x10))                              \
        shrlx_ld(W(RG), W(RM), W(DP))                                       \
        movlx_st(W(RG), Mebp, inf_SCR01(0x10))                              \
        movpx_ld(W(RG), Mebp, inf_SCR01(0))

#define shrpn_ri(RM, IM)                                                    \
        movpx_st(W(RM), Mebp, inf_SCR01(0))                                 \
        shryn_mi(Mebp,  inf_SCR01(0x00), W(IM))                             \
        shryn_mi(Mebp,  inf_SCR01(0x08), W(IM))                             \
        shryn_mi(Mebp,  inf_SCR01(0x10), W(IM))                             \
        shryn_mi(Mebp,  inf_SCR01(0x18), W(IM))                             \
        movpx_ld(W(RM), Mebp, inf_SCR01(0))

#define shrpn_ld(RG, RM, DP) /* loads SIMD, uses 1 elem at given address */ \
        movpx_st(W(RG), Mebp, inf_SCR01(0))                                 \
        stack_st(Recx)                                                      \
        movyx_ld(Recx, W(RM), W(DP))                                        \
        shryn_mx(Mebp,  inf_SCR01(0x00))                                    \
        shryn_mx(Mebp,  inf_SCR01(0x08))                                    \
        shryn_mx(Mebp,  inf_SCR01(0x10))                                    \
        shryn_mx(Mebp,  inf_SCR01(0x18))                                    \
        stack_ld(Recx)                                                      \
        movpx_ld(W(RG), Mebp, inf_SCR01(0))

/**************************   packed integer (AVX2)   *************************/

#else /* RT_256 >= 2 */

/* add */

#define addpx_rr(RG, RM)                                                    \
        VEX(RXB(RG), RXB(RM), REN(RG), 1, 1, 1) EMITB(0xD4)                 \
        MRM(REG(RG), MOD(RM), REG(RM))

#define addpx_ld(RG, RM, DP)                                                \
    ADR VEX(RXB(RG), RXB(RM), REN(RG), 1, 1, 1) EMITB(0xD4)                 \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

/* sub */

#define subpx_rr(RG, RM)                                                    \
        VEX(RXB(RG), RXB(RM), REN(RG), 1, 1, 1) EMITB(0xFB)                 \
        MRM(REG(RG), MOD(RM), REG(RM))

#define subpx_ld(RG, RM, DP)                                                \
    ADR VEX(RXB(RG), RXB(RM), REN(RG), 1, 1, 1) EMITB(0xFB)                 \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

/* shl */

#define shlpx_ri(RM, IM)                                                    \
        VEX(0,       RXB(RM), REN(RM), 1, 1, 1) EMITB(0x73)                 \
        MRM(0x06,    MOD(RM), REG(RM))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IM) & 0x3F))

#define shlpx_ld(RG, RM, DP) /* loads SIMD, uses 1 elem at given address */ \
    ADR VEX(RXB(RG), RXB(RM), REN(RG), 1, 1, 1) EMITB(0xF3)                 \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

/* shr */

#define shrpx_ri(RM, IM)                                                    \
        VEX(0,       RXB(RM), REN(RM), 1, 1, 1) EMITB(0x73)                 \
        MRM(0x02,    MOD(RM), REG(RM))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IM) & 0x3F))

#define shrpx_ld(RG, RM, DP) /* loads SIMD, uses 1 elem at given address */ \
    ADR VEX(RXB(RG), RXB(RM), REN(RG), 1, 1, 1) EMITB(0xD3)                 \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

#define shrpn_ri(RM, IM)                                                    \
        movpx_st(W(RM), Mebp, inf_SCR01(0))                                 \
        shryn_mi(Mebp,  inf_SCR01(0x00), W(IM))                             \
        shryn_mi(Mebp,  inf_SCR01(0x08), W(IM))                             \
        shryn_mi(Mebp,  inf_SCR01(0x10), W(IM))                             \
        shryn_mi(Mebp,  inf_SCR01(0x18), W(IM))                             \
        movpx_ld(W(RM), Mebp, inf_SCR01(0))

#define shrpn_ld(RG, RM, DP) /* loads SIMD, uses 1 elem at given address */ \
        movpx_st(W(RG), Mebp, inf_SCR01(0))                                 \
        stack_st(Recx)                                                      \
        movyx_ld(Recx, W(RM), W(DP))                                        \
        shryn_mx(Mebp,  inf_SCR01(0x00))                                    \
        shryn_mx(Mebp,  inf_SCR01(0x08))                                    \
        shryn_mx(Mebp,  inf_SCR01(0x10))                                    \
        shryn_mx(Mebp,  inf_SCR01(0x18))                                    \
        stack_ld(Recx)                                                      \
        movpx_ld(W(RG), Mebp, inf_SCR01(0))

#endif /* RT_256 >= 2 */

/**************************   helper macros (AVX1)   **************************/

/* simd mask
 * compatibility with AVX-512 and ARM-SVE can be achieved by always keeping
 * one hidden SIMD register holding all 1s and using one hidden mask register
 * first in cmp (c**ps) to produce compatible result in target SIMD register
 * then in CHECK_MASK to facilitate branching on a given condition value */

#define RT_SIMD_MASK_NONE       0x00    /* none satisfy the condition */
#define RT_SIMD_MASK_FULL       0x0F    /*  all satisfy the condition */

#define movsn_rr(RG, RM) /* not portable, do not use outside */             \
        VEX(RXB(RG), RXB(RM),     0x0, 1, 1, 1) EMITB(0x50)                 \
        MRM(REG(RG), MOD(RM), REG(RM))

#define CHECK_MASK(lb, mask, RG) /* destroys Reax */                        \
        movsn_rr(Reax, W(RG))                                               \
        cmpwx_ri(Reax, IH(RT_SIMD_MASK_##mask))                             \
        jeqxx_lb(lb)

/* simd mode
 * set via FCTRL macros, *_F for faster non-IEEE mode (optional on MIPS/Power),
 * original FCTRL blocks (FCTRL_ENTER/FCTRL_LEAVE) are defined in rtbase.h
 * NOTE: ARMv7 always uses ROUNDN non-IEEE mode for SIMD fp-arithmetic,
 * while fp<->int conversion takes ROUND* into account via VFP fallback */

#if RT_SIMD_FLUSH_ZERO == 0

#define RT_SIMD_MODE_ROUNDN     0x00    /* round towards near */
#define RT_SIMD_MODE_ROUNDM     0x01    /* round towards -inf */
#define RT_SIMD_MODE_ROUNDP     0x02    /* round towards +inf */
#define RT_SIMD_MODE_ROUNDZ     0x03    /* round towards zero */

#else /* RT_SIMD_FLUSH_ZERO */

#define RT_SIMD_MODE_ROUNDN     0x04    /* round towards near */
#define RT_SIMD_MODE_ROUNDM     0x05    /* round towards -inf */
#define RT_SIMD_MODE_ROUNDP     0x06    /* round towards +inf */
#define RT_SIMD_MODE_ROUNDZ     0x07    /* round towards zero */

#endif /* RT_SIMD_FLUSH_ZERO */

#define RT_SIMD_MODE_ROUNDN_F   0x04    /* round towards near */
#define RT_SIMD_MODE_ROUNDM_F   0x05    /* round towards -inf */
#define RT_SIMD_MODE_ROUNDP_F   0x06    /* round towards +inf */
#define RT_SIMD_MODE_ROUNDZ_F   0x07    /* round towards zero */

#define mxcsr_ld(RM, DP) /* not portable, do not use outside */             \
    ADR VEX(0,       RXB(RM),     0x0, 0, 0, 1) EMITB(0xAE)                 \
        MRM(0x02,    MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

#define mxcsr_st(RM, DP) /* not portable, do not use outside */             \
    ADR VEX(0,       RXB(RM),     0x0, 0, 0, 1) EMITB(0xAE)                 \
        MRM(0x03,    MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

#if RT_SIMD_FAST_FCTRL == 0

#define FCTRL_SET(mode)   /* sets given mode into fp control register */    \
        movwx_mi(Mebp, inf_SCR00, IH(RT_SIMD_MODE_##mode << 13 | 0x1F80))   \
        mxcsr_ld(Mebp, inf_SCR00)                                           \

#define FCTRL_RESET()     /* resumes default mode (ROUNDN) upon leave */    \
        mxcsr_ld(Mebp, inf_FCTRL((RT_SIMD_MODE_ROUNDN&3)*4))

#else /* RT_SIMD_FAST_FCTRL */

#define FCTRL_SET(mode)   /* sets given mode into fp control register */    \
        mxcsr_ld(Mebp, inf_FCTRL((RT_SIMD_MODE_##mode&3)*4))

#define FCTRL_RESET()     /* resumes default mode (ROUNDN) upon leave */    \
        mxcsr_ld(Mebp, inf_FCTRL((RT_SIMD_MODE_ROUNDN&3)*4))

#endif /* RT_SIMD_FAST_FCTRL */

/* cvt (fp-to-signed-int)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: ROUNDZ is not supported on pre-VSX Power systems, use cvz
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 64-bit signed int range */

#define rndps_rr(RG, RM)                                                    \
        VEX(RXB(RG), RXB(RM),     0x0, 1, 1, 3) EMITB(0x09)                 \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x04))

#define rndps_ld(RG, RM, DP)                                                \
    ADR VEX(RXB(RG), RXB(RM),     0x0, 1, 1, 3) EMITB(0x09)                 \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMITB(0x04))

#define cvtps_rr(RG, RM)                                                    \
        rndps_rr(W(RG), W(RM))                                              \
        cvzps_rr(W(RG), W(RG))

#define cvtps_ld(RG, RM, DP)                                                \
        rndps_ld(W(RG), W(RM), W(DP))                                       \
        cvzps_rr(W(RG), W(RG))

/* cvt (signed-int-to-fp)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: only default ROUNDN is supported on pre-VSX Power systems */

#define cvtpn_rr(RG, RM)                                                    \
        fpucw_st(Mebp,  inf_SCR00)                                          \
        mxcsr_st(Mebp,  inf_SCR02(0))                                       \
        shrwx_mi(Mebp,  inf_SCR02(0), IB(3))                                \
        andwx_mi(Mebp,  inf_SCR02(0), IH(0x0C00))                           \
        orrwx_mi(Mebp,  inf_SCR02(0), IB(0x7F))                             \
        fpucw_ld(Mebp,  inf_SCR02(0))                                       \
        cvnpn_rr(W(RG), W(RM))                                              \
        fpucw_ld(Mebp,  inf_SCR00)

#define cvtpn_ld(RG, RM, DP)                                                \
        movpx_ld(W(RG), W(RM), W(DP))                                       \
        cvtpn_rr(W(RG), W(RG))

/* cvr (fp-to-signed-int)
 * rounding mode is encoded directly (cannot be used in FCTRL blocks)
 * NOTE: on targets with full-IEEE SIMD fp-arithmetic the ROUND*_F mode
 * isn't always taken into account when used within full-IEEE ASM block
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 64-bit signed int range */

#define rnrps_rr(RG, RM, mode)                                              \
        VEX(RXB(RG), RXB(RM),     0x0, 1, 1, 3) EMITB(0x09)                 \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(RT_SIMD_MODE_##mode&3))

#define cvrps_rr(RG, RM, mode)                                              \
        rnrps_rr(W(RG), W(RM), mode)                                        \
        cvzps_rr(W(RG), W(RG))

/* mmv
 * uses Xmm0 implicitly as a mask register */

#define mmvpx_ld(RG, RM, DP) /* not portable, use conditionally */          \
    ADR VEX(RXB(RG), RXB(RM),     0x0, 1, 1, 2) EMITB(0x2D)                 \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

#define mmvpx_st(RG, RM, DP) /* not portable, use conditionally */          \
    ADR VEX(RXB(RG), RXB(RM),     0x0, 1, 1, 2) EMITB(0x2F)                 \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

/* sregs */

#define sregs_sa() /* save all SIMD regs, destroys Reax */                  \
        movxx_ld(Reax, Mebp, inf_REGS)                                      \
        movpx_st(Xmm0, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH*8))                                 \
        movpx_st(Xmm1, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH*8))                                 \
        movpx_st(Xmm2, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH*8))                                 \
        movpx_st(Xmm3, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH*8))                                 \
        movpx_st(Xmm4, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH*8))                                 \
        movpx_st(Xmm5, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH*8))                                 \
        movpx_st(Xmm6, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH*8))                                 \
        movpx_st(Xmm7, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH*8))                                 \
        movpx_st(Xmm8, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH*8))                                 \
        movpx_st(Xmm9, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH*8))                                 \
        movpx_st(XmmA, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH*8))                                 \
        movpx_st(XmmB, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH*8))                                 \
        movpx_st(XmmC, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH*8))                                 \
        movpx_st(XmmD, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH*8))                                 \
        movpx_st(XmmE, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH*8))                                 \
        movpx_st(XmmF, Oeax, PLAIN)

#define sregs_la() /* load all SIMD regs, destroys Reax */                  \
        movxx_ld(Reax, Mebp, inf_REGS)                                      \
        movpx_ld(Xmm0, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH*8))                                 \
        movpx_ld(Xmm1, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH*8))                                 \
        movpx_ld(Xmm2, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH*8))                                 \
        movpx_ld(Xmm3, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH*8))                                 \
        movpx_ld(Xmm4, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH*8))                                 \
        movpx_ld(Xmm5, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH*8))                                 \
        movpx_ld(Xmm6, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH*8))                                 \
        movpx_ld(Xmm7, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH*8))                                 \
        movpx_ld(Xmm8, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH*8))                                 \
        movpx_ld(Xmm9, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH*8))                                 \
        movpx_ld(XmmA, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH*8))                                 \
        movpx_ld(XmmB, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH*8))                                 \
        movpx_ld(XmmC, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH*8))                                 \
        movpx_ld(XmmD, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH*8))                                 \
        movpx_ld(XmmE, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH*8))                                 \
        movpx_ld(XmmF, Oeax, PLAIN)

#endif /* RT_256 */

#endif /* RT_SIMD_CODE */

#endif /* RT_RTARCH_X64_256_H */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/