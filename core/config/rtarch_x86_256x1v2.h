/******************************************************************************/
/* Copyright (c) 2013-2020 VectorChief (at github, bitbucket, sourceforge)    */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#ifndef RT_RTARCH_X86_256X1V2_H
#define RT_RTARCH_X86_256X1V2_H

#include "rtarch_x86.h"

#define RT_SIMD_REGS_256        8

/******************************************************************************/
/*********************************   LEGEND   *********************************/
/******************************************************************************/

/*
 * rtarch_x86_256x1v2.h: Implementation of x86 fp32 AVX1/2 instructions.
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
 * cmdi*_** - applies [cmd] to 32-bit SIMD element args, packed-128-bit
 * cmdj*_** - applies [cmd] to 64-bit SIMD element args, packed-128-bit
 * cmdl*_** - applies [cmd] to L-size SIMD element args, packed-128-bit
 *
 * cmdc*_** - applies [cmd] to 32-bit SIMD element args, packed-256-bit
 * cmdd*_** - applies [cmd] to 64-bit SIMD element args, packed-256-bit
 * cmdf*_** - applies [cmd] to L-size SIMD element args, packed-256-bit
 *
 * cmdo*_** - applies [cmd] to 32-bit SIMD element args, packed-var-len
 * cmdp*_** - applies [cmd] to L-size SIMD element args, packed-var-len
 * cmdq*_** - applies [cmd] to 64-bit SIMD element args, packed-var-len
 *
 * cmd*x_** - applies [cmd] to [p]acked unsigned integer args, [x] - default
 * cmd*n_** - applies [cmd] to [p]acked   signed integer args, [n] - negatable
 * cmd*s_** - applies [cmd] to [p]acked floating point   args, [s] - scalable
 *
 * The cmdp*_** (rtconf.h) instructions are intended for SPMD programming model
 * and can be configured to work with 32/64-bit data elements (fp+int).
 * In this model data paths are fixed-width, BASE and SIMD data elements are
 * width-compatible, code path divergence is handled via mkj**_** pseudo-ops.
 * Matching element-sized BASE subset cmdy*_** is defined in rtconf.h as well.
 *
 * Note, when using fixed-data-size 128/256-bit SIMD subsets simultaneously
 * upper 128-bit halves of full 256-bit SIMD registers may end up undefined.
 * On RISC targets they remain unchanged, while on x86-AVX they are zeroed.
 * This happens when registers written in 128-bit subset are then used/read
 * from within 256-bit subset. The same rule applies to mixing with 512-bit
 * and wider vectors. Use of scalars may leave respective vector registers
 * undefined, as seen from the perspective of any particular vector subset.
 *
 * 256-bit vectors used with wider subsets may not be compatible with regards
 * to memory loads/stores when mixed in the code. It means that data loaded
 * with wider vector and stored within 256-bit subset at the same address may
 * result in changing the initial representation in memory. The same can be
 * said about mixing vector and scalar subsets. Scalars can be completely
 * detached on some architectures. Use elm*x_st to store 1st vector element.
 * 128-bit vectors should be memory-compatible with any wider vector subset.
 *
 * Handling of NaNs in the floating point pipeline may not be consistent
 * across different architectures. Avoid NaNs entering the data flow by using
 * masking or control flow instructions. Apply special care when dealing with
 * floating point compare and min/max input/output. The result of floating point
 * compare instructions can be considered a -QNaN, though it is also interpreted
 * as integer -1 and is often treated as a mask. Most arithmetic instructions
 * should propagate QNaNs unchanged, however this behavior hasn't been verified.
 *
 * Interpretation of instruction parameters:
 *
 * upper-case params have triplet structure and require W to pass-forward
 * lower-case params are singular and can be used/passed as such directly
 *
 * XD - SIMD register serving as destination only, if present
 * XG - SIMD register serving as destination and first source
 * XS - SIMD register serving as second source (first if any)
 * XT - SIMD register serving as third source (second if any)
 *
 * RD - BASE register serving as destination only, if present
 * RG - BASE register serving as destination and first source
 * RS - BASE register serving as second source (first if any)
 * RT - BASE register serving as third source (second if any)
 *
 * MD - BASE addressing mode (Oeax, M***, I***) (memory-dest)
 * MG - BASE addressing mode (Oeax, M***, I***) (memory-dsrc)
 * MS - BASE addressing mode (Oeax, M***, I***) (memory-src2)
 * MT - BASE addressing mode (Oeax, M***, I***) (memory-src3)
 *
 * DD - displacement value (DP, DF, DG, DH, DV) (memory-dest)
 * DG - displacement value (DP, DF, DG, DH, DV) (memory-dsrc)
 * DS - displacement value (DP, DF, DG, DH, DV) (memory-src2)
 * DT - displacement value (DP, DF, DG, DH, DV) (memory-src3)
 *
 * IS - immediate value (is used as a second or first source)
 * IT - immediate value (is used as a third or second source)
 */

/******************************************************************************/
/********************************   INTERNAL   ********************************/
/******************************************************************************/

#if (defined RT_SIMD_CODE)

#if (RT_256X1 >= 1 && RT_256X1 <= 2)

#ifndef RT_RTARCH_X86_128X1V8_H
#undef  RT_128X1
#define RT_128X1  (8 + (RT_256X1 == 2)*24)
#include "rtarch_x86_128x1v8.h"
#endif /* RT_RTARCH_X86_128X1V8_H */

/******************************************************************************/
/********************************   EXTERNAL   ********************************/
/******************************************************************************/

/******************************************************************************/
/**********************************   SIMD   **********************************/
/******************************************************************************/

/* elm (D = S), store first SIMD element with natural alignment
 * allows to decouple scalar subset from SIMD where appropriate */

#define elmcx_st(XS, MD, DD) /* 1st elem as in mem with SIMD load/store */  \
        elmix_st(W(XS), W(MD), W(DD))

/***************   packed single-precision generic move/logic   ***************/

/* mov (D = S) */

#define movcx_rr(XD, XS)                                                    \
        V2X(0x00,    1, 0) EMITB(0x28)                                      \
        MRM(REG(XD), MOD(XS), REG(XS))

#define movcx_ld(XD, MS, DS)                                                \
        V2X(0x00,    1, 0) EMITB(0x28)                                      \
        MRM(REG(XD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define movcx_st(XS, MD, DD)                                                \
        V2X(0x00,    1, 0) EMITB(0x29)                                      \
        MRM(REG(XS), MOD(MD), REG(MD))                                      \
        AUX(SIB(MD), CMD(DD), EMPTY)

/* mmv (G = G mask-merge S) where (mask-elem: 0 keeps G, -1 picks S)
 * uses Xmm0 implicitly as a mask register, destroys Xmm0, 0-masked XS elems */

#define mmvcx_rr(XG, XS)                                                    \
        VEX(REG(XG), 1, 1, 3) EMITB(0x4A)                                   \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x00))

#define mmvcx_ld(XG, MS, DS)                                                \
        VEX(REG(XG), 1, 1, 3) EMITB(0x4A)                                   \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMITB(0x00))

#define mmvcx_st(XS, MG, DG)                                                \
        VEX(0x00,    1, 1, 2) EMITB(0x2E)                                   \
        MRM(REG(XS), MOD(MG), REG(MG))                                      \
        AUX(SIB(MG), CMD(DG), EMPTY)

/* and (G = G & S), (D = S & T) if (#D != #T) */

#define andcx_rr(XG, XS)                                                    \
        andcx3rr(W(XG), W(XG), W(XS))

#define andcx_ld(XG, MS, DS)                                                \
        andcx3ld(W(XG), W(XG), W(MS), W(DS))

#define andcx3rr(XD, XS, XT)                                                \
        V2X(REG(XS), 1, 0) EMITB(0x54)                                      \
        MRM(REG(XD), MOD(XT), REG(XT))

#define andcx3ld(XD, XS, MT, DT)                                            \
        V2X(REG(XS), 1, 0) EMITB(0x54)                                      \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* ann (G = ~G & S), (D = ~S & T) if (#D != #T) */

#define anncx_rr(XG, XS)                                                    \
        anncx3rr(W(XG), W(XG), W(XS))

#define anncx_ld(XG, MS, DS)                                                \
        anncx3ld(W(XG), W(XG), W(MS), W(DS))

#define anncx3rr(XD, XS, XT)                                                \
        V2X(REG(XS), 1, 0) EMITB(0x55)                                      \
        MRM(REG(XD), MOD(XT), REG(XT))

#define anncx3ld(XD, XS, MT, DT)                                            \
        V2X(REG(XS), 1, 0) EMITB(0x55)                                      \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* orr (G = G | S), (D = S | T) if (#D != #T) */

#define orrcx_rr(XG, XS)                                                    \
        orrcx3rr(W(XG), W(XG), W(XS))

#define orrcx_ld(XG, MS, DS)                                                \
        orrcx3ld(W(XG), W(XG), W(MS), W(DS))

#define orrcx3rr(XD, XS, XT)                                                \
        V2X(REG(XS), 1, 0) EMITB(0x56)                                      \
        MRM(REG(XD), MOD(XT), REG(XT))

#define orrcx3ld(XD, XS, MT, DT)                                            \
        V2X(REG(XS), 1, 0) EMITB(0x56)                                      \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* orn (G = ~G | S), (D = ~S | T) if (#D != #T) */

#define orncx_rr(XG, XS)                                                    \
        notcx_rx(W(XG))                                                     \
        orrcx_rr(W(XG), W(XS))

#define orncx_ld(XG, MS, DS)                                                \
        notcx_rx(W(XG))                                                     \
        orrcx_ld(W(XG), W(MS), W(DS))

#define orncx3rr(XD, XS, XT)                                                \
        notcx_rr(W(XD), W(XS))                                              \
        orrcx_rr(W(XD), W(XT))

#define orncx3ld(XD, XS, MT, DT)                                            \
        notcx_rr(W(XD), W(XS))                                              \
        orrcx_ld(W(XD), W(MT), W(DT))

/* xor (G = G ^ S), (D = S ^ T) if (#D != #T) */

#define xorcx_rr(XG, XS)                                                    \
        xorcx3rr(W(XG), W(XG), W(XS))

#define xorcx_ld(XG, MS, DS)                                                \
        xorcx3ld(W(XG), W(XG), W(MS), W(DS))

#define xorcx3rr(XD, XS, XT)                                                \
        V2X(REG(XS), 1, 0) EMITB(0x57)                                      \
        MRM(REG(XD), MOD(XT), REG(XT))

#define xorcx3ld(XD, XS, MT, DT)                                            \
        V2X(REG(XS), 1, 0) EMITB(0x57)                                      \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* not (G = ~G), (D = ~S) */

#define notcx_rx(XG)                                                        \
        notcx_rr(W(XG), W(XG))

#define notcx_rr(XD, XS)                                                    \
        anncx3ld(W(XD), W(XS), Mebp, inf_GPC07)

/************   packed single-precision floating-point arithmetic   ***********/

/* neg (G = -G), (D = -S) */

#define negcs_rx(XG)                                                        \
        negcs_rr(W(XG), W(XG))

#define negcs_rr(XD, XS)                                                    \
        xorcx3ld(W(XD), W(XS), Mebp, inf_GPC06_32)

/* add (G = G + S), (D = S + T) if (#D != #T) */

#define addcs_rr(XG, XS)                                                    \
        addcs3rr(W(XG), W(XG), W(XS))

#define addcs_ld(XG, MS, DS)                                                \
        addcs3ld(W(XG), W(XG), W(MS), W(DS))

#define addcs3rr(XD, XS, XT)                                                \
        V2X(REG(XS), 1, 0) EMITB(0x58)                                      \
        MRM(REG(XD), MOD(XT), REG(XT))

#define addcs3ld(XD, XS, MT, DT)                                            \
        V2X(REG(XS), 1, 0) EMITB(0x58)                                      \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

        /* adp, adh are defined in rtbase.h (first 15-regs only)
         * under "COMMON SIMD INSTRUCTIONS" section */

#undef  adpcs_rx
#define adpcs_rx(XD) /* not portable, do not use outside */                 \
        movix_ld(W(XD), Mebp, inf_SCR01(0x00))                              \
        adpis_ld(W(XD), Mebp, inf_SCR01(0x10))                              \
        movix_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movix_ld(W(XD), Mebp, inf_SCR02(0x00))                              \
        adpis_ld(W(XD), Mebp, inf_SCR02(0x10))                              \
        movix_st(W(XD), Mebp, inf_SCR01(0x10))

/* sub (G = G - S), (D = S - T) if (#D != #T) */

#define subcs_rr(XG, XS)                                                    \
        subcs3rr(W(XG), W(XG), W(XS))

#define subcs_ld(XG, MS, DS)                                                \
        subcs3ld(W(XG), W(XG), W(MS), W(DS))

#define subcs3rr(XD, XS, XT)                                                \
        V2X(REG(XS), 1, 0) EMITB(0x5C)                                      \
        MRM(REG(XD), MOD(XT), REG(XT))

#define subcs3ld(XD, XS, MT, DT)                                            \
        V2X(REG(XS), 1, 0) EMITB(0x5C)                                      \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* mul (G = G * S), (D = S * T) if (#D != #T) */

#define mulcs_rr(XG, XS)                                                    \
        mulcs3rr(W(XG), W(XG), W(XS))

#define mulcs_ld(XG, MS, DS)                                                \
        mulcs3ld(W(XG), W(XG), W(MS), W(DS))

#define mulcs3rr(XD, XS, XT)                                                \
        V2X(REG(XS), 1, 0) EMITB(0x59)                                      \
        MRM(REG(XD), MOD(XT), REG(XT))

#define mulcs3ld(XD, XS, MT, DT)                                            \
        V2X(REG(XS), 1, 0) EMITB(0x59)                                      \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

        /* mlp, mlh are defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* div (G = G / S), (D = S / T) if (#D != #T) and on ARMv7 if (#D != #S) */

#define divcs_rr(XG, XS)                                                    \
        divcs3rr(W(XG), W(XG), W(XS))

#define divcs_ld(XG, MS, DS)                                                \
        divcs3ld(W(XG), W(XG), W(MS), W(DS))

#define divcs3rr(XD, XS, XT)                                                \
        V2X(REG(XS), 1, 0) EMITB(0x5E)                                      \
        MRM(REG(XD), MOD(XT), REG(XT))

#define divcs3ld(XD, XS, MT, DT)                                            \
        V2X(REG(XS), 1, 0) EMITB(0x5E)                                      \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* sqr (D = sqrt S) */

#define sqrcs_rr(XD, XS)                                                    \
        V2X(0x00,    1, 0) EMITB(0x51)                                      \
        MRM(REG(XD), MOD(XS), REG(XS))

#define sqrcs_ld(XD, MS, DS)                                                \
        V2X(0x00,    1, 0) EMITB(0x51)                                      \
        MRM(REG(XD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

/* cbr (D = cbrt S) */

        /* cbe, cbs, cbr are defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* rcp (D = 1.0 / S)
 * accuracy/behavior may vary across supported targets, use accordingly */

#if RT_SIMD_COMPAT_RCP != 1

#define rcecs_rr(XD, XS)                                                    \
        V2X(0x00,    1, 0) EMITB(0x53)                                      \
        MRM(REG(XD), MOD(XS), REG(XS))

#define rcscs_rr(XG, XS) /* destroys XS */                                  \
        mulcs_rr(W(XS), W(XG))                                              \
        mulcs_rr(W(XS), W(XG))                                              \
        addcs_rr(W(XG), W(XG))                                              \
        subcs_rr(W(XG), W(XS))

#endif /* RT_SIMD_COMPAT_RCP */

        /* rce, rcs, rcp are defined in rtconf.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* rsq (D = 1.0 / sqrt S)
 * accuracy/behavior may vary across supported targets, use accordingly */

#if RT_SIMD_COMPAT_RSQ != 1

#define rsecs_rr(XD, XS)                                                    \
        V2X(0x00,    1, 0) EMITB(0x52)                                      \
        MRM(REG(XD), MOD(XS), REG(XS))

#define rsscs_rr(XG, XS) /* destroys XS */                                  \
        mulcs_rr(W(XS), W(XG))                                              \
        mulcs_rr(W(XS), W(XG))                                              \
        subcs_ld(W(XS), Mebp, inf_GPC03_32)                                 \
        mulcs_ld(W(XS), Mebp, inf_GPC02_32)                                 \
        mulcs_rr(W(XG), W(XS))

#endif /* RT_SIMD_COMPAT_RSQ */

        /* rse, rss, rsq are defined in rtconf.h
         * under "COMMON SIMD INSTRUCTIONS" section */

#if (RT_256X1 < 2) && !(RT_SIMD == 128 && RT_128X1 == 16)

#define prmcx_rr(XD, XS, IT) /* not portable, do not use outside */         \
        VEX(REG(XD), 1, 1, 3) EMITB(0x06)                                   \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IT)))

#define cvycs_rr(XD, XS)     /* not portable, do not use outside */         \
        V2X(0x00,    1, 0) EMITB(0x5A)                                      \
        MRM(REG(XD), MOD(XS), REG(XS))

#define cvycs_ld(XD, MS, DS) /* not portable, do not use outside */         \
        V2X(0x00,    1, 0) EMITB(0x5A)                                      \
        MRM(REG(XD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMPTY)

#define cvxds_rr(XD, XS)     /* not portable, do not use outside */         \
        V2X(0x00,    1, 1) EMITB(0x5A)                                      \
        MRM(REG(XD), MOD(XS), REG(XS))

#define X(val, typ, cmd)  (val+16), typ, cmd

#define addds_ld(XG, MS, DS) /* not portable, do not use outside */         \
        V2X(REG(XG), 1, 1) EMITB(0x58)                                      \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define subds_ld(XG, MS, DS) /* not portable, do not use outside */         \
        V2X(REG(XG), 1, 1) EMITB(0x5C)                                      \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define mulds_ld(XG, MS, DS) /* not portable, do not use outside */         \
        V2X(REG(XG), 1, 1) EMITB(0x59)                                      \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#if RT_SIMD_COMPAT_FMA == 0

/* fma (G = G + S * T) if (#G != #S && #G != #T)
 * NOTE: x87 fpu-fallbacks for fma/fms use round-to-nearest mode by default,
 * enable RT_SIMD_COMPAT_FMR for current SIMD rounding mode to be honoured */

#define fmacs_rr(XG, XS, XT)                                                \
        movcx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        mulcs_rr(W(XS), W(XT))                                              \
        addcs_rr(W(XG), W(XS))                                              \
        movcx_ld(W(XS), Mebp, inf_SCR01(0))

#define fmacs_ld(XG, XS, MT, DT)                                            \
        movcx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        mulcs_ld(W(XS), W(MT), W(DT))                                       \
        addcs_rr(W(XG), W(XS))                                              \
        movcx_ld(W(XS), Mebp, inf_SCR01(0))

#elif RT_SIMD_COMPAT_FMA == 1

/* fma (G = G + S * T) if (#G != #S && #G != #T)
 * NOTE: x87 fpu-fallbacks for fma/fms use round-to-nearest mode by default,
 * enable RT_SIMD_COMPAT_FMR for current SIMD rounding mode to be honoured */

#define fmacs_rr(XG, XS, XT)                                                \
        movcx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movcx_st(W(XG), Mebp, inf_SCR02(0))                                 \
        cvycs_rr(W(XG), W(XS))                     /* 1st-pass -> */        \
        cvycs_rr(W(XS), W(XT))                                              \
        mulds_rr(W(XS), W(XG))                                              \
        cvycs_ld(W(XG), Mebp, inf_SCR02(0x00))                              \
        addds_rr(W(XG), W(XS))                                              \
        cvxds_rr(W(XG), W(XG))                                              \
        movix_st(W(XG), Mebp, inf_SCR02(0x00))                              \
        movcx_ld(W(XS), Mebp, inf_SCR01(0))                                 \
        prmcx_rr(W(XT), W(XT), IB(1))              /* 1st-pass <- */        \
        cvycs_ld(W(XG), Mebp, inf_SCR01(0x10))     /* 2nd-pass -> */        \
        cvycs_rr(W(XS), W(XT))                                              \
        mulds_rr(W(XS), W(XG))                                              \
        cvycs_ld(W(XG), Mebp, inf_SCR02(0x10))                              \
        addds_rr(W(XG), W(XS))                                              \
        cvxds_rr(W(XG), W(XG))                                              \
        movix_st(W(XG), Mebp, inf_SCR02(0x10))                              \
        prmcx_rr(W(XT), W(XT), IB(1))              /* 2nd-pass <- */        \
        movcx_ld(W(XG), Mebp, inf_SCR02(0))                                 \
        movcx_ld(W(XS), Mebp, inf_SCR01(0))

#define fmacs_ld(XG, XS, MT, DT)                                            \
        movcx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movcx_st(W(XG), Mebp, inf_SCR02(0))                                 \
        cvycs_rr(W(XG), W(XS))                     /* 1st-pass -> */        \
        cvycs_ld(W(XS), W(MT), W(DT))                                       \
        mulds_rr(W(XS), W(XG))                                              \
        cvycs_ld(W(XG), Mebp, inf_SCR02(0x00))                              \
        addds_rr(W(XG), W(XS))                                              \
        cvxds_rr(W(XG), W(XG))                                              \
        movix_st(W(XG), Mebp, inf_SCR02(0x00))     /* 1st-pass <- */        \
        cvycs_ld(W(XG), Mebp, inf_SCR01(0x10))     /* 2nd-pass -> */        \
        cvycs_ld(W(XS), W(MT), X(DT))                                       \
        mulds_rr(W(XS), W(XG))                                              \
        cvycs_ld(W(XG), Mebp, inf_SCR02(0x10))                              \
        addds_rr(W(XG), W(XS))                                              \
        cvxds_rr(W(XG), W(XG))                                              \
        movix_st(W(XG), Mebp, inf_SCR02(0x10))     /* 2nd-pass <- */        \
        movcx_ld(W(XG), Mebp, inf_SCR02(0))                                 \
        movcx_ld(W(XS), Mebp, inf_SCR01(0))

#endif /* RT_SIMD_COMPAT_FMA */

#if RT_SIMD_COMPAT_FMS == 0

/* fms (G = G - S * T) if (#G != #S && #G != #T)
 * NOTE: due to final negation being outside of rounding on all POWER systems
 * only symmetric rounding modes (RN, RZ) are compatible across all targets */

#define fmscs_rr(XG, XS, XT)                                                \
        movcx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        mulcs_rr(W(XS), W(XT))                                              \
        subcs_rr(W(XG), W(XS))                                              \
        movcx_ld(W(XS), Mebp, inf_SCR01(0))

#define fmscs_ld(XG, XS, MT, DT)                                            \
        movcx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        mulcs_ld(W(XS), W(MT), W(DT))                                       \
        subcs_rr(W(XG), W(XS))                                              \
        movcx_ld(W(XS), Mebp, inf_SCR01(0))

#elif RT_SIMD_COMPAT_FMS == 1

/* fms (G = G - S * T) if (#G != #S && #G != #T)
 * NOTE: due to final negation being outside of rounding on all POWER systems
 * only symmetric rounding modes (RN, RZ) are compatible across all targets */

#define fmscs_rr(XG, XS, XT)                                                \
        movcx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movcx_st(W(XG), Mebp, inf_SCR02(0))                                 \
        cvycs_rr(W(XG), W(XS))                     /* 1st-pass -> */        \
        cvycs_rr(W(XS), W(XT))                                              \
        mulds_rr(W(XS), W(XG))                                              \
        cvycs_ld(W(XG), Mebp, inf_SCR02(0x00))                              \
        subds_rr(W(XG), W(XS))                                              \
        cvxds_rr(W(XG), W(XG))                                              \
        movix_st(W(XG), Mebp, inf_SCR02(0x00))                              \
        movcx_ld(W(XS), Mebp, inf_SCR01(0))                                 \
        prmcx_rr(W(XT), W(XT), IB(1))              /* 1st-pass <- */        \
        cvycs_ld(W(XG), Mebp, inf_SCR01(0x10))     /* 2nd-pass -> */        \
        cvycs_rr(W(XS), W(XT))                                              \
        mulds_rr(W(XS), W(XG))                                              \
        cvycs_ld(W(XG), Mebp, inf_SCR02(0x10))                              \
        subds_rr(W(XG), W(XS))                                              \
        cvxds_rr(W(XG), W(XG))                                              \
        movix_st(W(XG), Mebp, inf_SCR02(0x10))                              \
        prmcx_rr(W(XT), W(XT), IB(1))              /* 2nd-pass <- */        \
        movcx_ld(W(XG), Mebp, inf_SCR02(0))                                 \
        movcx_ld(W(XS), Mebp, inf_SCR01(0))

#define fmscs_ld(XG, XS, MT, DT)                                            \
        movcx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movcx_st(W(XG), Mebp, inf_SCR02(0))                                 \
        cvycs_rr(W(XG), W(XS))                     /* 1st-pass -> */        \
        cvycs_ld(W(XS), W(MT), W(DT))                                       \
        mulds_rr(W(XS), W(XG))                                              \
        cvycs_ld(W(XG), Mebp, inf_SCR02(0x00))                              \
        subds_rr(W(XG), W(XS))                                              \
        cvxds_rr(W(XG), W(XG))                                              \
        movix_st(W(XG), Mebp, inf_SCR02(0x00))     /* 1st-pass <- */        \
        cvycs_ld(W(XG), Mebp, inf_SCR01(0x10))     /* 2nd-pass -> */        \
        cvycs_ld(W(XS), W(MT), X(DT))                                       \
        mulds_rr(W(XS), W(XG))                                              \
        cvycs_ld(W(XG), Mebp, inf_SCR02(0x10))                              \
        subds_rr(W(XG), W(XS))                                              \
        cvxds_rr(W(XG), W(XG))                                              \
        movix_st(W(XG), Mebp, inf_SCR02(0x10))     /* 2nd-pass <- */        \
        movcx_ld(W(XG), Mebp, inf_SCR02(0))                                 \
        movcx_ld(W(XS), Mebp, inf_SCR01(0))

#endif /* RT_SIMD_COMPAT_FMS */

#else /* RT_256X1 >= 2 || RT_SIMD == 128 && RT_128X1 == 16, AVX2 or FMA3 */

/* fma (G = G + S * T) if (#G != #S && #G != #T)
 * NOTE: x87 fpu-fallbacks for fma/fms use round-to-nearest mode by default,
 * enable RT_SIMD_COMPAT_FMR for current SIMD rounding mode to be honoured */

#if RT_SIMD_COMPAT_FMA <= 1

#define fmacs_rr(XG, XS, XT)                                                \
        VEX(REG(XS), 1, 1, 2) EMITB(0xB8)                                   \
        MRM(REG(XG), MOD(XT), REG(XT))

#define fmacs_ld(XG, XS, MT, DT)                                            \
        VEX(REG(XS), 1, 1, 2) EMITB(0xB8)                                   \
        MRM(REG(XG), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

#endif /* RT_SIMD_COMPAT_FMA */

/* fms (G = G - S * T) if (#G != #S && #G != #T)
 * NOTE: due to final negation being outside of rounding on all POWER systems
 * only symmetric rounding modes (RN, RZ) are compatible across all targets */

#if RT_SIMD_COMPAT_FMS <= 1

#define fmscs_rr(XG, XS, XT)                                                \
        VEX(REG(XS), 1, 1, 2) EMITB(0xBC)                                   \
        MRM(REG(XG), MOD(XT), REG(XT))

#define fmscs_ld(XG, XS, MT, DT)                                            \
        VEX(REG(XS), 1, 1, 2) EMITB(0xBC)                                   \
        MRM(REG(XG), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

#endif /* RT_SIMD_COMPAT_FMS */

#endif /* RT_256X1 >= 2 || RT_SIMD == 128 && RT_128X1 == 16, AVX2 or FMA3 */

/*************   packed single-precision floating-point compare   *************/

/* min (G = G < S ? G : S), (D = S < T ? S : T) if (#D != #T) */

#define mincs_rr(XG, XS)                                                    \
        mincs3rr(W(XG), W(XG), W(XS))

#define mincs_ld(XG, MS, DS)                                                \
        mincs3ld(W(XG), W(XG), W(MS), W(DS))

#define mincs3rr(XD, XS, XT)                                                \
        V2X(REG(XS), 1, 0) EMITB(0x5D)                                      \
        MRM(REG(XD), MOD(XT), REG(XT))

#define mincs3ld(XD, XS, MT, DT)                                            \
        V2X(REG(XS), 1, 0) EMITB(0x5D)                                      \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

        /* mnp, mnh are defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* max (G = G > S ? G : S), (D = S > T ? S : T) if (#D != #T) */

#define maxcs_rr(XG, XS)                                                    \
        maxcs3rr(W(XG), W(XG), W(XS))

#define maxcs_ld(XG, MS, DS)                                                \
        maxcs3ld(W(XG), W(XG), W(MS), W(DS))

#define maxcs3rr(XD, XS, XT)                                                \
        V2X(REG(XS), 1, 0) EMITB(0x5F)                                      \
        MRM(REG(XD), MOD(XT), REG(XT))

#define maxcs3ld(XD, XS, MT, DT)                                            \
        V2X(REG(XS), 1, 0) EMITB(0x5F)                                      \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

        /* mxp, mxh are defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* ceq (G = G == S ? -1 : 0), (D = S == T ? -1 : 0) if (#D != #T) */

#define ceqcs_rr(XG, XS)                                                    \
        ceqcs3rr(W(XG), W(XG), W(XS))

#define ceqcs_ld(XG, MS, DS)                                                \
        ceqcs3ld(W(XG), W(XG), W(MS), W(DS))

#define ceqcs3rr(XD, XS, XT)                                                \
        V2X(REG(XS), 1, 0) EMITB(0xC2)                                      \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x00))

#define ceqcs3ld(XD, XS, MT, DT)                                            \
        V2X(REG(XS), 1, 0) EMITB(0xC2)                                      \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMITB(0x00))

/* cne (G = G != S ? -1 : 0), (D = S != T ? -1 : 0) if (#D != #T) */

#define cnecs_rr(XG, XS)                                                    \
        cnecs3rr(W(XG), W(XG), W(XS))

#define cnecs_ld(XG, MS, DS)                                                \
        cnecs3ld(W(XG), W(XG), W(MS), W(DS))

#define cnecs3rr(XD, XS, XT)                                                \
        V2X(REG(XS), 1, 0) EMITB(0xC2)                                      \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x04))

#define cnecs3ld(XD, XS, MT, DT)                                            \
        V2X(REG(XS), 1, 0) EMITB(0xC2)                                      \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMITB(0x04))

/* clt (G = G < S ? -1 : 0), (D = S < T ? -1 : 0) if (#D != #T) */

#define cltcs_rr(XG, XS)                                                    \
        cltcs3rr(W(XG), W(XG), W(XS))

#define cltcs_ld(XG, MS, DS)                                                \
        cltcs3ld(W(XG), W(XG), W(MS), W(DS))

#define cltcs3rr(XD, XS, XT)                                                \
        V2X(REG(XS), 1, 0) EMITB(0xC2)                                      \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x01))

#define cltcs3ld(XD, XS, MT, DT)                                            \
        V2X(REG(XS), 1, 0) EMITB(0xC2)                                      \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMITB(0x01))

/* cle (G = G <= S ? -1 : 0), (D = S <= T ? -1 : 0) if (#D != #T) */

#define clecs_rr(XG, XS)                                                    \
        clecs3rr(W(XG), W(XG), W(XS))

#define clecs_ld(XG, MS, DS)                                                \
        clecs3ld(W(XG), W(XG), W(MS), W(DS))

#define clecs3rr(XD, XS, XT)                                                \
        V2X(REG(XS), 1, 0) EMITB(0xC2)                                      \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x02))

#define clecs3ld(XD, XS, MT, DT)                                            \
        V2X(REG(XS), 1, 0) EMITB(0xC2)                                      \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMITB(0x02))

/* cgt (G = G > S ? -1 : 0), (D = S > T ? -1 : 0) if (#D != #T) */

#define cgtcs_rr(XG, XS)                                                    \
        cgtcs3rr(W(XG), W(XG), W(XS))

#define cgtcs_ld(XG, MS, DS)                                                \
        cgtcs3ld(W(XG), W(XG), W(MS), W(DS))

#define cgtcs3rr(XD, XS, XT)                                                \
        V2X(REG(XS), 1, 0) EMITB(0xC2)                                      \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x06))

#define cgtcs3ld(XD, XS, MT, DT)                                            \
        V2X(REG(XS), 1, 0) EMITB(0xC2)                                      \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMITB(0x06))

/* cge (G = G >= S ? -1 : 0), (D = S >= T ? -1 : 0) if (#D != #T) */

#define cgecs_rr(XG, XS)                                                    \
        cgecs3rr(W(XG), W(XG), W(XS))

#define cgecs_ld(XG, MS, DS)                                                \
        cgecs3ld(W(XG), W(XG), W(MS), W(DS))

#define cgecs3rr(XD, XS, XT)                                                \
        V2X(REG(XS), 1, 0) EMITB(0xC2)                                      \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x05))

#define cgecs3ld(XD, XS, MT, DT)                                            \
        V2X(REG(XS), 1, 0) EMITB(0xC2)                                      \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMITB(0x05))

/* mkj (jump to lb) if (S satisfies mask condition) */

#define RT_SIMD_MASK_NONE32_256    0x00     /* none satisfy the condition */
#define RT_SIMD_MASK_FULL32_256    0xFF     /*  all satisfy the condition */

#define mkjcx_rx(XS, mask, lb)   /* destroys Reax, if S == mask jump lb */  \
        V2X(0x00,    1, 0) EMITB(0x50)                                      \
        MRM(0x00,    MOD(XS), REG(XS))                                      \
        cmpwx_ri(Reax, IH(RT_SIMD_MASK_##mask##32_256))                     \
        jeqxx_lb(lb)

/*************   packed single-precision floating-point convert   *************/

/* cvz (D = fp-to-signed-int S)
 * rounding mode is encoded directly (can be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp32 SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnzcs_rr(XD, XS)     /* round towards zero */                       \
        VEX(0x00,    1, 1, 3) EMITB(0x08)                                   \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x03))

#define rnzcs_ld(XD, MS, DS) /* round towards zero */                       \
        VEX(0x00,    1, 1, 3) EMITB(0x08)                                   \
        MRM(REG(XD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMITB(0x03))

#define cvzcs_rr(XD, XS)     /* round towards zero */                       \
        V2X(0x00,    1, 2) EMITB(0x5B)                                      \
        MRM(REG(XD), MOD(XS), REG(XS))

#define cvzcs_ld(XD, MS, DS) /* round towards zero */                       \
        V2X(0x00,    1, 2) EMITB(0x5B)                                      \
        MRM(REG(XD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

/* cvp (D = fp-to-signed-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp32 SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnpcs_rr(XD, XS)     /* round towards +inf */                       \
        VEX(0x00,    1, 1, 3) EMITB(0x08)                                   \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x02))

#define rnpcs_ld(XD, MS, DS) /* round towards +inf */                       \
        VEX(0x00,    1, 1, 3) EMITB(0x08)                                   \
        MRM(REG(XD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMITB(0x02))

#define cvpcs_rr(XD, XS)     /* round towards +inf */                       \
        rnpcs_rr(W(XD), W(XS))                                              \
        cvzcs_rr(W(XD), W(XD))

#define cvpcs_ld(XD, MS, DS) /* round towards +inf */                       \
        rnpcs_ld(W(XD), W(MS), W(DS))                                       \
        cvzcs_rr(W(XD), W(XD))

/* cvm (D = fp-to-signed-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp32 SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnmcs_rr(XD, XS)     /* round towards -inf */                       \
        VEX(0x00,    1, 1, 3) EMITB(0x08)                                   \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x01))

#define rnmcs_ld(XD, MS, DS) /* round towards -inf */                       \
        VEX(0x00,    1, 1, 3) EMITB(0x08)                                   \
        MRM(REG(XD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMITB(0x01))

#define cvmcs_rr(XD, XS)     /* round towards -inf */                       \
        rnmcs_rr(W(XD), W(XS))                                              \
        cvzcs_rr(W(XD), W(XD))

#define cvmcs_ld(XD, MS, DS) /* round towards -inf */                       \
        rnmcs_ld(W(XD), W(MS), W(DS))                                       \
        cvzcs_rr(W(XD), W(XD))

/* cvn (D = fp-to-signed-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp32 SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnncs_rr(XD, XS)     /* round towards near */                       \
        VEX(0x00,    1, 1, 3) EMITB(0x08)                                   \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x00))

#define rnncs_ld(XD, MS, DS) /* round towards near */                       \
        VEX(0x00,    1, 1, 3) EMITB(0x08)                                   \
        MRM(REG(XD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMITB(0x00))

#define cvncs_rr(XD, XS)     /* round towards near */                       \
        cvtcs_rr(W(XD), W(XS))

#define cvncs_ld(XD, MS, DS) /* round towards near */                       \
        cvtcs_ld(W(XD), W(MS), W(DS))

/* cvn (D = signed-int-to-fp S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks) */

#define cvncn_rr(XD, XS)     /* round towards near */                       \
        cvtcn_rr(W(XD), W(XS))

#define cvncn_ld(XD, MS, DS) /* round towards near */                       \
        cvtcn_ld(W(XD), W(MS), W(DS))

/* cvt (D = fp-to-signed-int S)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: ROUNDZ is not supported on pre-VSX POWER systems, use cvz
 * NOTE: due to compatibility with legacy targets, fp32 SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rndcs_rr(XD, XS)                                                    \
        VEX(0x00,    1, 1, 3) EMITB(0x08)                                   \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x04))

#define rndcs_ld(XD, MS, DS)                                                \
        VEX(0x00,    1, 1, 3) EMITB(0x08)                                   \
        MRM(REG(XD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMITB(0x04))

#define cvtcs_rr(XD, XS)                                                    \
        V2X(0x00,    1, 1) EMITB(0x5B)                                      \
        MRM(REG(XD), MOD(XS), REG(XS))

#define cvtcs_ld(XD, MS, DS)                                                \
        V2X(0x00,    1, 1) EMITB(0x5B)                                      \
        MRM(REG(XD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

/* cvt (D = signed-int-to-fp S)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: only default ROUNDN is supported on pre-VSX POWER systems */

#define cvtcn_rr(XD, XS)                                                    \
        V2X(0x00,    1, 0) EMITB(0x5B)                                      \
        MRM(REG(XD), MOD(XS), REG(XS))

#define cvtcn_ld(XD, MS, DS)                                                \
        V2X(0x00,    1, 0) EMITB(0x5B)                                      \
        MRM(REG(XD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

/* cvr (D = fp-to-signed-int S)
 * rounding mode is encoded directly (cannot be used in FCTRL blocks)
 * NOTE: on targets with full-IEEE SIMD fp-arithmetic the ROUND*_F mode
 * isn't always taken into account when used within full-IEEE ASM block
 * NOTE: due to compatibility with legacy targets, fp32 SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnrcs_rr(XD, XS, mode)                                              \
        VEX(0x00,    1, 1, 3) EMITB(0x08)                                   \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(RT_SIMD_MODE_##mode&3))

#define cvrcs_rr(XD, XS, mode)                                              \
        rnrcs_rr(W(XD), W(XS), mode)                                        \
        cvzcs_rr(W(XD), W(XD))

/************   packed single-precision integer arithmetic/shifts   ***********/

#if (RT_256X1 < 2)

/* add (G = G + S), (D = S + T) if (#D != #T) */

#define addcx_rr(XG, XS)                                                    \
        addcx3rr(W(XG), W(XG), W(XS))

#define addcx_ld(XG, MS, DS)                                                \
        addcx3ld(W(XG), W(XG), W(MS), W(DS))

#define addcx3rr(XD, XS, XT)                                                \
        movcx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movcx_st(W(XT), Mebp, inf_SCR02(0))                                 \
        movix_ld(W(XD), Mebp, inf_SCR01(0x00))                              \
        addix_ld(W(XD), Mebp, inf_SCR02(0x00))                              \
        movix_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movix_ld(W(XD), Mebp, inf_SCR01(0x10))                              \
        addix_ld(W(XD), Mebp, inf_SCR02(0x10))                              \
        movix_st(W(XD), Mebp, inf_SCR01(0x10))                              \
        movcx_ld(W(XD), Mebp, inf_SCR01(0))

#define addcx3ld(XD, XS, MT, DT)                                            \
        movcx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movcx_ld(W(XD), W(MT), W(DT))                                       \
        movcx_st(W(XD), Mebp, inf_SCR02(0))                                 \
        movix_ld(W(XD), Mebp, inf_SCR01(0x00))                              \
        addix_ld(W(XD), Mebp, inf_SCR02(0x00))                              \
        movix_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movix_ld(W(XD), Mebp, inf_SCR01(0x10))                              \
        addix_ld(W(XD), Mebp, inf_SCR02(0x10))                              \
        movix_st(W(XD), Mebp, inf_SCR01(0x10))                              \
        movcx_ld(W(XD), Mebp, inf_SCR01(0))

/* sub (G = G - S), (D = S - T) if (#D != #T) */

#define subcx_rr(XG, XS)                                                    \
        subcx3rr(W(XG), W(XG), W(XS))

#define subcx_ld(XG, MS, DS)                                                \
        subcx3ld(W(XG), W(XG), W(MS), W(DS))

#define subcx3rr(XD, XS, XT)                                                \
        movcx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movcx_st(W(XT), Mebp, inf_SCR02(0))                                 \
        movix_ld(W(XD), Mebp, inf_SCR01(0x00))                              \
        subix_ld(W(XD), Mebp, inf_SCR02(0x00))                              \
        movix_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movix_ld(W(XD), Mebp, inf_SCR01(0x10))                              \
        subix_ld(W(XD), Mebp, inf_SCR02(0x10))                              \
        movix_st(W(XD), Mebp, inf_SCR01(0x10))                              \
        movcx_ld(W(XD), Mebp, inf_SCR01(0))

#define subcx3ld(XD, XS, MT, DT)                                            \
        movcx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movcx_ld(W(XD), W(MT), W(DT))                                       \
        movcx_st(W(XD), Mebp, inf_SCR02(0))                                 \
        movix_ld(W(XD), Mebp, inf_SCR01(0x00))                              \
        subix_ld(W(XD), Mebp, inf_SCR02(0x00))                              \
        movix_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movix_ld(W(XD), Mebp, inf_SCR01(0x10))                              \
        subix_ld(W(XD), Mebp, inf_SCR02(0x10))                              \
        movix_st(W(XD), Mebp, inf_SCR01(0x10))                              \
        movcx_ld(W(XD), Mebp, inf_SCR01(0))

/* mul (G = G * S), (D = S * T) if (#D != #T) */

#define mulcx_rr(XG, XS)                                                    \
        mulcx3rr(W(XG), W(XG), W(XS))

#define mulcx_ld(XG, MS, DS)                                                \
        mulcx3ld(W(XG), W(XG), W(MS), W(DS))

#define mulcx3rr(XD, XS, XT)                                                \
        movcx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movcx_st(W(XT), Mebp, inf_SCR02(0))                                 \
        movix_ld(W(XD), Mebp, inf_SCR01(0x00))                              \
        mulix_ld(W(XD), Mebp, inf_SCR02(0x00))                              \
        movix_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movix_ld(W(XD), Mebp, inf_SCR01(0x10))                              \
        mulix_ld(W(XD), Mebp, inf_SCR02(0x10))                              \
        movix_st(W(XD), Mebp, inf_SCR01(0x10))                              \
        movcx_ld(W(XD), Mebp, inf_SCR01(0))

#define mulcx3ld(XD, XS, MT, DT)                                            \
        movcx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movcx_ld(W(XD), W(MT), W(DT))                                       \
        movcx_st(W(XD), Mebp, inf_SCR02(0))                                 \
        movix_ld(W(XD), Mebp, inf_SCR01(0x00))                              \
        mulix_ld(W(XD), Mebp, inf_SCR02(0x00))                              \
        movix_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movix_ld(W(XD), Mebp, inf_SCR01(0x10))                              \
        mulix_ld(W(XD), Mebp, inf_SCR02(0x10))                              \
        movix_st(W(XD), Mebp, inf_SCR01(0x10))                              \
        movcx_ld(W(XD), Mebp, inf_SCR01(0))

/* shl (G = G << S), (D = S << T) if (#D != #T) - plain, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define shlcx_ri(XG, IS)                                                    \
        shlcx3ri(W(XG), W(XG), W(IS))

#define shlcx_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shlcx3ld(W(XG), W(XG), W(MS), W(DS))

#define shlcx3ri(XD, XS, IT)                                                \
        movcx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        shlix3ri(W(XD), W(XS), W(IT))                                       \
        movix_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movix_ld(W(XD), Mebp, inf_SCR01(0x10))                              \
        shlix_ri(W(XD), W(IT))                                              \
        movix_st(W(XD), Mebp, inf_SCR01(0x10))                              \
        movcx_ld(W(XD), Mebp, inf_SCR01(0))

#define shlcx3ld(XD, XS, MT, DT)                                            \
        movcx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        shlix3ld(W(XD), W(XS), W(MT), W(DT))                                \
        movix_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movix_ld(W(XD), Mebp, inf_SCR01(0x10))                              \
        shlix_ld(W(XD), W(MT), W(DT))                                       \
        movix_st(W(XD), Mebp, inf_SCR01(0x10))                              \
        movcx_ld(W(XD), Mebp, inf_SCR01(0))

/* shr (G = G >> S), (D = S >> T) if (#D != #T) - plain, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define shrcx_ri(XG, IS)                                                    \
        shrcx3ri(W(XG), W(XG), W(IS))

#define shrcx_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shrcx3ld(W(XG), W(XG), W(MS), W(DS))

#define shrcx3ri(XD, XS, IT)                                                \
        movcx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        shrix3ri(W(XD), W(XS), W(IT))                                       \
        movix_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movix_ld(W(XD), Mebp, inf_SCR01(0x10))                              \
        shrix_ri(W(XD), W(IT))                                              \
        movix_st(W(XD), Mebp, inf_SCR01(0x10))                              \
        movcx_ld(W(XD), Mebp, inf_SCR01(0))

#define shrcx3ld(XD, XS, MT, DT)                                            \
        movcx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        shrix3ld(W(XD), W(XS), W(MT), W(DT))                                \
        movix_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movix_ld(W(XD), Mebp, inf_SCR01(0x10))                              \
        shrix_ld(W(XD), W(MT), W(DT))                                       \
        movix_st(W(XD), Mebp, inf_SCR01(0x10))                              \
        movcx_ld(W(XD), Mebp, inf_SCR01(0))

/* shr (G = G >> S), (D = S >> T) if (#D != #T) - plain, signed
 * for maximum compatibility: shift count must be modulo elem-size */

#define shrcn_ri(XG, IS)                                                    \
        shrcn3ri(W(XG), W(XG), W(IS))

#define shrcn_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shrcn3ld(W(XG), W(XG), W(MS), W(DS))

#define shrcn3ri(XD, XS, IT)                                                \
        movcx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        shrin3ri(W(XD), W(XS), W(IT))                                       \
        movix_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movix_ld(W(XD), Mebp, inf_SCR01(0x10))                              \
        shrin_ri(W(XD), W(IT))                                              \
        movix_st(W(XD), Mebp, inf_SCR01(0x10))                              \
        movcx_ld(W(XD), Mebp, inf_SCR01(0))

#define shrcn3ld(XD, XS, MT, DT)                                            \
        movcx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        shrin3ld(W(XD), W(XS), W(MT), W(DT))                                \
        movix_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movix_ld(W(XD), Mebp, inf_SCR01(0x10))                              \
        shrin_ld(W(XD), W(MT), W(DT))                                       \
        movix_st(W(XD), Mebp, inf_SCR01(0x10))                              \
        movcx_ld(W(XD), Mebp, inf_SCR01(0))

/* svl (G = G << S), (D = S << T) if (#D != #T) - variable, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define svlcx_rr(XG, XS)     /* variable shift with per-elem count */       \
        svlcx3rr(W(XG), W(XG), W(XS))

#define svlcx_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svlcx3ld(W(XG), W(XG), W(MS), W(DS))

#define svlcx3rr(XD, XS, XT)                                                \
        movcx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movcx_st(W(XT), Mebp, inf_SCR02(0))                                 \
        stack_st(Recx)                                                      \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x00))                              \
        shlwx_mx(Mebp,  inf_SCR01(0x00))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x04))                              \
        shlwx_mx(Mebp,  inf_SCR01(0x04))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x08))                              \
        shlwx_mx(Mebp,  inf_SCR01(0x08))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x0C))                              \
        shlwx_mx(Mebp,  inf_SCR01(0x0C))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x10))                              \
        shlwx_mx(Mebp,  inf_SCR01(0x10))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x14))                              \
        shlwx_mx(Mebp,  inf_SCR01(0x14))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x18))                              \
        shlwx_mx(Mebp,  inf_SCR01(0x18))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x1C))                              \
        shlwx_mx(Mebp,  inf_SCR01(0x1C))                                    \
        stack_ld(Recx)                                                      \
        movcx_ld(W(XD), Mebp, inf_SCR01(0))

#define svlcx3ld(XD, XS, MT, DT)                                            \
        movcx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movcx_ld(W(XD), W(MT), W(DT))                                       \
        movcx_st(W(XD), Mebp, inf_SCR02(0))                                 \
        stack_st(Recx)                                                      \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x00))                              \
        shlwx_mx(Mebp,  inf_SCR01(0x00))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x04))                              \
        shlwx_mx(Mebp,  inf_SCR01(0x04))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x08))                              \
        shlwx_mx(Mebp,  inf_SCR01(0x08))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x0C))                              \
        shlwx_mx(Mebp,  inf_SCR01(0x0C))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x10))                              \
        shlwx_mx(Mebp,  inf_SCR01(0x10))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x14))                              \
        shlwx_mx(Mebp,  inf_SCR01(0x14))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x18))                              \
        shlwx_mx(Mebp,  inf_SCR01(0x18))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x1C))                              \
        shlwx_mx(Mebp,  inf_SCR01(0x1C))                                    \
        stack_ld(Recx)                                                      \
        movcx_ld(W(XD), Mebp, inf_SCR01(0))

/* svr (G = G >> S), (D = S >> T) if (#D != #T) - variable, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define svrcx_rr(XG, XS)     /* variable shift with per-elem count */       \
        svrcx3rr(W(XG), W(XG), W(XS))

#define svrcx_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svrcx3ld(W(XG), W(XG), W(MS), W(DS))

#define svrcx3rr(XD, XS, XT)                                                \
        movcx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movcx_st(W(XT), Mebp, inf_SCR02(0))                                 \
        stack_st(Recx)                                                      \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x00))                              \
        shrwx_mx(Mebp,  inf_SCR01(0x00))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x04))                              \
        shrwx_mx(Mebp,  inf_SCR01(0x04))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x08))                              \
        shrwx_mx(Mebp,  inf_SCR01(0x08))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x0C))                              \
        shrwx_mx(Mebp,  inf_SCR01(0x0C))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x10))                              \
        shrwx_mx(Mebp,  inf_SCR01(0x10))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x14))                              \
        shrwx_mx(Mebp,  inf_SCR01(0x14))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x18))                              \
        shrwx_mx(Mebp,  inf_SCR01(0x18))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x1C))                              \
        shrwx_mx(Mebp,  inf_SCR01(0x1C))                                    \
        stack_ld(Recx)                                                      \
        movcx_ld(W(XD), Mebp, inf_SCR01(0))

#define svrcx3ld(XD, XS, MT, DT)                                            \
        movcx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movcx_ld(W(XD), W(MT), W(DT))                                       \
        movcx_st(W(XD), Mebp, inf_SCR02(0))                                 \
        stack_st(Recx)                                                      \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x00))                              \
        shrwx_mx(Mebp,  inf_SCR01(0x00))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x04))                              \
        shrwx_mx(Mebp,  inf_SCR01(0x04))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x08))                              \
        shrwx_mx(Mebp,  inf_SCR01(0x08))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x0C))                              \
        shrwx_mx(Mebp,  inf_SCR01(0x0C))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x10))                              \
        shrwx_mx(Mebp,  inf_SCR01(0x10))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x14))                              \
        shrwx_mx(Mebp,  inf_SCR01(0x14))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x18))                              \
        shrwx_mx(Mebp,  inf_SCR01(0x18))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x1C))                              \
        shrwx_mx(Mebp,  inf_SCR01(0x1C))                                    \
        stack_ld(Recx)                                                      \
        movcx_ld(W(XD), Mebp, inf_SCR01(0))

/* svr (G = G >> S), (D = S >> T) if (#D != #T) - variable, signed
 * for maximum compatibility: shift count must be modulo elem-size */

#define svrcn_rr(XG, XS)     /* variable shift with per-elem count */       \
        svrcn3rr(W(XG), W(XG), W(XS))

#define svrcn_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svrcn3ld(W(XG), W(XG), W(MS), W(DS))

#define svrcn3rr(XD, XS, XT)                                                \
        movcx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movcx_st(W(XT), Mebp, inf_SCR02(0))                                 \
        stack_st(Recx)                                                      \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x00))                              \
        shrwn_mx(Mebp,  inf_SCR01(0x00))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x04))                              \
        shrwn_mx(Mebp,  inf_SCR01(0x04))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x08))                              \
        shrwn_mx(Mebp,  inf_SCR01(0x08))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x0C))                              \
        shrwn_mx(Mebp,  inf_SCR01(0x0C))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x10))                              \
        shrwn_mx(Mebp,  inf_SCR01(0x10))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x14))                              \
        shrwn_mx(Mebp,  inf_SCR01(0x14))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x18))                              \
        shrwn_mx(Mebp,  inf_SCR01(0x18))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x1C))                              \
        shrwn_mx(Mebp,  inf_SCR01(0x1C))                                    \
        stack_ld(Recx)                                                      \
        movcx_ld(W(XD), Mebp, inf_SCR01(0))

#define svrcn3ld(XD, XS, MT, DT)                                            \
        movcx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movcx_ld(W(XD), W(MT), W(DT))                                       \
        movcx_st(W(XD), Mebp, inf_SCR02(0))                                 \
        stack_st(Recx)                                                      \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x00))                              \
        shrwn_mx(Mebp,  inf_SCR01(0x00))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x04))                              \
        shrwn_mx(Mebp,  inf_SCR01(0x04))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x08))                              \
        shrwn_mx(Mebp,  inf_SCR01(0x08))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x0C))                              \
        shrwn_mx(Mebp,  inf_SCR01(0x0C))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x10))                              \
        shrwn_mx(Mebp,  inf_SCR01(0x10))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x14))                              \
        shrwn_mx(Mebp,  inf_SCR01(0x14))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x18))                              \
        shrwn_mx(Mebp,  inf_SCR01(0x18))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x1C))                              \
        shrwn_mx(Mebp,  inf_SCR01(0x1C))                                    \
        stack_ld(Recx)                                                      \
        movcx_ld(W(XD), Mebp, inf_SCR01(0))

#else /* RT_256X1 >= 2, AVX2 */

/* add (G = G + S), (D = S + T) if (#D != #T) */

#define addcx_rr(XG, XS)                                                    \
        addcx3rr(W(XG), W(XG), W(XS))

#define addcx_ld(XG, MS, DS)                                                \
        addcx3ld(W(XG), W(XG), W(MS), W(DS))

#define addcx3rr(XD, XS, XT)                                                \
        V2X(REG(XS), 1, 1) EMITB(0xFE)                                      \
        MRM(REG(XD), MOD(XT), REG(XT))

#define addcx3ld(XD, XS, MT, DT)                                            \
        V2X(REG(XS), 1, 1) EMITB(0xFE)                                      \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* sub (G = G - S), (D = S - T) if (#D != #T) */

#define subcx_rr(XG, XS)                                                    \
        subcx3rr(W(XG), W(XG), W(XS))

#define subcx_ld(XG, MS, DS)                                                \
        subcx3ld(W(XG), W(XG), W(MS), W(DS))

#define subcx3rr(XD, XS, XT)                                                \
        V2X(REG(XS), 1, 1) EMITB(0xFA)                                      \
        MRM(REG(XD), MOD(XT), REG(XT))

#define subcx3ld(XD, XS, MT, DT)                                            \
        V2X(REG(XS), 1, 1) EMITB(0xFA)                                      \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* mul (G = G * S), (D = S * T) if (#D != #T) */

#define mulcx_rr(XG, XS)                                                    \
        mulcx3rr(W(XG), W(XG), W(XS))

#define mulcx_ld(XG, MS, DS)                                                \
        mulcx3ld(W(XG), W(XG), W(MS), W(DS))

#define mulcx3rr(XD, XS, XT)                                                \
        VEX(REG(XS), 1, 1, 2) EMITB(0x40)                                   \
        MRM(REG(XD), MOD(XT), REG(XT))

#define mulcx3ld(XD, XS, MT, DT)                                            \
        VEX(REG(XS), 1, 1, 2) EMITB(0x40)                                   \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* shl (G = G << S), (D = S << T) if (#D != #T) - plain, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define shlcx_ri(XG, IS)                                                    \
        shlcx3ri(W(XG), W(XG), W(IS))

#define shlcx_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shlcx3ld(W(XG), W(XG), W(MS), W(DS))

#define shlcx3ri(XD, XS, IT)                                                \
        V2X(REG(XD), 1, 1) EMITB(0x72)                                      \
        MRM(0x06,    MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IT)))

#define shlcx3ld(XD, XS, MT, DT)                                            \
        V2X(REG(XS), 1, 1) EMITB(0xF2)                                      \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* shr (G = G >> S), (D = S >> T) if (#D != #T) - plain, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define shrcx_ri(XG, IS)                                                    \
        shrcx3ri(W(XG), W(XG), W(IS))

#define shrcx_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shrcx3ld(W(XG), W(XG), W(MS), W(DS))

#define shrcx3ri(XD, XS, IT)                                                \
        V2X(REG(XD), 1, 1) EMITB(0x72)                                      \
        MRM(0x02,    MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IT)))

#define shrcx3ld(XD, XS, MT, DT)                                            \
        V2X(REG(XS), 1, 1) EMITB(0xD2)                                      \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* shr (G = G >> S), (D = S >> T) if (#D != #T) - plain, signed
 * for maximum compatibility: shift count must be modulo elem-size */

#define shrcn_ri(XG, IS)                                                    \
        shrcn3ri(W(XG), W(XG), W(IS))

#define shrcn_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shrcn3ld(W(XG), W(XG), W(MS), W(DS))

#define shrcn3ri(XD, XS, IT)                                                \
        V2X(REG(XD), 1, 1) EMITB(0x72)                                      \
        MRM(0x04,    MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IT)))

#define shrcn3ld(XD, XS, MT, DT)                                            \
        V2X(REG(XS), 1, 1) EMITB(0xE2)                                      \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* svl (G = G << S), (D = S << T) if (#D != #T) - variable, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define svlcx_rr(XG, XS)     /* variable shift with per-elem count */       \
        svlcx3rr(W(XG), W(XG), W(XS))

#define svlcx_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svlcx3ld(W(XG), W(XG), W(MS), W(DS))

#define svlcx3rr(XD, XS, XT)                                                \
        VEX(REG(XS), 1, 1, 2) EMITB(0x47)                                   \
        MRM(REG(XD), MOD(XT), REG(XT))

#define svlcx3ld(XD, XS, MT, DT)                                            \
        VEX(REG(XS), 1, 1, 2) EMITB(0x47)                                   \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* svr (G = G >> S), (D = S >> T) if (#D != #T) - variable, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define svrcx_rr(XG, XS)     /* variable shift with per-elem count */       \
        svrcx3rr(W(XG), W(XG), W(XS))

#define svrcx_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svrcx3ld(W(XG), W(XG), W(MS), W(DS))

#define svrcx3rr(XD, XS, XT)                                                \
        VEX(REG(XS), 1, 1, 2) EMITB(0x45)                                   \
        MRM(REG(XD), MOD(XT), REG(XT))

#define svrcx3ld(XD, XS, MT, DT)                                            \
        VEX(REG(XS), 1, 1, 2) EMITB(0x45)                                   \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* svr (G = G >> S), (D = S >> T) if (#D != #T) - variable, signed
 * for maximum compatibility: shift count must be modulo elem-size */

#define svrcn_rr(XG, XS)     /* variable shift with per-elem count */       \
        svrcn3rr(W(XG), W(XG), W(XS))

#define svrcn_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svrcn3ld(W(XG), W(XG), W(MS), W(DS))

#define svrcn3rr(XD, XS, XT)                                                \
        VEX(REG(XS), 1, 1, 2) EMITB(0x46)                                   \
        MRM(REG(XD), MOD(XT), REG(XT))

#define svrcn3ld(XD, XS, MT, DT)                                            \
        VEX(REG(XS), 1, 1, 2) EMITB(0x46)                                   \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

#endif /* RT_256X1 >= 2, AVX2 */

/****************   packed half-precision generic move/logic   ****************/

/* mov (D = S) */

#define movax_rr(XD, XS)                                                    \
        V2X(0x00,    1, 0) EMITB(0x28)                                      \
        MRM(REG(XD), MOD(XS), REG(XS))

#define movax_ld(XD, MS, DS)                                                \
        V2X(0x00,    1, 0) EMITB(0x28)                                      \
        MRM(REG(XD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define movax_st(XS, MD, DD)                                                \
        V2X(0x00,    1, 0) EMITB(0x29)                                      \
        MRM(REG(XS), MOD(MD), REG(MD))                                      \
        AUX(SIB(MD), CMD(DD), EMPTY)

/* mmv (G = G mask-merge S) where (mask-elem: 0 keeps G, -1 picks S)
 * uses Xmm0 implicitly as a mask register, destroys Xmm0, 0-masked XS elems */

#define mmvax_rr(XG, XS)                                                    \
        andax_rr(W(XS), Xmm0)                                               \
        annax_rr(Xmm0, W(XG))                                               \
        orrax_rr(Xmm0, W(XS))                                               \
        movax_rr(W(XG), Xmm0)

#define mmvax_ld(XG, MS, DS)                                                \
        notax_rx(Xmm0)                                                      \
        andax_rr(W(XG), Xmm0)                                               \
        annax_ld(Xmm0, W(MS), W(DS))                                        \
        orrax_rr(W(XG), Xmm0)

#define mmvax_st(XS, MG, DG)                                                \
        andax_rr(W(XS), Xmm0)                                               \
        annax_ld(Xmm0, W(MG), W(DG))                                        \
        orrax_rr(Xmm0, W(XS))                                               \
        movax_st(Xmm0, W(MG), W(DG))

/* and (G = G & S), (D = S & T) if (#D != #T) */

#define andax_rr(XG, XS)                                                    \
        andax3rr(W(XG), W(XG), W(XS))

#define andax_ld(XG, MS, DS)                                                \
        andax3ld(W(XG), W(XG), W(MS), W(DS))

#define andax3rr(XD, XS, XT)                                                \
        V2X(REG(XS), 1, 1) EMITB(0xDB)                                      \
        MRM(REG(XD), MOD(XT), REG(XT))

#define andax3ld(XD, XS, MT, DT)                                            \
        V2X(REG(XS), 1, 1) EMITB(0xDB)                                      \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* ann (G = ~G & S), (D = ~S & T) if (#D != #T) */

#define annax_rr(XG, XS)                                                    \
        annax3rr(W(XG), W(XG), W(XS))

#define annax_ld(XG, MS, DS)                                                \
        annax3ld(W(XG), W(XG), W(MS), W(DS))

#define annax3rr(XD, XS, XT)                                                \
        V2X(REG(XS), 1, 1) EMITB(0xDF)                                      \
        MRM(REG(XD), MOD(XT), REG(XT))

#define annax3ld(XD, XS, MT, DT)                                            \
        V2X(REG(XS), 1, 1) EMITB(0xDF)                                      \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* orr (G = G | S), (D = S | T) if (#D != #T) */

#define orrax_rr(XG, XS)                                                    \
        orrax3rr(W(XG), W(XG), W(XS))

#define orrax_ld(XG, MS, DS)                                                \
        orrax3ld(W(XG), W(XG), W(MS), W(DS))

#define orrax3rr(XD, XS, XT)                                                \
        V2X(REG(XS), 1, 1) EMITB(0xEB)                                      \
        MRM(REG(XD), MOD(XT), REG(XT))

#define orrax3ld(XD, XS, MT, DT)                                            \
        V2X(REG(XS), 1, 1) EMITB(0xEB)                                      \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* orn (G = ~G | S), (D = ~S | T) if (#D != #T) */

#define ornax_rr(XG, XS)                                                    \
        notax_rx(W(XG))                                                     \
        orrax_rr(W(XG), W(XS))

#define ornax_ld(XG, MS, DS)                                                \
        notax_rx(W(XG))                                                     \
        orrax_ld(W(XG), W(MS), W(DS))

#define ornax3rr(XD, XS, XT)                                                \
        notax_rr(W(XD), W(XS))                                              \
        orrax_rr(W(XD), W(XT))

#define ornax3ld(XD, XS, MT, DT)                                            \
        notax_rr(W(XD), W(XS))                                              \
        orrax_ld(W(XD), W(MT), W(DT))

/* xor (G = G ^ S), (D = S ^ T) if (#D != #T) */

#define xorax_rr(XG, XS)                                                    \
        xorax3rr(W(XG), W(XG), W(XS))

#define xorax_ld(XG, MS, DS)                                                \
        xorax3ld(W(XG), W(XG), W(MS), W(DS))

#define xorax3rr(XD, XS, XT)                                                \
        V2X(REG(XS), 1, 1) EMITB(0xEF)                                      \
        MRM(REG(XD), MOD(XT), REG(XT))

#define xorax3ld(XD, XS, MT, DT)                                            \
        V2X(REG(XS), 1, 1) EMITB(0xEF)                                      \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* not (G = ~G), (D = ~S) */

#define notax_rx(XG)                                                        \
        notax_rr(W(XG), W(XG))

#define notax_rr(XD, XS)                                                    \
        annax3ld(W(XD), W(XS), Mebp, inf_GPC07)

/*************   packed half-precision integer arithmetic/shifts   ************/

#if (RT_256X1 < 2)

/* add (G = G + S), (D = S + T) if (#D != #T) */

#define addax_rr(XG, XS)                                                    \
        addax3rr(W(XG), W(XG), W(XS))

#define addax_ld(XG, MS, DS)                                                \
        addax3ld(W(XG), W(XG), W(MS), W(DS))

#define addax3rr(XD, XS, XT)                                                \
        movax_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movax_st(W(XT), Mebp, inf_SCR02(0))                                 \
        addax_rx(W(XD))                                                     \
        movax_ld(W(XD), Mebp, inf_SCR01(0))

#define addax3ld(XD, XS, MT, DT)                                            \
        movax_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movax_ld(W(XD), W(MT), W(DT))                                       \
        movax_st(W(XD), Mebp, inf_SCR02(0))                                 \
        addax_rx(W(XD))                                                     \
        movax_ld(W(XD), Mebp, inf_SCR01(0))

#define addax_rx(XD) /* not portable, do not use outside */                 \
        movgx_ld(W(XD), Mebp, inf_SCR01(0x00))                              \
        addgx_ld(W(XD), Mebp, inf_SCR02(0x00))                              \
        movgx_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movgx_ld(W(XD), Mebp, inf_SCR01(0x10))                              \
        addgx_ld(W(XD), Mebp, inf_SCR02(0x10))                              \
        movgx_st(W(XD), Mebp, inf_SCR01(0x10))

/* ads (G = G + S), (D = S + T) if (#D != #T) - saturate, unsigned */

#define adsax_rr(XG, XS)                                                    \
        adsax3rr(W(XG), W(XG), W(XS))

#define adsax_ld(XG, MS, DS)                                                \
        adsax3ld(W(XG), W(XG), W(MS), W(DS))

#define adsax3rr(XD, XS, XT)                                                \
        movax_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movax_st(W(XT), Mebp, inf_SCR02(0))                                 \
        adsax_rx(W(XD))                                                     \
        movax_ld(W(XD), Mebp, inf_SCR01(0))

#define adsax3ld(XD, XS, MT, DT)                                            \
        movax_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movax_ld(W(XD), W(MT), W(DT))                                       \
        movax_st(W(XD), Mebp, inf_SCR02(0))                                 \
        adsax_rx(W(XD))                                                     \
        movax_ld(W(XD), Mebp, inf_SCR01(0))

#define adsax_rx(XD) /* not portable, do not use outside */                 \
        movgx_ld(W(XD), Mebp, inf_SCR01(0x00))                              \
        adsgx_ld(W(XD), Mebp, inf_SCR02(0x00))                              \
        movgx_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movgx_ld(W(XD), Mebp, inf_SCR01(0x10))                              \
        adsgx_ld(W(XD), Mebp, inf_SCR02(0x10))                              \
        movgx_st(W(XD), Mebp, inf_SCR01(0x10))

/* ads (G = G + S), (D = S + T) if (#D != #T) - saturate, signed */

#define adsan_rr(XG, XS)                                                    \
        adsan3rr(W(XG), W(XG), W(XS))

#define adsan_ld(XG, MS, DS)                                                \
        adsan3ld(W(XG), W(XG), W(MS), W(DS))

#define adsan3rr(XD, XS, XT)                                                \
        movax_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movax_st(W(XT), Mebp, inf_SCR02(0))                                 \
        adsan_rx(W(XD))                                                     \
        movax_ld(W(XD), Mebp, inf_SCR01(0))

#define adsan3ld(XD, XS, MT, DT)                                            \
        movax_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movax_ld(W(XD), W(MT), W(DT))                                       \
        movax_st(W(XD), Mebp, inf_SCR02(0))                                 \
        adsan_rx(W(XD))                                                     \
        movax_ld(W(XD), Mebp, inf_SCR01(0))

#define adsan_rx(XD) /* not portable, do not use outside */                 \
        movgx_ld(W(XD), Mebp, inf_SCR01(0x00))                              \
        adsgn_ld(W(XD), Mebp, inf_SCR02(0x00))                              \
        movgx_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movgx_ld(W(XD), Mebp, inf_SCR01(0x10))                              \
        adsgn_ld(W(XD), Mebp, inf_SCR02(0x10))                              \
        movgx_st(W(XD), Mebp, inf_SCR01(0x10))

/* sub (G = G - S), (D = S - T) if (#D != #T) */

#define subax_rr(XG, XS)                                                    \
        subax3rr(W(XG), W(XG), W(XS))

#define subax_ld(XG, MS, DS)                                                \
        subax3ld(W(XG), W(XG), W(MS), W(DS))

#define subax3rr(XD, XS, XT)                                                \
        movax_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movax_st(W(XT), Mebp, inf_SCR02(0))                                 \
        subax_rx(W(XD))                                                     \
        movax_ld(W(XD), Mebp, inf_SCR01(0))

#define subax3ld(XD, XS, MT, DT)                                            \
        movax_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movax_ld(W(XD), W(MT), W(DT))                                       \
        movax_st(W(XD), Mebp, inf_SCR02(0))                                 \
        subax_rx(W(XD))                                                     \
        movax_ld(W(XD), Mebp, inf_SCR01(0))

#define subax_rx(XD) /* not portable, do not use outside */                 \
        movgx_ld(W(XD), Mebp, inf_SCR01(0x00))                              \
        subgx_ld(W(XD), Mebp, inf_SCR02(0x00))                              \
        movgx_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movgx_ld(W(XD), Mebp, inf_SCR01(0x10))                              \
        subgx_ld(W(XD), Mebp, inf_SCR02(0x10))                              \
        movgx_st(W(XD), Mebp, inf_SCR01(0x10))

/* sbs (G = G - S), (D = S - T) if (#D != #T) - saturate, unsigned */

#define sbsax_rr(XG, XS)                                                    \
        sbsax3rr(W(XG), W(XG), W(XS))

#define sbsax_ld(XG, MS, DS)                                                \
        sbsax3ld(W(XG), W(XG), W(MS), W(DS))

#define sbsax3rr(XD, XS, XT)                                                \
        movax_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movax_st(W(XT), Mebp, inf_SCR02(0))                                 \
        sbsax_rx(W(XD))                                                     \
        movax_ld(W(XD), Mebp, inf_SCR01(0))

#define sbsax3ld(XD, XS, MT, DT)                                            \
        movax_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movax_ld(W(XD), W(MT), W(DT))                                       \
        movax_st(W(XD), Mebp, inf_SCR02(0))                                 \
        sbsax_rx(W(XD))                                                     \
        movax_ld(W(XD), Mebp, inf_SCR01(0))

#define sbsax_rx(XD) /* not portable, do not use outside */                 \
        movgx_ld(W(XD), Mebp, inf_SCR01(0x00))                              \
        sbsgx_ld(W(XD), Mebp, inf_SCR02(0x00))                              \
        movgx_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movgx_ld(W(XD), Mebp, inf_SCR01(0x10))                              \
        sbsgx_ld(W(XD), Mebp, inf_SCR02(0x10))                              \
        movgx_st(W(XD), Mebp, inf_SCR01(0x10))

/* sbs (G = G - S), (D = S - T) if (#D != #T) - saturate, signed */

#define sbsan_rr(XG, XS)                                                    \
        sbsan3rr(W(XG), W(XG), W(XS))

#define sbsan_ld(XG, MS, DS)                                                \
        sbsan3ld(W(XG), W(XG), W(MS), W(DS))

#define sbsan3rr(XD, XS, XT)                                                \
        movax_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movax_st(W(XT), Mebp, inf_SCR02(0))                                 \
        sbsan_rx(W(XD))                                                     \
        movax_ld(W(XD), Mebp, inf_SCR01(0))

#define sbsan3ld(XD, XS, MT, DT)                                            \
        movax_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movax_ld(W(XD), W(MT), W(DT))                                       \
        movax_st(W(XD), Mebp, inf_SCR02(0))                                 \
        sbsan_rx(W(XD))                                                     \
        movax_ld(W(XD), Mebp, inf_SCR01(0))

#define sbsan_rx(XD) /* not portable, do not use outside */                 \
        movgx_ld(W(XD), Mebp, inf_SCR01(0x00))                              \
        sbsgn_ld(W(XD), Mebp, inf_SCR02(0x00))                              \
        movgx_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movgx_ld(W(XD), Mebp, inf_SCR01(0x10))                              \
        sbsgn_ld(W(XD), Mebp, inf_SCR02(0x10))                              \
        movgx_st(W(XD), Mebp, inf_SCR01(0x10))

/* mul (G = G * S), (D = S * T) if (#D != #T) */

#define mulax_rr(XG, XS)                                                    \
        mulax3rr(W(XG), W(XG), W(XS))

#define mulax_ld(XG, MS, DS)                                                \
        mulax3ld(W(XG), W(XG), W(MS), W(DS))

#define mulax3rr(XD, XS, XT)                                                \
        movax_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movax_st(W(XT), Mebp, inf_SCR02(0))                                 \
        mulax_rx(W(XD))                                                     \
        movax_ld(W(XD), Mebp, inf_SCR01(0))

#define mulax3ld(XD, XS, MT, DT)                                            \
        movax_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movax_ld(W(XD), W(MT), W(DT))                                       \
        movax_st(W(XD), Mebp, inf_SCR02(0))                                 \
        mulax_rx(W(XD))                                                     \
        movax_ld(W(XD), Mebp, inf_SCR01(0))

#define mulax_rx(XD) /* not portable, do not use outside */                 \
        movgx_ld(W(XD), Mebp, inf_SCR01(0x00))                              \
        mulgx_ld(W(XD), Mebp, inf_SCR02(0x00))                              \
        movgx_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movgx_ld(W(XD), Mebp, inf_SCR01(0x10))                              \
        mulgx_ld(W(XD), Mebp, inf_SCR02(0x10))                              \
        movgx_st(W(XD), Mebp, inf_SCR01(0x10))

/* shl (G = G << S), (D = S << T) if (#D != #T) - plain, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define shlax_ri(XG, IS)                                                    \
        shlax3ri(W(XG), W(XG), W(IS))

#define shlax_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shlax3ld(W(XG), W(XG), W(MS), W(DS))

#define shlax3ri(XD, XS, IT)                                                \
        movax_st(W(XS), Mebp, inf_SCR01(0))                                 \
        shlgx3ri(W(XD), W(XS), W(IT))                                       \
        movgx_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movgx_ld(W(XD), Mebp, inf_SCR01(0x10))                              \
        shlgx_ri(W(XD), W(IT))                                              \
        movgx_st(W(XD), Mebp, inf_SCR01(0x10))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0))

#define shlax3ld(XD, XS, MT, DT)                                            \
        movax_st(W(XS), Mebp, inf_SCR01(0))                                 \
        shlgx3ld(W(XD), W(XS), W(MT), W(DT))                                \
        movgx_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movgx_ld(W(XD), Mebp, inf_SCR01(0x10))                              \
        shlgx_ld(W(XD), W(MT), W(DT))                                       \
        movgx_st(W(XD), Mebp, inf_SCR01(0x10))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0))

/* shr (G = G >> S), (D = S >> T) if (#D != #T) - plain, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define shrax_ri(XG, IS)                                                    \
        shrax3ri(W(XG), W(XG), W(IS))

#define shrax_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shrax3ld(W(XG), W(XG), W(MS), W(DS))

#define shrax3ri(XD, XS, IT)                                                \
        movax_st(W(XS), Mebp, inf_SCR01(0))                                 \
        shrgx3ri(W(XD), W(XS), W(IT))                                       \
        movgx_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movgx_ld(W(XD), Mebp, inf_SCR01(0x10))                              \
        shrgx_ri(W(XD), W(IT))                                              \
        movgx_st(W(XD), Mebp, inf_SCR01(0x10))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0))

#define shrax3ld(XD, XS, MT, DT)                                            \
        movax_st(W(XS), Mebp, inf_SCR01(0))                                 \
        shrgx3ld(W(XD), W(XS), W(MT), W(DT))                                \
        movgx_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movgx_ld(W(XD), Mebp, inf_SCR01(0x10))                              \
        shrgx_ld(W(XD), W(MT), W(DT))                                       \
        movgx_st(W(XD), Mebp, inf_SCR01(0x10))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0))

/* shr (G = G >> S), (D = S >> T) if (#D != #T) - plain, signed
 * for maximum compatibility: shift count must be modulo elem-size */

#define shran_ri(XG, IS)                                                    \
        shran3ri(W(XG), W(XG), W(IS))

#define shran_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shran3ld(W(XG), W(XG), W(MS), W(DS))

#define shran3ri(XD, XS, IT)                                                \
        movax_st(W(XS), Mebp, inf_SCR01(0))                                 \
        shrgn3ri(W(XD), W(XS), W(IT))                                       \
        movgx_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movgx_ld(W(XD), Mebp, inf_SCR01(0x10))                              \
        shrgn_ri(W(XD), W(IT))                                              \
        movgx_st(W(XD), Mebp, inf_SCR01(0x10))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0))

#define shran3ld(XD, XS, MT, DT)                                            \
        movax_st(W(XS), Mebp, inf_SCR01(0))                                 \
        shrgn3ld(W(XD), W(XS), W(MT), W(DT))                                \
        movgx_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movgx_ld(W(XD), Mebp, inf_SCR01(0x10))                              \
        shrgn_ld(W(XD), W(MT), W(DT))                                       \
        movgx_st(W(XD), Mebp, inf_SCR01(0x10))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0))

#else /* RT_256X1 >= 2, AVX2 */

/* add (G = G + S), (D = S + T) if (#D != #T) */

#define addax_rr(XG, XS)                                                    \
        addax3rr(W(XG), W(XG), W(XS))

#define addax_ld(XG, MS, DS)                                                \
        addax3ld(W(XG), W(XG), W(MS), W(DS))

#define addax3rr(XD, XS, XT)                                                \
        V2X(REG(XS), 1, 1) EMITB(0xFD)                                      \
        MRM(REG(XD), MOD(XT), REG(XT))

#define addax3ld(XD, XS, MT, DT)                                            \
        V2X(REG(XS), 1, 1) EMITB(0xFD)                                      \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* ads (G = G + S), (D = S + T) if (#D != #T) - saturate, unsigned */

#define adsax_rr(XG, XS)                                                    \
        adsax3rr(W(XG), W(XG), W(XS))

#define adsax_ld(XG, MS, DS)                                                \
        adsax3ld(W(XG), W(XG), W(MS), W(DS))

#define adsax3rr(XD, XS, XT)                                                \
        V2X(REG(XS), 1, 1) EMITB(0xDD)                                      \
        MRM(REG(XD), MOD(XT), REG(XT))

#define adsax3ld(XD, XS, MT, DT)                                            \
        V2X(REG(XS), 1, 1) EMITB(0xDD)                                      \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* ads (G = G + S), (D = S + T) if (#D != #T) - saturate, signed */

#define adsan_rr(XG, XS)                                                    \
        adsan3rr(W(XG), W(XG), W(XS))

#define adsan_ld(XG, MS, DS)                                                \
        adsan3ld(W(XG), W(XG), W(MS), W(DS))

#define adsan3rr(XD, XS, XT)                                                \
        V2X(REG(XS), 1, 1) EMITB(0xED)                                      \
        MRM(REG(XD), MOD(XT), REG(XT))

#define adsan3ld(XD, XS, MT, DT)                                            \
        V2X(REG(XS), 1, 1) EMITB(0xED)                                      \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* sub (G = G - S), (D = S - T) if (#D != #T) */

#define subax_rr(XG, XS)                                                    \
        subax3rr(W(XG), W(XG), W(XS))

#define subax_ld(XG, MS, DS)                                                \
        subax3ld(W(XG), W(XG), W(MS), W(DS))

#define subax3rr(XD, XS, XT)                                                \
        V2X(REG(XS), 1, 1) EMITB(0xF9)                                      \
        MRM(REG(XD), MOD(XT), REG(XT))

#define subax3ld(XD, XS, MT, DT)                                            \
        V2X(REG(XS), 1, 1) EMITB(0xF9)                                      \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* sbs (G = G - S), (D = S - T) if (#D != #T) - saturate, unsigned */

#define sbsax_rr(XG, XS)                                                    \
        sbsax3rr(W(XG), W(XG), W(XS))

#define sbsax_ld(XG, MS, DS)                                                \
        sbsax3ld(W(XG), W(XG), W(MS), W(DS))

#define sbsax3rr(XD, XS, XT)                                                \
        V2X(REG(XS), 1, 1) EMITB(0xD9)                                      \
        MRM(REG(XD), MOD(XT), REG(XT))

#define sbsax3ld(XD, XS, MT, DT)                                            \
        V2X(REG(XS), 1, 1) EMITB(0xD9)                                      \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* sbs (G = G - S), (D = S - T) if (#D != #T) - saturate, signed */

#define sbsan_rr(XG, XS)                                                    \
        sbsan3rr(W(XG), W(XG), W(XS))

#define sbsan_ld(XG, MS, DS)                                                \
        sbsan3ld(W(XG), W(XG), W(MS), W(DS))

#define sbsan3rr(XD, XS, XT)                                                \
        V2X(REG(XS), 1, 1) EMITB(0xE9)                                      \
        MRM(REG(XD), MOD(XT), REG(XT))

#define sbsan3ld(XD, XS, MT, DT)                                            \
        V2X(REG(XS), 1, 1) EMITB(0xE9)                                      \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* mul (G = G * S), (D = S * T) if (#D != #T) */

#define mulax_rr(XG, XS)                                                    \
        mulax3rr(W(XG), W(XG), W(XS))

#define mulax_ld(XG, MS, DS)                                                \
        mulax3ld(W(XG), W(XG), W(MS), W(DS))

#define mulax3rr(XD, XS, XT)                                                \
        V2X(REG(XS), 1, 1) EMITB(0xD5)                                      \
        MRM(REG(XD), MOD(XT), REG(XT))

#define mulax3ld(XD, XS, MT, DT)                                            \
        V2X(REG(XS), 1, 1) EMITB(0xD5)                                      \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* shl (G = G << S), (D = S << T) if (#D != #T) - plain, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define shlax_ri(XG, IS)                                                    \
        shlax3ri(W(XG), W(XG), W(IS))

#define shlax_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shlax3ld(W(XG), W(XG), W(MS), W(DS))

#define shlax3ri(XD, XS, IT)                                                \
        V2X(REG(XD), 1, 1) EMITB(0x71)                                      \
        MRM(0x06,    MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IT)))

#define shlax3ld(XD, XS, MT, DT)                                            \
        V2X(REG(XS), 1, 1) EMITB(0xF1)                                      \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* shr (G = G >> S), (D = S >> T) if (#D != #T) - plain, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define shrax_ri(XG, IS)                                                    \
        shrax3ri(W(XG), W(XG), W(IS))

#define shrax_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shrax3ld(W(XG), W(XG), W(MS), W(DS))

#define shrax3ri(XD, XS, IT)                                                \
        V2X(REG(XD), 1, 1) EMITB(0x71)                                      \
        MRM(0x02,    MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IT)))

#define shrax3ld(XD, XS, MT, DT)                                            \
        V2X(REG(XS), 1, 1) EMITB(0xD1)                                      \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* shr (G = G >> S), (D = S >> T) if (#D != #T) - plain, signed
 * for maximum compatibility: shift count must be modulo elem-size */

#define shran_ri(XG, IS)                                                    \
        shran3ri(W(XG), W(XG), W(IS))

#define shran_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shran3ld(W(XG), W(XG), W(MS), W(DS))

#define shran3ri(XD, XS, IT)                                                \
        V2X(REG(XD), 1, 1) EMITB(0x71)                                      \
        MRM(0x04,    MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IT)))

#define shran3ld(XD, XS, MT, DT)                                            \
        V2X(REG(XS), 1, 1) EMITB(0xE1)                                      \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

#endif /* RT_256X1 >= 2, AVX2 */

/* svl (G = G << S), (D = S << T) if (#D != #T) - variable, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define svlax_rr(XG, XS)     /* variable shift with per-elem count */       \
        svlax3rr(W(XG), W(XG), W(XS))

#define svlax_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svlax3ld(W(XG), W(XG), W(MS), W(DS))

#define svlax3rr(XD, XS, XT)                                                \
        movax_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movax_st(W(XT), Mebp, inf_SCR02(0))                                 \
        svlax_xx()                                                          \
        movax_ld(W(XD), Mebp, inf_SCR01(0))

#define svlax3ld(XD, XS, MT, DT)                                            \
        movax_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movax_ld(W(XD), W(MT), W(DT))                                       \
        movax_st(W(XD), Mebp, inf_SCR02(0))                                 \
        svlax_xx()                                                          \
        movax_ld(W(XD), Mebp, inf_SCR01(0))

#define svlax_xx() /* not portable, do not use outside */                   \
        stack_st(Recx)                                                      \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x00))                              \
        shlhx_mx(Mebp,  inf_SCR01(0x00))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x02))                              \
        shlhx_mx(Mebp,  inf_SCR01(0x02))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x04))                              \
        shlhx_mx(Mebp,  inf_SCR01(0x04))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x06))                              \
        shlhx_mx(Mebp,  inf_SCR01(0x06))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x08))                              \
        shlhx_mx(Mebp,  inf_SCR01(0x08))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x0A))                              \
        shlhx_mx(Mebp,  inf_SCR01(0x0A))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x0C))                              \
        shlhx_mx(Mebp,  inf_SCR01(0x0C))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x0E))                              \
        shlhx_mx(Mebp,  inf_SCR01(0x0E))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x10))                              \
        shlhx_mx(Mebp,  inf_SCR01(0x10))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x12))                              \
        shlhx_mx(Mebp,  inf_SCR01(0x12))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x14))                              \
        shlhx_mx(Mebp,  inf_SCR01(0x14))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x16))                              \
        shlhx_mx(Mebp,  inf_SCR01(0x16))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x18))                              \
        shlhx_mx(Mebp,  inf_SCR01(0x18))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x1A))                              \
        shlhx_mx(Mebp,  inf_SCR01(0x1A))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x1C))                              \
        shlhx_mx(Mebp,  inf_SCR01(0x1C))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x1E))                              \
        shlhx_mx(Mebp,  inf_SCR01(0x1E))                                    \
        stack_ld(Recx)

/* svr (G = G >> S), (D = S >> T) if (#D != #T) - variable, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define svrax_rr(XG, XS)     /* variable shift with per-elem count */       \
        svrax3rr(W(XG), W(XG), W(XS))

#define svrax_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svrax3ld(W(XG), W(XG), W(MS), W(DS))

#define svrax3rr(XD, XS, XT)                                                \
        movax_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movax_st(W(XT), Mebp, inf_SCR02(0))                                 \
        svrax_xx()                                                          \
        movax_ld(W(XD), Mebp, inf_SCR01(0))

#define svrax3ld(XD, XS, MT, DT)                                            \
        movax_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movax_ld(W(XD), W(MT), W(DT))                                       \
        movax_st(W(XD), Mebp, inf_SCR02(0))                                 \
        svrax_xx()                                                          \
        movax_ld(W(XD), Mebp, inf_SCR01(0))

#define svrax_xx() /* not portable, do not use outside */                   \
        stack_st(Recx)                                                      \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x00))                              \
        shrhx_mx(Mebp,  inf_SCR01(0x00))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x02))                              \
        shrhx_mx(Mebp,  inf_SCR01(0x02))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x04))                              \
        shrhx_mx(Mebp,  inf_SCR01(0x04))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x06))                              \
        shrhx_mx(Mebp,  inf_SCR01(0x06))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x08))                              \
        shrhx_mx(Mebp,  inf_SCR01(0x08))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x0A))                              \
        shrhx_mx(Mebp,  inf_SCR01(0x0A))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x0C))                              \
        shrhx_mx(Mebp,  inf_SCR01(0x0C))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x0E))                              \
        shrhx_mx(Mebp,  inf_SCR01(0x0E))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x10))                              \
        shrhx_mx(Mebp,  inf_SCR01(0x10))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x12))                              \
        shrhx_mx(Mebp,  inf_SCR01(0x12))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x14))                              \
        shrhx_mx(Mebp,  inf_SCR01(0x14))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x16))                              \
        shrhx_mx(Mebp,  inf_SCR01(0x16))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x18))                              \
        shrhx_mx(Mebp,  inf_SCR01(0x18))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x1A))                              \
        shrhx_mx(Mebp,  inf_SCR01(0x1A))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x1C))                              \
        shrhx_mx(Mebp,  inf_SCR01(0x1C))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x1E))                              \
        shrhx_mx(Mebp,  inf_SCR01(0x1E))                                    \
        stack_ld(Recx)

/* svr (G = G >> S), (D = S >> T) if (#D != #T) - variable, signed
 * for maximum compatibility: shift count must be modulo elem-size */

#define svran_rr(XG, XS)     /* variable shift with per-elem count */       \
        svran3rr(W(XG), W(XG), W(XS))

#define svran_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svran3ld(W(XG), W(XG), W(MS), W(DS))

#define svran3rr(XD, XS, XT)                                                \
        movax_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movax_st(W(XT), Mebp, inf_SCR02(0))                                 \
        svran_xx()                                                          \
        movax_ld(W(XD), Mebp, inf_SCR01(0))

#define svran3ld(XD, XS, MT, DT)                                            \
        movax_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movax_ld(W(XD), W(MT), W(DT))                                       \
        movax_st(W(XD), Mebp, inf_SCR02(0))                                 \
        svran_xx()                                                          \
        movax_ld(W(XD), Mebp, inf_SCR01(0))

#define svran_xx() /* not portable, do not use outside */                   \
        stack_st(Recx)                                                      \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x00))                              \
        shrhn_mx(Mebp,  inf_SCR01(0x00))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x02))                              \
        shrhn_mx(Mebp,  inf_SCR01(0x02))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x04))                              \
        shrhn_mx(Mebp,  inf_SCR01(0x04))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x06))                              \
        shrhn_mx(Mebp,  inf_SCR01(0x06))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x08))                              \
        shrhn_mx(Mebp,  inf_SCR01(0x08))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x0A))                              \
        shrhn_mx(Mebp,  inf_SCR01(0x0A))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x0C))                              \
        shrhn_mx(Mebp,  inf_SCR01(0x0C))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x0E))                              \
        shrhn_mx(Mebp,  inf_SCR01(0x0E))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x10))                              \
        shrhn_mx(Mebp,  inf_SCR01(0x10))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x12))                              \
        shrhn_mx(Mebp,  inf_SCR01(0x12))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x14))                              \
        shrhn_mx(Mebp,  inf_SCR01(0x14))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x16))                              \
        shrhn_mx(Mebp,  inf_SCR01(0x16))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x18))                              \
        shrhn_mx(Mebp,  inf_SCR01(0x18))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x1A))                              \
        shrhn_mx(Mebp,  inf_SCR01(0x1A))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x1C))                              \
        shrhn_mx(Mebp,  inf_SCR01(0x1C))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x1E))                              \
        shrhn_mx(Mebp,  inf_SCR01(0x1E))                                    \
        stack_ld(Recx)

/******************************************************************************/
/********************************   INTERNAL   ********************************/
/******************************************************************************/

#if   (RT_SIMD == 256)

#define muvcx_ld(XD, MS, DS) /* not portable, do not use outside */         \
        V2X(0x00,    1, 0) EMITB(0x28)                                      \
        MRM(REG(XD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define muvcx_st(XS, MD, DD) /* not portable, do not use outside */         \
        V2X(0x00,    1, 0) EMITB(0x29)                                      \
        MRM(REG(XS), MOD(MD), REG(MD))                                      \
        AUX(SIB(MD), CMD(DD), EMPTY)

#elif (RT_SIMD == 128)

#define muvcx_ld(XD, MS, DS) /* not portable, do not use outside */         \
        V2X(0x00,    1, 0) EMITB(0x10)                                      \
        MRM(REG(XD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define muvcx_st(XS, MD, DD) /* not portable, do not use outside */         \
        V2X(0x00,    1, 0) EMITB(0x11)                                      \
        MRM(REG(XS), MOD(MD), REG(MD))                                      \
        AUX(SIB(MD), CMD(DD), EMPTY)

#endif /* RT_SIMD: 256, 128 */

/* sregs */

#undef  sregs_sa
#define sregs_sa() /* save all SIMD regs, destroys Reax */                  \
        movxx_ld(Reax, Mebp, inf_REGS)                                      \
        muvcx_st(Xmm0, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_256*4))                           \
        muvcx_st(Xmm1, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_256*4))                           \
        muvcx_st(Xmm2, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_256*4))                           \
        muvcx_st(Xmm3, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_256*4))                           \
        muvcx_st(Xmm4, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_256*4))                           \
        muvcx_st(Xmm5, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_256*4))                           \
        muvcx_st(Xmm6, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_256*4))                           \
        muvcx_st(Xmm7, Oeax, PLAIN)

#undef  sregs_la
#define sregs_la() /* load all SIMD regs, destroys Reax */                  \
        movxx_ld(Reax, Mebp, inf_REGS)                                      \
        muvcx_ld(Xmm0, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_256*4))                           \
        muvcx_ld(Xmm1, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_256*4))                           \
        muvcx_ld(Xmm2, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_256*4))                           \
        muvcx_ld(Xmm3, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_256*4))                           \
        muvcx_ld(Xmm4, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_256*4))                           \
        muvcx_ld(Xmm5, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_256*4))                           \
        muvcx_ld(Xmm6, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_256*4))                           \
        muvcx_ld(Xmm7, Oeax, PLAIN)

#endif /* RT_256X1 */

#endif /* RT_SIMD_CODE */

#endif /* RT_RTARCH_X86_256X1V2_H */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
