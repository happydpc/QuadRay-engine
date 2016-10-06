/******************************************************************************/
/* Copyright (c) 2013-2016 VectorChief (at github, bitbucket, sourceforge)    */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#ifndef RT_RTARCH_P64_128_H
#define RT_RTARCH_P64_128_H

#if   RT_ADDRESS == 32

#include "rtarch_p32.h"

#elif RT_ADDRESS == 64

#include "rtarch_p64.h"

#else  /* RT_ADDRESS */

#error "unsupported address size, check RT_ADDRESS in makefiles"

#endif /* RT_ADDRESS */

#define RT_SIMD_REGS        16
#define RT_SIMD_WIDTH       2
#define RT_SIMD_ALIGN       16
#define RT_SIMD_SET(s, v)   s[0]=s[1]=v

#if defined (RT_SIMD_CODE)

#undef  sregs_sa
#undef  sregs_la
#undef  movpx_ld
#undef  EMITS
#define EMITS(w) EMITW(w)

/******************************************************************************/
/*********************************   LEGEND   *********************************/
/******************************************************************************/

/*
 * rtarch_p64_128.h: Implementation of Power fp64 VMX/VSX instructions.
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

/* structural */

#define MXM(reg, ren, rem)                                                  \
        ((rem) << 11 | (ren) << 16 | (reg) << 21)

#define MPM(reg, brm, vdp, bxx, pxx)                                        \
        (pxx(vdp) | bxx(brm) << 16 | (reg) << 21)

/* selectors  */

#define  B2(val, tp1, tp2)  B2##tp2
#define  P2(val, tp1, tp2)  P2##tp2
#define  C2(val, tp1, tp2)  C2##tp2

/* displacement encoding SIMD(TP2) */

#define B20(br) (br)
#define P20(dp) (0x00000000 | ((dp) & 0x7FF0))
#define C20(br, dp) EMPTY

#define B21(br) (br)
#define P21(dp) (0x44000214 | TDxx << 11)
#define C21(br, dp) EMITW(0x60000000 | TDxx << 16 | (0xFFF0 & (dp)))

#define B22(br) (br)
#define P22(dp) (0x44000214 | TDxx << 11)
#define C22(br, dp) EMITW(0x64000000 | TDxx << 16 | (0x7FFF & (dp) >> 16))  \
                    EMITW(0x60000000 | TDxx << 16 | TDxx << 21 |            \
                                                    (0xFFF0 & (dp)))

/* registers    REG   (check mapping with ASM_ENTER/ASM_LEAVE in rtarch.h) */

#define TmmR    0x17  /* v23, Rounding Mode */
#define TmmS    0x18  /* v24, SIGN */
#define TmmQ    0x19  /* v25, QNAN */
#define TmmA    0x1A  /* v26, +1.0 */
#define TmmB    0x1B  /* v27, -0.5 */
#define TmmC    0x1C  /* v28 */
#define TmmD    0x1D  /* v29 */
#define TmmE    0x1E  /* v30 */
#define Tmm1    0x1F  /* v31 */

/******************************************************************************/
/********************************   EXTERNAL   ********************************/
/******************************************************************************/

/* registers    REG,  MOD,  SIB */

#define Xmm0    0x00, 0x00, EMPTY       /* v0 */
#define Xmm1    0x01, 0x00, EMPTY       /* v1 */
#define Xmm2    0x02, 0x00, EMPTY       /* v2 */
#define Xmm3    0x03, 0x00, EMPTY       /* v3 */
#define Xmm4    0x04, 0x00, EMPTY       /* v4 */
#define Xmm5    0x05, 0x00, EMPTY       /* v5 */
#define Xmm6    0x06, 0x00, EMPTY       /* v6 */
#define Xmm7    0x07, 0x00, EMPTY       /* v7 */
#define Xmm8    0x08, 0x00, EMPTY       /* v8 */
#define Xmm9    0x09, 0x00, EMPTY       /* v9 */
#define XmmA    0x0A, 0x00, EMPTY       /* v10 */
#define XmmB    0x0B, 0x00, EMPTY       /* v11 */
#define XmmC    0x0C, 0x00, EMPTY       /* v12 */
#define XmmD    0x0D, 0x00, EMPTY       /* v13 */
#define XmmE    0x0E, 0x00, EMPTY       /* v14 */
#define XmmF    0x0F, 0x00, EMPTY       /* v15 */

#if (RT_128 >= 2)

/******************************************************************************/
/**********************************   VSX   ***********************************/
/******************************************************************************/

/**************************   packed generic (SIMD)   *************************/

/* mov */

#define movpx_rr(RG, RM)                                                    \
        EMITW(0xF0000497 | MXM(REG(RG), REG(RM), REG(RM)))

#define movpx_ld(RG, RM, DP)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x7C000699 | MXM(REG(RG), Teax & (MOD(RM) == TPxx), TPxx))    \
                                                       /* ^ == -1 if true */

#define movpx_st(RG, RM, DP)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x7C000799 | MXM(REG(RG), Teax & (MOD(RM) == TPxx), TPxx))    \
                                                       /* ^ == -1 if true */

#define adrpx_ld(RG, RM, DP) /* RG is a BASE reg, DP is SIMD-aligned */     \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x38000000 | MPM(REG(RG), MOD(RM), VAL(DP), B2(DP), P2(DP)))

/* and */

#define andpx_rr(RG, RM)                                                    \
        EMITW(0xF0000417 | MXM(REG(RG), REG(RG), REG(RM)))

#define andpx_ld(RG, RM, DP)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x7C000699 | MXM(Tmm1,    Teax & (MOD(RM) == TPxx), TPxx))    \
        EMITW(0xF0000417 | MXM(REG(RG), REG(RG), Tmm1))/* ^ == -1 if true */

/* ann */

#define annpx_rr(RG, RM)                                                    \
        EMITW(0xF0000457 | MXM(REG(RG), REG(RM), REG(RG)))

#define annpx_ld(RG, RM, DP)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x7C000699 | MXM(Tmm1,    Teax & (MOD(RM) == TPxx), TPxx))    \
        EMITW(0xF0000457 | MXM(REG(RG), Tmm1,    REG(RG)))/* ^ == -1 if true */

/* orr */

#define orrpx_rr(RG, RM)                                                    \
        EMITW(0xF0000497 | MXM(REG(RG), REG(RG), REG(RM)))

#define orrpx_ld(RG, RM, DP)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x7C000699 | MXM(Tmm1,    Teax & (MOD(RM) == TPxx), TPxx))    \
        EMITW(0xF0000497 | MXM(REG(RG), REG(RG), Tmm1))/* ^ == -1 if true */

/* xor */

#define xorpx_rr(RG, RM)                                                    \
        EMITW(0xF00004D7 | MXM(REG(RG), REG(RG), REG(RM)))

#define xorpx_ld(RG, RM, DP)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x7C000699 | MXM(Tmm1,    Teax & (MOD(RM) == TPxx), TPxx))    \
        EMITW(0xF00004D7 | MXM(REG(RG), REG(RG), Tmm1))/* ^ == -1 if true */

/**************   packed double precision floating point (SIMD)   *************/

/* add */

#define addps_rr(RG, RM)                                                    \
        EMITW(0xF0000307 | MXM(REG(RG), REG(RG), REG(RM)))

#define addps_ld(RG, RM, DP)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x7C000699 | MXM(Tmm1,    Teax & (MOD(RM) == TPxx), TPxx))    \
        EMITW(0xF0000307 | MXM(REG(RG), REG(RG), Tmm1))/* ^ == -1 if true */

/* sub */

#define subps_rr(RG, RM)                                                    \
        EMITW(0xF0000347 | MXM(REG(RG), REG(RG), REG(RM)))

#define subps_ld(RG, RM, DP)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x7C000699 | MXM(Tmm1,    Teax & (MOD(RM) == TPxx), TPxx))    \
        EMITW(0xF0000347 | MXM(REG(RG), REG(RG), Tmm1))/* ^ == -1 if true */

/* mul */

#define mulps_rr(RG, RM)                                                    \
        EMITW(0xF0000387 | MXM(REG(RG), REG(RG), REG(RM)))

#define mulps_ld(RG, RM, DP)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x7C000699 | MXM(Tmm1,    Teax & (MOD(RM) == TPxx), TPxx))    \
        EMITW(0xF0000387 | MXM(REG(RG), REG(RG), Tmm1))/* ^ == -1 if true */

/* div */

#define divps_rr(RG, RM)                                                    \
        EMITW(0xF00003C7 | MXM(REG(RG), REG(RG), REG(RM)))

#define divps_ld(RG, RM, DP)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x7C000699 | MXM(Tmm1,    Teax & (MOD(RM) == TPxx), TPxx))    \
        EMITW(0xF00003C7 | MXM(REG(RG), REG(RG), Tmm1))/* ^ == -1 if true */

/* sqr */

#define sqrps_rr(RG, RM)                                                    \
        EMITW(0xF000032F | MXM(REG(RG), 0x00,    REG(RM)))

#define sqrps_ld(RG, RM, DP)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x7C000699 | MXM(Tmm1,    Teax & (MOD(RM) == TPxx), TPxx))    \
        EMITW(0xF000032F | MXM(REG(RG), 0x00,    Tmm1))/* ^ == -1 if true */

/* cbr */

        /* cbe, cbs, cbr defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* rcp
 * accuracy/behavior may vary across supported targets, use accordingly */

#if RT_SIMD_COMPAT_RCP == 0

#define rceps_rr(RG, RM)                                                    \
        EMITW(0xF000036B | MXM(REG(RG), 0x00,    REG(RM)))

#define rcsps_rr(RG, RM) /* destroys RM */                                  \
        EMITW(0xF00007CF | MXM(REG(RM), REG(RG), TmmA))                     \
        EMITW(0xF000030F | MXM(REG(RG), REG(RG), REG(RM)))

#endif /* RT_SIMD_COMPAT_RCP */

        /* rcp defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* rsq
 * accuracy/behavior may vary across supported targets, use accordingly */

#if RT_SIMD_COMPAT_RSQ == 0

#define rseps_rr(RG, RM)                                                    \
        EMITW(0xF000032B | MXM(REG(RG), 0x00,    REG(RM)))

#define rssps_rr(RG, RM) /* destroys RM */                                  \
        EMITW(0xF0000387 | MXM(TmmD,    REG(RG), REG(RG)))                  \
        EMITW(0xF0000387 | MXM(TmmC,    REG(RG), TmmB))                     \
        EMITW(0xF00007CF | MXM(TmmD,    REG(RM), TmmA))                     \
        EMITW(0xF000078F | MXM(REG(RG), TmmD,    TmmC))

#endif /* RT_SIMD_COMPAT_RSQ */

        /* rsq defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* min */

#define minps_rr(RG, RM)                                                    \
        EMITW(0xF0000747 | MXM(REG(RG), REG(RG), REG(RM)))

#define minps_ld(RG, RM, DP)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x7C000699 | MXM(Tmm1,    Teax & (MOD(RM) == TPxx), TPxx))    \
        EMITW(0xF0000747 | MXM(REG(RG), REG(RG), Tmm1))/* ^ == -1 if true */

/* max */

#define maxps_rr(RG, RM)                                                    \
        EMITW(0xF0000707 | MXM(REG(RG), REG(RG), REG(RM)))

#define maxps_ld(RG, RM, DP)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x7C000699 | MXM(Tmm1,    Teax & (MOD(RM) == TPxx), TPxx))    \
        EMITW(0xF0000707 | MXM(REG(RG), REG(RG), Tmm1))/* ^ == -1 if true */

/* cmp */

#define ceqps_rr(RG, RM)                                                    \
        EMITW(0xF000031F | MXM(REG(RG), REG(RG), REG(RM)))

#define ceqps_ld(RG, RM, DP)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x7C000699 | MXM(Tmm1,    Teax & (MOD(RM) == TPxx), TPxx))    \
        EMITW(0xF000031F | MXM(REG(RG), REG(RG), Tmm1))/* ^ == -1 if true */

#define cneps_rr(RG, RM)                                                    \
        EMITW(0xF000031F | MXM(REG(RG), REG(RG), REG(RM)))                  \
        EMITW(0xF0000517 | MXM(REG(RG), REG(RG), REG(RG)))

#define cneps_ld(RG, RM, DP)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x7C000699 | MXM(Tmm1,    Teax & (MOD(RM) == TPxx), TPxx))    \
        EMITW(0xF000031F | MXM(REG(RG), REG(RG), Tmm1))/* ^ == -1 if true */\
        EMITW(0xF0000517 | MXM(REG(RG), REG(RG), REG(RG)))

#define cltps_rr(RG, RM)                                                    \
        EMITW(0xF000035F | MXM(REG(RG), REG(RM), REG(RG)))

#define cltps_ld(RG, RM, DP)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x7C000699 | MXM(Tmm1,    Teax & (MOD(RM) == TPxx), TPxx))    \
        EMITW(0xF000035F | MXM(REG(RG), Tmm1,    REG(RG)))/* ^ == -1 if true */

#define cleps_rr(RG, RM)                                                    \
        EMITW(0xF000039F | MXM(REG(RG), REG(RM), REG(RG)))

#define cleps_ld(RG, RM, DP)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x7C000699 | MXM(Tmm1,    Teax & (MOD(RM) == TPxx), TPxx))    \
        EMITW(0xF000039F | MXM(REG(RG), Tmm1,    REG(RG)))/* ^ == -1 if true */

#define cgtps_rr(RG, RM)                                                    \
        EMITW(0xF000035F | MXM(REG(RG), REG(RG), REG(RM)))

#define cgtps_ld(RG, RM, DP)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x7C000699 | MXM(Tmm1,    Teax & (MOD(RM) == TPxx), TPxx))    \
        EMITW(0xF000035F | MXM(REG(RG), REG(RG), Tmm1))/* ^ == -1 if true */

#define cgeps_rr(RG, RM)                                                    \
        EMITW(0xF000039F | MXM(REG(RG), REG(RG), REG(RM)))

#define cgeps_ld(RG, RM, DP)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x7C000699 | MXM(Tmm1,    Teax & (MOD(RM) == TPxx), TPxx))    \
        EMITW(0xF000039F | MXM(REG(RG), REG(RG), Tmm1))/* ^ == -1 if true */

/* cvz (fp-to-signed-int)
 * rounding mode is encoded directly (can be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 64-bit signed int range */

#define rnzps_rr(RG, RM)     /* round towards zero */                       \
        EMITW(0xF0000367 | MXM(REG(RG), 0x00,    REG(RM)))

#define rnzps_ld(RG, RM, DP) /* round towards zero */                       \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x7C000699 | MXM(Tmm1,    Teax & (MOD(RM) == TPxx), TPxx))    \
        EMITW(0xF0000367 | MXM(REG(RG), 0x00,    Tmm1))/* ^ == -1 if true */

#define cvzps_rr(RG, RM)     /* round towards zero */                       \
        EMITW(0xF0000763 | MXM(REG(RG), 0x00,    REG(RM)))

#define cvzps_ld(RG, RM, DP) /* round towards zero */                       \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x7C000699 | MXM(Tmm1,    Teax & (MOD(RM) == TPxx), TPxx))    \
        EMITW(0xF0000763 | MXM(REG(RG), 0x00,    Tmm1))/* ^ == -1 if true */

/* cvp (fp-to-signed-int)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 64-bit signed int range */

#define rnpps_rr(RG, RM)     /* round towards +inf */                       \
        EMITW(0xF00003A7 | MXM(REG(RG), 0x00,    REG(RM)))

#define rnpps_ld(RG, RM, DP) /* round towards +inf */                       \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x7C000699 | MXM(Tmm1,    Teax & (MOD(RM) == TPxx), TPxx))    \
        EMITW(0xF00003A7 | MXM(REG(RG), 0x00,    Tmm1))/* ^ == -1 if true */

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
        EMITW(0xF00003E7 | MXM(REG(RG), 0x00,    REG(RM)))

#define rnmps_ld(RG, RM, DP) /* round towards -inf */                       \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x7C000699 | MXM(Tmm1,    Teax & (MOD(RM) == TPxx), TPxx))    \
        EMITW(0xF00003E7 | MXM(REG(RG), 0x00,    Tmm1))/* ^ == -1 if true */

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
        EMITW(0xF00003AF | MXM(REG(RG), 0x00,    REG(RM)))

#define rnnps_ld(RG, RM, DP) /* round towards near */                       \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x7C000699 | MXM(Tmm1,    Teax & (MOD(RM) == TPxx), TPxx))    \
        EMITW(0xF00003AF | MXM(REG(RG), 0x00,    Tmm1))/* ^ == -1 if true */

#define cvnps_rr(RG, RM)     /* round towards near */                       \
        rnnps_rr(W(RG), W(RM))                                              \
        cvzps_rr(W(RG), W(RG))

#define cvnps_ld(RG, RM, DP) /* round towards near */                       \
        rnnps_ld(W(RG), W(RM), W(DP))                                       \
        cvzps_rr(W(RG), W(RG))

/* cvn (signed-int-to-fp)
 * rounding mode encoded directly (cannot be used in FCTRL blocks) */

#define cvnpn_rr(RG, RM)     /* round towards near */                       \
        cvtpn_rr(W(RG), W(RM))

#define cvnpn_ld(RG, RM, DP) /* round towards near */                       \
        cvtpn_ld(W(RG), W(RM), W(DP))

/**************************   packed integer (SIMD)   *************************/

#if (RT_128 < 4)

/* add */

#define addpx_rr(RG, RM)                                                    \
        movpx_st(W(RG), Mebp, inf_SCR01(0))                                 \
        movpx_st(W(RM), Mebp, inf_SCR02(0))                                 \
        stack_st(Reax)                                                      \
        movyx_ld(Reax,  Mebp, inf_SCR02(0x00))                              \
        addyx_st(Reax,  Mebp, inf_SCR01(0x00))                              \
        movyx_ld(Reax,  Mebp, inf_SCR02(0x08))                              \
        addyx_st(Reax,  Mebp, inf_SCR01(0x08))                              \
        stack_ld(Reax)                                                      \
        movpx_ld(W(RG), Mebp, inf_SCR01(0))

#define addpx_ld(RG, RM, DP)                                                \
        movpx_st(W(RG), Mebp, inf_SCR01(0))                                 \
        movpx_ld(W(RG), W(RM), W(DP))                                       \
        movpx_st(W(RG), Mebp, inf_SCR02(0))                                 \
        stack_st(Reax)                                                      \
        movyx_ld(Reax,  Mebp, inf_SCR02(0x00))                              \
        addyx_st(Reax,  Mebp, inf_SCR01(0x00))                              \
        movyx_ld(Reax,  Mebp, inf_SCR02(0x08))                              \
        addyx_st(Reax,  Mebp, inf_SCR01(0x08))                              \
        stack_ld(Reax)                                                      \
        movpx_ld(W(RG), Mebp, inf_SCR01(0))

/* sub */

#define subpx_rr(RG, RM)                                                    \
        movpx_st(W(RG), Mebp, inf_SCR01(0))                                 \
        movpx_st(W(RM), Mebp, inf_SCR02(0))                                 \
        stack_st(Reax)                                                      \
        movyx_ld(Reax,  Mebp, inf_SCR02(0x00))                              \
        subyx_st(Reax,  Mebp, inf_SCR01(0x00))                              \
        movyx_ld(Reax,  Mebp, inf_SCR02(0x08))                              \
        subyx_st(Reax,  Mebp, inf_SCR01(0x08))                              \
        stack_ld(Reax)                                                      \
        movpx_ld(W(RG), Mebp, inf_SCR01(0))

#define subpx_ld(RG, RM, DP)                                                \
        movpx_st(W(RG), Mebp, inf_SCR01(0))                                 \
        movpx_ld(W(RG), W(RM), W(DP))                                       \
        movpx_st(W(RG), Mebp, inf_SCR02(0))                                 \
        stack_st(Reax)                                                      \
        movyx_ld(Reax,  Mebp, inf_SCR02(0x00))                              \
        subyx_st(Reax,  Mebp, inf_SCR01(0x00))                              \
        movyx_ld(Reax,  Mebp, inf_SCR02(0x08))                              \
        subyx_st(Reax,  Mebp, inf_SCR01(0x08))                              \
        stack_ld(Reax)                                                      \
        movpx_ld(W(RG), Mebp, inf_SCR01(0))

/* shl */

#define shlpx_ri(RM, IM)                                                    \
        movpx_st(W(RM), Mebp, inf_SCR01(0))                                 \
        shlyx_mi(Mebp,  inf_SCR01(0x00), W(IM))                             \
        shlyx_mi(Mebp,  inf_SCR01(0x08), W(IM))                             \
        movpx_ld(W(RM), Mebp, inf_SCR01(0))

#define shlpx_ld(RG, RM, DP) /* loads SIMD, uses 1 elem at given address */ \
        movpx_st(W(RG), Mebp, inf_SCR01(0))                                 \
        stack_st(Recx)                                                      \
        movyx_ld(Recx,  W(RM), W(DP))                                       \
        shlyx_mx(Mebp,  inf_SCR01(0x00))                                    \
        shlyx_mx(Mebp,  inf_SCR01(0x08))                                    \
        stack_ld(Recx)                                                      \
        movpx_ld(W(RG), Mebp, inf_SCR01(0))

/* shr */

#define shrpx_ri(RM, IM)                                                    \
        movpx_st(W(RM), Mebp, inf_SCR01(0))                                 \
        shryx_mi(Mebp,  inf_SCR01(0x00), W(IM))                             \
        shryx_mi(Mebp,  inf_SCR01(0x08), W(IM))                             \
        movpx_ld(W(RM), Mebp, inf_SCR01(0))

#define shrpx_ld(RG, RM, DP) /* loads SIMD, uses 1 elem at given address */ \
        movpx_st(W(RG), Mebp, inf_SCR01(0))                                 \
        stack_st(Recx)                                                      \
        movyx_ld(Recx,  W(RM), W(DP))                                       \
        shryx_mx(Mebp,  inf_SCR01(0x00))                                    \
        shryx_mx(Mebp,  inf_SCR01(0x08))                                    \
        stack_ld(Recx)                                                      \
        movpx_ld(W(RG), Mebp, inf_SCR01(0))

#define shrpn_ri(RM, IM)                                                    \
        movpx_st(W(RM), Mebp, inf_SCR01(0))                                 \
        shryn_mi(Mebp,  inf_SCR01(0x00), W(IM))                             \
        shryn_mi(Mebp,  inf_SCR01(0x08), W(IM))                             \
        movpx_ld(W(RM), Mebp, inf_SCR01(0))

#define shrpn_ld(RG, RM, DP) /* loads SIMD, uses 1 elem at given address */ \
        movpx_st(W(RG), Mebp, inf_SCR01(0))                                 \
        stack_st(Recx)                                                      \
        movyx_ld(Recx,  W(RM), W(DP))                                       \
        shryn_mx(Mebp,  inf_SCR01(0x00))                                    \
        shryn_mx(Mebp,  inf_SCR01(0x08))                                    \
        stack_ld(Recx)                                                      \
        movpx_ld(W(RG), Mebp, inf_SCR01(0))

#else /* RT_128 >= 4 */

/* add */

#define addpx_rr(RG, RM)                                                    \
        EMITW(0x100000C0 | MXM(REG(RG), REG(RG), REG(RM)))

#define addpx_ld(RG, RM, DP)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x7C000699 | MXM(Tmm1,    Teax & (MOD(RM) == TPxx), TPxx))    \
        EMITW(0x100000C0 | MXM(REG(RG), REG(RG), Tmm1))/* ^ == -1 if true */

/* sub */

#define subpx_rr(RG, RM)                                                    \
        EMITW(0x100004C0 | MXM(REG(RG), REG(RG), REG(RM)))

#define subpx_ld(RG, RM, DP)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x7C000699 | MXM(Tmm1,    Teax & (MOD(RM) == TPxx), TPxx))    \
        EMITW(0x100004C0 | MXM(REG(RG), REG(RG), Tmm1))/* ^ == -1 if true */

/* shl */

#define shlpx_ri(RM, IM)                                                    \
        movyx_mi(Mebp, inf_SCR00, W(IM))                                    \
        shlpx_ld(W(RM), Mebp, inf_SCR00)

#define shlpx_ld(RG, RM, DP) /* loads SIMD, uses 1 elem at given address */ \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x7C000299 | MXM(Tmm1,    Teax & (MOD(RM) == TPxx), TPxx))    \
        EMITW(0x100005C4 | MXM(REG(RG), REG(RG), Tmm1))/* ^ == -1 if true */

/* shr */

#define shrpx_ri(RM, IM)                                                    \
        movyx_mi(Mebp, inf_SCR00, W(IM))                                    \
        shrpx_ld(W(RM), Mebp, inf_SCR00)

#define shrpx_ld(RG, RM, DP) /* loads SIMD, uses 1 elem at given address */ \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x7C000299 | MXM(Tmm1,    Teax & (MOD(RM) == TPxx), TPxx))    \
        EMITW(0x100006C4 | MXM(REG(RG), REG(RG), Tmm1))/* ^ == -1 if true */

#define shrpn_ri(RM, IM)                                                    \
        movyx_mi(Mebp, inf_SCR00, W(IM))                                    \
        shrpn_ld(W(RM), Mebp, inf_SCR00)

#define shrpn_ld(RG, RM, DP) /* loads SIMD, uses 1 elem at given address */ \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x7C000299 | MXM(Tmm1,    Teax & (MOD(RM) == TPxx), TPxx))    \
        EMITW(0x100003C4 | MXM(REG(RG), REG(RG), Tmm1))/* ^ == -1 if true */

#endif /* RT_128 >= 4 */

/**************************   helper macros (SIMD)   **************************/

/* simd mask
 * compatibility with AVX-512 and ARM-SVE can be achieved by always keeping
 * one hidden SIMD register holding all 1s and using one hidden mask register
 * first in cmp (c**ps) to produce compatible result in target SIMD register
 * then in CHECK_MASK to facilitate branching on a given condition value */

#define RT_SIMD_MASK_NONE       MN      /* none satisfy the condition */
#define RT_SIMD_MASK_FULL       MF      /*  all satisfy the condition */

#define S0(mask)    S1(mask)
#define S1(mask)    S##mask
#define SMN(rg, lb) ASM_BEG ASM_OP2(beq, cr6, lb) ASM_END
#define SMF(rg, lb) ASM_BEG ASM_OP2(blt, cr6, lb) ASM_END

#if (RT_128 < 4)

#define CHECK_MASK(lb, mask, RG) /* destroys Reax */                        \
        EMITW(0x10000486 | MXM(REG(RG), REG(RG), TmmQ))                     \
        AUW(EMPTY, EMPTY, EMPTY, EMPTY, lb, S0(RT_SIMD_MASK_##mask), EMPTY2)

#else /* RT_128 >= 4 */

#define CHECK_MASK(lb, mask, RG) /* destroys Reax */                        \
        EMITW(0x100004C7 | MXM(REG(RG), REG(RG), TmmQ))                     \
        AUW(EMPTY, EMPTY, EMPTY, EMPTY, lb, S0(RT_SIMD_MASK_##mask), EMPTY2)

#endif /* RT_128 >= 4 */

/* simd mode
 * set via FCTRL macros, *_F for faster non-IEEE mode (optional on MIPS/Power),
 * original FCTRL blocks (FCTRL_ENTER/FCTRL_LEAVE) are defined in rtbase.h
 * NOTE: ARMv7 always uses ROUNDN non-IEEE mode for SIMD fp-arithmetic,
 * while fp<->int conversion takes ROUND* into account via VFP fallback */

#if RT_SIMD_FLUSH_ZERO == 0

#define RT_SIMD_MODE_ROUNDN     0x00    /* round towards near */
#define RT_SIMD_MODE_ROUNDM     0x03    /* round towards -inf */
#define RT_SIMD_MODE_ROUNDP     0x02    /* round towards +inf */
#define RT_SIMD_MODE_ROUNDZ     0x01    /* round towards zero */

#else /* RT_SIMD_FLUSH_ZERO */

#define RT_SIMD_MODE_ROUNDN     0x04    /* round towards near */
#define RT_SIMD_MODE_ROUNDM     0x07    /* round towards -inf */
#define RT_SIMD_MODE_ROUNDP     0x06    /* round towards +inf */
#define RT_SIMD_MODE_ROUNDZ     0x05    /* round towards zero */

#endif /* RT_SIMD_FLUSH_ZERO */

#define RT_SIMD_MODE_ROUNDN_F   0x04    /* round towards near */
#define RT_SIMD_MODE_ROUNDM_F   0x07    /* round towards -inf */
#define RT_SIMD_MODE_ROUNDP_F   0x06    /* round towards +inf */
#define RT_SIMD_MODE_ROUNDZ_F   0x05    /* round towards zero */

#define fpscr_ld(RG) /* not portable, do not use outside */                 \
        EMITW(0xFE00058E | MRM(0x00,    REG(RG), 0x00))

#define fpscr_st(RG) /* not portable, do not use outside */                 \
        EMITW(0xFC00048E | MRM(REG(RG), 0x00,    0x00))

#define FCTRL_SET(mode)   /* sets given mode into fp control register */    \
        EMITW(0xFF80010C | RT_SIMD_MODE_##mode << 12)

#define FCTRL_RESET()     /* resumes default mode (ROUNDN) upon leave */    \
        EMITW(0xFF80010C)

/* cvt (fp-to-signed-int)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: ROUNDZ is not supported on pre-VSX Power systems, use cvz
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 64-bit signed int range */

#define rndps_rr(RG, RM)                                                    \
        EMITW(0xF00003AF | MXM(REG(RG), 0x00,    REG(RM)))

#define rndps_ld(RG, RM, DP)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x7C000699 | MXM(Tmm1,    Teax & (MOD(RM) == TPxx), TPxx))    \
        EMITW(0xF00003AF | MXM(REG(RG), 0x00,    Tmm1))/* ^ == -1 if true */

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
        EMITW(0xF00007E3 | MXM(REG(RG), 0x00,    REG(RM)))

#define cvtpn_ld(RG, RM, DP)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x7C000699 | MXM(Tmm1,    Teax & (MOD(RM) == TPxx), TPxx))    \
        EMITW(0xF00007E3 | MXM(REG(RG), 0x00,    Tmm1))/* ^ == -1 if true */

/* cvr (fp-to-signed-int)
 * rounding mode is encoded directly (cannot be used in FCTRL blocks)
 * NOTE: on targets with full-IEEE SIMD fp-arithmetic the ROUND*_F mode
 * isn't always taken into account when used within full-IEEE ASM block
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 64-bit signed int range */

#define rnrps_rr(RG, RM, mode)                                              \
        FCTRL_ENTER(mode)                                                   \
        rndps_rr(W(RG), W(RM))                                              \
        FCTRL_LEAVE(mode)

#define cvrps_rr(RG, RM, mode)                                              \
        rnrps_rr(W(RG), W(RM), mode)                                        \
        cvzps_rr(W(RG), W(RG))

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
        movpx_st(XmmF, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH*8))                                 \
        EMITW(0x7C000799 | MXM(TmmR,    0x00,    Teax))                     \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH*8))                                 \
        EMITW(0x7C000799 | MXM(TmmS,    0x00,    Teax))                     \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH*8))                                 \
        EMITW(0x7C000799 | MXM(TmmQ,    0x00,    Teax))                     \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH*8))                                 \
        EMITW(0x7C000799 | MXM(TmmA,    0x00,    Teax))                     \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH*8))                                 \
        EMITW(0x7C000799 | MXM(TmmB,    0x00,    Teax))                     \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH*8))                                 \
        EMITW(0x7C000799 | MXM(TmmC,    0x00,    Teax))                     \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH*8))                                 \
        EMITW(0x7C000799 | MXM(TmmD,    0x00,    Teax))                     \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH*8))                                 \
        EMITW(0x7C000799 | MXM(TmmE,    0x00,    Teax))                     \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH*8))                                 \
        EMITW(0x7C000799 | MXM(Tmm1,    0x00,    Teax))

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
        movpx_ld(XmmF, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH*8))                                 \
        EMITW(0x7C000699 | MXM(TmmR,    0x00,    Teax))                     \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH*8))                                 \
        EMITW(0x7C000699 | MXM(TmmS,    0x00,    Teax))                     \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH*8))                                 \
        EMITW(0x7C000699 | MXM(TmmQ,    0x00,    Teax))                     \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH*8))                                 \
        EMITW(0x7C000699 | MXM(TmmA,    0x00,    Teax))                     \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH*8))                                 \
        EMITW(0x7C000699 | MXM(TmmB,    0x00,    Teax))                     \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH*8))                                 \
        EMITW(0x7C000699 | MXM(TmmC,    0x00,    Teax))                     \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH*8))                                 \
        EMITW(0x7C000699 | MXM(TmmD,    0x00,    Teax))                     \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH*8))                                 \
        EMITW(0x7C000699 | MXM(TmmE,    0x00,    Teax))                     \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH*8))                                 \
        EMITW(0x7C000699 | MXM(Tmm1,    0x00,    Teax))

#endif /* RT_128 >= 2 */

#endif /* RT_SIMD_CODE */

#endif /* RT_RTARCH_P64_128_H */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
