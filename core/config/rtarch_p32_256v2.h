/******************************************************************************/
/* Copyright (c) 2013-2016 VectorChief (at github, bitbucket, sourceforge)    */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#ifndef RT_RTARCH_P32_256V2_H
#define RT_RTARCH_P32_256V2_H

#include "rtarch_p64_128v4.h"

#define RT_SIMD_REGS_256        16
#define RT_SIMD_ALIGN_256       32
#define RT_SIMD_WIDTH64_256     4
#define RT_SIMD_SET64_256(s, v) s[0]=s[1]=s[2]=s[3]=v
#define RT_SIMD_WIDTH32_256     8
#define RT_SIMD_SET32_256(s, v) s[0]=s[1]=s[2]=s[3]=s[4]=s[5]=s[6]=s[7]=v

/******************************************************************************/
/*********************************   LEGEND   *********************************/
/******************************************************************************/

/*
 * rtarch_p32_256v2.h: Implementation of Power fp32 VSX1/2 instructions (pairs).
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
 * cmdo*_** - applies [cmd] to 32-bit SIMD register/memory/immediate args
 * cmdp*_** - applies [cmd] to L-size SIMD register/memory/immediate args
 * cmdq*_** - applies [cmd] to 64-bit SIMD register/memory/immediate args
 *
 * The cmdp*_** (rtbase.h) instructions are intended for SPMD programming model
 * and can be configured to work with 32/64-bit data-elements (int, fp).
 * In this model data-paths are fixed-width, BASE and SIMD data-elements are
 * width-compatible, code-path divergence is handled via CHECK_MASK macro.
 * Matching element-sized BASE subset cmdy*_** is defined in rtbase.h.
 *
 * Interpretation of instruction parameters:
 *
 * upper-case params have triplet structure and require W to pass-forward
 * lower-case params are singular and can be used/passed as such directly
 *
 * XD - SIMD register serving as destination only, if present
 * XG - SIMD register serving as destination and fisrt source
 * XS - SIMD register serving as second source (first if any)
 * XT - SIMD register serving as third source (second if any)
 *
 * RD - BASE register serving as destination only, if present
 * RG - BASE register serving as destination and fisrt source
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

#if defined (RT_SIMD_CODE)

#if defined (RT_256) && (RT_256 != 0) && (RT_SIMD_COMPAT_XMM > 0)

#undef  sregs_sa
#undef  sregs_la

/******************************************************************************/
/********************************   EXTERNAL   ********************************/
/******************************************************************************/

/******************************************************************************/
/**********************************   VSX   ***********************************/
/******************************************************************************/

/**************************   packed generic (SIMD)   *************************/

/* mov (D = S) */

#define movcx_rr(XD, XS)                                                    \
        EMITW(0xF0000497 | MXM(REG(XD), REG(XS), REG(XS)))                  \
        EMITW(0xF0000497 | MXM(RYG(XD), RYG(XS), RYG(XS)))

#define movcx_ld(XD, MS, DS)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000619 | MXM(REG(XD), Teax & (MOD(MS) == TPxx), TPxx))    \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VYL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VYL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000619 | MXM(RYG(XD), Teax & (MOD(MS) == TPxx), TPxx))    \
                                                       /* ^ == -1 if true */

#define movcx_st(XS, MD, DD)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MD), VAL(DD), C2(DD), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MD), VAL(DD), B2(DD), P2(DD)))  \
        EMITW(0x7C000719 | MXM(REG(XS), Teax & (MOD(MD) == TPxx), TPxx))    \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MD), VYL(DD), C2(DD), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MD), VYL(DD), B2(DD), P2(DD)))  \
        EMITW(0x7C000719 | MXM(RYG(XS), Teax & (MOD(MD) == TPxx), TPxx))    \
                                                       /* ^ == -1 if true */

/* mmv (G = G mask-merge S, mask: 0 - keeps G, 1 - picks S with elem-size frag)
 * uses Xmm0 implicitly as a mask register, destroys Xmm0, XS unmasked frags */

#define mmvcx_rr(XG, XS)                                                    \
        EMITW(0xF000003F | MXM(REG(XG), REG(XG), REG(XS)))                  \
        EMITW(0xF000043F | MXM(RYG(XG), RYG(XG), RYG(XS)))

#define mmvcx_ld(XG, MS, DS)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000619 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0xF000003F | MXM(REG(XG), REG(XG), TmmM))/* ^ == -1 if true */\
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VYL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VYL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000619 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0xF000043F | MXM(RYG(XG), RYG(XG), TmmM))/* ^ == -1 if true */

#define mmvcx_st(XS, MG, DG)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MG), VAL(DG), C2(DG), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MG), VAL(DG), B2(DG), P2(DG)))  \
        EMITW(0x7C000619 | MXM(TmmM,    Teax & (MOD(MG) == TPxx), TPxx))    \
        EMITW(0xF000003F | MXM(TmmM,    TmmM,    REG(XS)))                  \
        EMITW(0x7C000719 | MXM(TmmM,    Teax & (MOD(MG) == TPxx), TPxx))    \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MG), VYL(DG), C2(DG), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MG), VYL(DG), B2(DG), P2(DG)))  \
        EMITW(0x7C000619 | MXM(TmmM,    Teax & (MOD(MG) == TPxx), TPxx))    \
        EMITW(0xF000043F | MXM(TmmM,    TmmM,    RYG(XS)))                  \
        EMITW(0x7C000719 | MXM(TmmM,    Teax & (MOD(MG) == TPxx), TPxx))    \
                                                       /* ^ == -1 if true */

/* and (G = G & S) */

#define andcx_rr(XG, XS)                                                    \
        EMITW(0xF0000417 | MXM(REG(XG), REG(XG), REG(XS)))                  \
        EMITW(0xF0000417 | MXM(RYG(XG), RYG(XG), RYG(XS)))

#define andcx_ld(XG, MS, DS)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000619 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0xF0000417 | MXM(REG(XG), REG(XG), TmmM))/* ^ == -1 if true */\
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VYL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VYL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000619 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0xF0000417 | MXM(RYG(XG), RYG(XG), TmmM))/* ^ == -1 if true */

/* ann (G = ~G & S) */

#define anncx_rr(XG, XS)                                                    \
        EMITW(0xF0000457 | MXM(REG(XG), REG(XS), REG(XG)))                  \
        EMITW(0xF0000457 | MXM(RYG(XG), RYG(XS), RYG(XG)))

#define anncx_ld(XG, MS, DS)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000619 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0xF0000457 | MXM(REG(XG), TmmM,    REG(XG)))                  \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VYL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VYL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000619 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0xF0000457 | MXM(RYG(XG), TmmM,    RYG(XG)))

/* orr (G = G | S) */

#define orrcx_rr(XG, XS)                                                    \
        EMITW(0xF0000497 | MXM(REG(XG), REG(XG), REG(XS)))                  \
        EMITW(0xF0000497 | MXM(RYG(XG), RYG(XG), RYG(XS)))

#define orrcx_ld(XG, MS, DS)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000619 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0xF0000497 | MXM(REG(XG), REG(XG), TmmM))/* ^ == -1 if true */\
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VYL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VYL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000619 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0xF0000497 | MXM(RYG(XG), RYG(XG), TmmM))/* ^ == -1 if true */

/* orn (G = ~G | S) */

#if (RT_256 < 2)

#define orncx_rr(XG, XS)                                                    \
        notcx_rx(W(XG))                                                     \
        orrcx_rr(W(XG), W(XS))

#define orncx_ld(XG, MS, DS)                                                \
        notcx_rx(W(XG))                                                     \
        orrcx_ld(W(XG), W(MS), W(DS))

#else /* RT_256 >= 2 */

#define orncx_rr(XG, XS)                                                    \
        EMITW(0xF0000557 | MXM(REG(XG), REG(XS), REG(XG)))                  \
        EMITW(0xF0000557 | MXM(RYG(XG), RYG(XS), RYG(XG)))

#define orncx_ld(XG, MS, DS)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000619 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0xF0000557 | MXM(REG(XG), TmmM,    REG(XG)))                  \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VYL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VYL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000619 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0xF0000557 | MXM(RYG(XG), TmmM,    RYG(XG)))

#endif /* RT_256 >= 2 */

/* xor (G = G ^ S) */

#define xorcx_rr(XG, XS)                                                    \
        EMITW(0xF00004D7 | MXM(REG(XG), REG(XG), REG(XS)))                  \
        EMITW(0xF00004D7 | MXM(RYG(XG), RYG(XG), RYG(XS)))

#define xorcx_ld(XG, MS, DS)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000619 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0xF00004D7 | MXM(REG(XG), REG(XG), TmmM))/* ^ == -1 if true */\
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VYL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VYL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000619 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0xF00004D7 | MXM(RYG(XG), RYG(XG), TmmM))/* ^ == -1 if true */

/* not (G = ~G) */

#define notcx_rx(XG)                                                        \
        EMITW(0xF0000517 | MXM(REG(XG), REG(XG), REG(XG)))                  \
        EMITW(0xF0000517 | MXM(RYG(XG), RYG(XG), RYG(XG)))

/**************   packed single precision floating point (SIMD)   *************/

/* neg (G = -G) */

#define negcs_rx(XG)                                                        \
        EMITW(0xF00006E7 | MXM(REG(XG), 0x00,    REG(XG)))                  \
        EMITW(0xF00006E7 | MXM(RYG(XG), 0x00,    RYG(XG)))

/* add (G = G + S) */

#define addcs_rr(XG, XS)                                                    \
        EMITW(0xF0000207 | MXM(REG(XG), REG(XG), REG(XS)))                  \
        EMITW(0xF0000207 | MXM(RYG(XG), RYG(XG), RYG(XS)))

#define addcs_ld(XG, MS, DS)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000619 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0xF0000207 | MXM(REG(XG), REG(XG), TmmM))/* ^ == -1 if true */\
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VYL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VYL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000619 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0xF0000207 | MXM(RYG(XG), RYG(XG), TmmM))/* ^ == -1 if true */

/* sub (G = G - S) */

#define subcs_rr(XG, XS)                                                    \
        EMITW(0xF0000247 | MXM(REG(XG), REG(XG), REG(XS)))                  \
        EMITW(0xF0000247 | MXM(RYG(XG), RYG(XG), RYG(XS)))

#define subcs_ld(XG, MS, DS)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000619 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0xF0000247 | MXM(REG(XG), REG(XG), TmmM))/* ^ == -1 if true */\
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VYL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VYL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000619 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0xF0000247 | MXM(RYG(XG), RYG(XG), TmmM))/* ^ == -1 if true */

/* mul (G = G * S) */

#define mulcs_rr(XG, XS)                                                    \
        EMITW(0xF0000287 | MXM(REG(XG), REG(XG), REG(XS)))                  \
        EMITW(0xF0000287 | MXM(RYG(XG), RYG(XG), RYG(XS)))

#define mulcs_ld(XG, MS, DS)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000619 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0xF0000287 | MXM(REG(XG), REG(XG), TmmM))/* ^ == -1 if true */\
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VYL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VYL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000619 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0xF0000287 | MXM(RYG(XG), RYG(XG), TmmM))/* ^ == -1 if true */

/* div (G = G / S) */

#define divcs_rr(XG, XS)                                                    \
        EMITW(0xF00002C7 | MXM(REG(XG), REG(XG), REG(XS)))                  \
        EMITW(0xF00002C7 | MXM(RYG(XG), RYG(XG), RYG(XS)))

#define divcs_ld(XG, MS, DS)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000619 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0xF00002C7 | MXM(REG(XG), REG(XG), TmmM))/* ^ == -1 if true */\
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VYL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VYL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000619 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0xF00002C7 | MXM(RYG(XG), RYG(XG), TmmM))/* ^ == -1 if true */

/* sqr (D = sqrt S) */

#define sqrcs_rr(XD, XS)                                                    \
        EMITW(0xF000022F | MXM(REG(XD), 0x00,    REG(XS)))                  \
        EMITW(0xF000022F | MXM(RYG(XD), 0x00,    RYG(XS)))

#define sqrcs_ld(XD, MS, DS)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000619 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0xF000022F | MXM(REG(XD), 0x00,    TmmM))/* ^ == -1 if true */\
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VYL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VYL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000619 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0xF000022F | MXM(RYG(XD), 0x00,    TmmM))/* ^ == -1 if true */

/* cbr (D = cbrt S) */

        /* cbe, cbs, cbr defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* rcp (D = 1.0 / S)
 * accuracy/behavior may vary across supported targets, use accordingly */

#if RT_SIMD_COMPAT_RCP != 1

#define rcecs_rr(XD, XS)                                                    \
        EMITW(0xF000026B | MXM(REG(XD), 0x00,    REG(XS)))                  \
        EMITW(0xF000026B | MXM(RYG(XD), 0x00,    RYG(XS)))

#define rcscs_rr(XG, XS) /* destroys XS */                                  \
        EMITW(0xF00006CD | MXM(REG(XS), REG(XG), TmmQ))                     \
        EMITW(0xF000020F | MXM(REG(XG), REG(XG), REG(XS)))                  \
        EMITW(0xF00006CD | MXM(RYG(XS), RYG(XG), TmmQ))                     \
        EMITW(0xF000020F | MXM(RYG(XG), RYG(XG), RYG(XS)))

#endif /* RT_SIMD_COMPAT_RCP */

        /* rcp defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* rsq (D = 1.0 / sqrt S)
 * accuracy/behavior may vary across supported targets, use accordingly */

#if RT_SIMD_COMPAT_RSQ != 1

#define rsecs_rr(XD, XS)                                                    \
        EMITW(0xF000022B | MXM(REG(XD), 0x00,    REG(XS)))                  \
        EMITW(0xF000022B | MXM(RYG(XD), 0x00,    RYG(XS)))

#define rsscs_rr(XG, XS) /* destroys XS */                                  \
        EMITW(0xF0000287 | MXM(TmmM,    REG(XG), REG(XG)))                  \
        EMITW(0xF0000285 | MXM(TmmQ,    REG(XG), TmmM))                     \
        EMITW(0xF00006CD | MXM(TmmM,    REG(XS), TmmQ))                     \
        EMITW(0xF000068F | MXM(REG(XG), TmmM,    TmmQ))                     \
        EMITW(0xF0000287 | MXM(TmmM,    RYG(XG), RYG(XG)))                  \
        EMITW(0xF0000285 | MXM(TmmQ,    RYG(XG), TmmM))                     \
        EMITW(0xF00006CD | MXM(TmmM,    RYG(XS), TmmQ))                     \
        EMITW(0xF000068F | MXM(RYG(XG), TmmM,    TmmQ))                     \
        EMITW(0x1000038C | MXM(TmmQ,    0x1F,    0x00))

#endif /* RT_SIMD_COMPAT_RSQ */

        /* rsq defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* fma (G = G + S * T)
 * NOTE: x87 fpu-fallbacks for fma/fms use round-to-nearest mode by default,
 * enable RT_SIMD_COMPAT_FMR for current SIMD rounding mode to be honoured */

#if RT_SIMD_COMPAT_FMA <= 1

#define fmacs_rr(XG, XS, XT)                                                \
        EMITW(0xF000020F | MXM(REG(XG), REG(XS), REG(XT)))                  \
        EMITW(0xF000020F | MXM(RYG(XG), RYG(XS), RYG(XT)))

#define fmacs_ld(XG, XS, MT, DT)                                            \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C000619 | MXM(TmmM,    Teax & (MOD(MT) == TPxx), TPxx))    \
        EMITW(0xF000020F | MXM(REG(XG), REG(XS), TmmM))                     \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MT), VYL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MT), VYL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C000619 | MXM(TmmM,    Teax & (MOD(MT) == TPxx), TPxx))    \
        EMITW(0xF000020F | MXM(RYG(XG), RYG(XS), TmmM))

#endif /* RT_SIMD_COMPAT_FMA */

/* fms (G = G - S * T)
 * NOTE: due to final negation being outside of rounding on all Power systems
 * only symmetric rounding modes (RN, RZ) are compatible across all targets */

#if RT_SIMD_COMPAT_FMS <= 1

#define fmscs_rr(XG, XS, XT)                                                \
        EMITW(0xF000068F | MXM(REG(XG), REG(XS), REG(XT)))                  \
        EMITW(0xF000068F | MXM(RYG(XG), RYG(XS), RYG(XT)))

#define fmscs_ld(XG, XS, MT, DT)                                            \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C000619 | MXM(TmmM,    Teax & (MOD(MT) == TPxx), TPxx))    \
        EMITW(0xF000068F | MXM(REG(XG), REG(XS), TmmM))                     \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MT), VYL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MT), VYL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C000619 | MXM(TmmM,    Teax & (MOD(MT) == TPxx), TPxx))    \
        EMITW(0xF000068F | MXM(RYG(XG), RYG(XS), TmmM))

#endif /* RT_SIMD_COMPAT_FMS */

/* min (G = G < S ? G : S) */

#define mincs_rr(XG, XS)                                                    \
        EMITW(0xF0000647 | MXM(REG(XG), REG(XG), REG(XS)))                  \
        EMITW(0xF0000647 | MXM(RYG(XG), RYG(XG), RYG(XS)))

#define mincs_ld(XG, MS, DS)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000619 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0xF0000647 | MXM(REG(XG), REG(XG), TmmM))/* ^ == -1 if true */\
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VYL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VYL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000619 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0xF0000647 | MXM(RYG(XG), RYG(XG), TmmM))/* ^ == -1 if true */

/* max (G = G > S ? G : S) */

#define maxcs_rr(XG, XS)                                                    \
        EMITW(0xF0000607 | MXM(REG(XG), REG(XG), REG(XS)))                  \
        EMITW(0xF0000607 | MXM(RYG(XG), RYG(XG), RYG(XS)))

#define maxcs_ld(XG, MS, DS)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000619 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0xF0000607 | MXM(REG(XG), REG(XG), TmmM))/* ^ == -1 if true */\
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VYL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VYL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000619 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0xF0000607 | MXM(RYG(XG), RYG(XG), TmmM))/* ^ == -1 if true */

/* cmp (G = G ? S) */

#define ceqcs_rr(XG, XS)                                                    \
        EMITW(0xF000021F | MXM(REG(XG), REG(XG), REG(XS)))                  \
        EMITW(0xF000021F | MXM(RYG(XG), RYG(XG), RYG(XS)))

#define ceqcs_ld(XG, MS, DS)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000619 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0xF000021F | MXM(REG(XG), REG(XG), TmmM))/* ^ == -1 if true */\
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VYL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VYL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000619 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0xF000021F | MXM(RYG(XG), RYG(XG), TmmM))/* ^ == -1 if true */

#define cnecs_rr(XG, XS)                                                    \
        EMITW(0xF000021F | MXM(REG(XG), REG(XG), REG(XS)))                  \
        EMITW(0xF0000517 | MXM(REG(XG), REG(XG), REG(XG)))                  \
        EMITW(0xF000021F | MXM(RYG(XG), RYG(XG), RYG(XS)))                  \
        EMITW(0xF0000517 | MXM(RYG(XG), RYG(XG), RYG(XG)))

#define cnecs_ld(XG, MS, DS)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000619 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0xF000021F | MXM(REG(XG), REG(XG), TmmM))/* ^ == -1 if true */\
        EMITW(0xF0000517 | MXM(REG(XG), REG(XG), REG(XG)))                  \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VYL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VYL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000619 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0xF000021F | MXM(RYG(XG), RYG(XG), TmmM))/* ^ == -1 if true */\
        EMITW(0xF0000517 | MXM(RYG(XG), RYG(XG), RYG(XG)))

#define cltcs_rr(XG, XS)                                                    \
        EMITW(0xF000025F | MXM(REG(XG), REG(XS), REG(XG)))                  \
        EMITW(0xF000025F | MXM(RYG(XG), RYG(XS), RYG(XG)))

#define cltcs_ld(XG, MS, DS)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000619 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0xF000025F | MXM(REG(XG), TmmM,    REG(XG)))                  \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VYL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VYL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000619 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0xF000025F | MXM(RYG(XG), TmmM,    RYG(XG)))

#define clecs_rr(XG, XS)                                                    \
        EMITW(0xF000029F | MXM(REG(XG), REG(XS), REG(XG)))                  \
        EMITW(0xF000029F | MXM(RYG(XG), RYG(XS), RYG(XG)))

#define clecs_ld(XG, MS, DS)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000619 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0xF000029F | MXM(REG(XG), TmmM,    REG(XG)))                  \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VYL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VYL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000619 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0xF000029F | MXM(RYG(XG), TmmM,    RYG(XG)))

#define cgtcs_rr(XG, XS)                                                    \
        EMITW(0xF000025F | MXM(REG(XG), REG(XG), REG(XS)))                  \
        EMITW(0xF000025F | MXM(RYG(XG), RYG(XG), RYG(XS)))

#define cgtcs_ld(XG, MS, DS)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000619 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0xF000025F | MXM(REG(XG), REG(XG), TmmM))/* ^ == -1 if true */\
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VYL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VYL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000619 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0xF000025F | MXM(RYG(XG), RYG(XG), TmmM))/* ^ == -1 if true */

#define cgecs_rr(XG, XS)                                                    \
        EMITW(0xF000029F | MXM(REG(XG), REG(XG), REG(XS)))                  \
        EMITW(0xF000029F | MXM(RYG(XG), RYG(XG), RYG(XS)))

#define cgecs_ld(XG, MS, DS)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000619 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0xF000029F | MXM(REG(XG), REG(XG), TmmM))/* ^ == -1 if true */\
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VYL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VYL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000619 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0xF000029F | MXM(RYG(XG), RYG(XG), TmmM))/* ^ == -1 if true */

/* cvz (D = fp-to-signed-int S)
 * rounding mode is encoded directly (can be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnzcs_rr(XD, XS)     /* round towards zero */                       \
        EMITW(0xF0000267 | MXM(REG(XD), 0x00,    REG(XS)))                  \
        EMITW(0xF0000267 | MXM(RYG(XD), 0x00,    RYG(XS)))

#define rnzcs_ld(XD, MS, DS) /* round towards zero */                       \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000619 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0xF0000267 | MXM(REG(XD), 0x00,    TmmM))/* ^ == -1 if true */\
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VYL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VYL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000619 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0xF0000267 | MXM(RYG(XD), 0x00,    TmmM))/* ^ == -1 if true */

#define cvzcs_rr(XD, XS)     /* round towards zero */                       \
        EMITW(0xF0000263 | MXM(REG(XD), 0x00,    REG(XS)))                  \
        EMITW(0xF0000263 | MXM(RYG(XD), 0x00,    RYG(XS)))

#define cvzcs_ld(XD, MS, DS) /* round towards zero */                       \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000619 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0xF0000263 | MXM(REG(XD), 0x00,    TmmM))/* ^ == -1 if true */\
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VYL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VYL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000619 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0xF0000263 | MXM(RYG(XD), 0x00,    TmmM))/* ^ == -1 if true */

/* cvp (D = fp-to-signed-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnpcs_rr(XD, XS)     /* round towards +inf */                       \
        EMITW(0xF00002A7 | MXM(REG(XD), 0x00,    REG(XS)))                  \
        EMITW(0xF00002A7 | MXM(RYG(XD), 0x00,    RYG(XS)))

#define rnpcs_ld(XD, MS, DS) /* round towards +inf */                       \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000619 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0xF00002A7 | MXM(REG(XD), 0x00,    TmmM))/* ^ == -1 if true */\
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VYL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VYL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000619 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0xF00002A7 | MXM(RYG(XD), 0x00,    TmmM))/* ^ == -1 if true */

#define cvpcs_rr(XD, XS)     /* round towards +inf */                       \
        rnpcs_rr(W(XD), W(XS))                                              \
        cvzcs_rr(W(XD), W(XD))

#define cvpcs_ld(XD, MS, DS) /* round towards +inf */                       \
        rnpcs_ld(W(XD), W(MS), W(DS))                                       \
        cvzcs_rr(W(XD), W(XD))

/* cvm (D = fp-to-signed-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnmcs_rr(XD, XS)     /* round towards -inf */                       \
        EMITW(0xF00002E7 | MXM(REG(XD), 0x00,    REG(XS)))                  \
        EMITW(0xF00002E7 | MXM(RYG(XD), 0x00,    RYG(XS)))

#define rnmcs_ld(XD, MS, DS) /* round towards -inf */                       \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000619 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0xF00002E7 | MXM(REG(XD), 0x00,    TmmM))/* ^ == -1 if true */\
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VYL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VYL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000619 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0xF00002E7 | MXM(RYG(XD), 0x00,    TmmM))/* ^ == -1 if true */

#define cvmcs_rr(XD, XS)     /* round towards -inf */                       \
        rnmcs_rr(W(XD), W(XS))                                              \
        cvzcs_rr(W(XD), W(XD))

#define cvmcs_ld(XD, MS, DS) /* round towards -inf */                       \
        rnmcs_ld(W(XD), W(MS), W(DS))                                       \
        cvzcs_rr(W(XD), W(XD))

/* cvn (D = fp-to-signed-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnncs_rr(XD, XS)     /* round towards near */                       \
        EMITW(0xF00002AF | MXM(REG(XD), 0x00,    REG(XS)))                  \
        EMITW(0xF00002AF | MXM(RYG(XD), 0x00,    RYG(XS)))

#define rnncs_ld(XD, MS, DS) /* round towards near */                       \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000619 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0xF00002AF | MXM(REG(XD), 0x00,    TmmM))/* ^ == -1 if true */\
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VYL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VYL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000619 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0xF00002AF | MXM(RYG(XD), 0x00,    TmmM))/* ^ == -1 if true */

#define cvncs_rr(XD, XS)     /* round towards near */                       \
        rnncs_rr(W(XD), W(XS))                                              \
        cvzcs_rr(W(XD), W(XD))

#define cvncs_ld(XD, MS, DS) /* round towards near */                       \
        rnncs_ld(W(XD), W(MS), W(DS))                                       \
        cvzcs_rr(W(XD), W(XD))

/* cvn (D = signed-int-to-fp S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks) */

#define cvncn_rr(XD, XS)     /* round towards near */                       \
        cvtcn_rr(W(XD), W(XS))

#define cvncn_ld(XD, MS, DS) /* round towards near */                       \
        cvtcn_ld(W(XD), W(MS), W(DS))

/**************************   packed integer (SIMD)   *************************/

/* add (G = G + S) */

#define addcx_rr(XG, XS)                                                    \
        EMITW(0x10000080 | MXM(REG(XG), REG(XG), REG(XS)))                  \
        EMITW(0x10000080 | MXM(RYG(XG), RYG(XG), RYG(XS)))

#define addcx_ld(XG, MS, DS)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000619 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0x10000080 | MXM(REG(XG), REG(XG), TmmM))/* ^ == -1 if true */\
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VYL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VYL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000619 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0x10000080 | MXM(RYG(XG), RYG(XG), TmmM))/* ^ == -1 if true */

/* sub (G = G - S) */

#define subcx_rr(XG, XS)                                                    \
        EMITW(0x10000480 | MXM(REG(XG), REG(XG), REG(XS)))                  \
        EMITW(0x10000480 | MXM(RYG(XG), RYG(XG), RYG(XS)))

#define subcx_ld(XG, MS, DS)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000619 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0x10000480 | MXM(REG(XG), REG(XG), TmmM))/* ^ == -1 if true */\
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VYL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VYL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000619 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0x10000480 | MXM(RYG(XG), RYG(XG), TmmM))/* ^ == -1 if true */

/* shl (G = G << S)
 * for maximum compatibility, shift count mustn't exceed elem-size */

#define shlcx_ri(XG, IS)                                                    \
        EMITW(0x1000038C | MXM(TmmM,    (0x1F & VAL(IS)), 0x00))            \
        EMITW(0x10000184 | MXM(REG(XG), REG(XG), TmmM))                     \
        EMITW(0x10000184 | MXM(RYG(XG), RYG(XG), TmmM))

#if RT_ENDIAN == 0

#define shlcx_ld(XG, MS, DS) /* loads SIMD, uses 64-bit at given address */ \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C00008E | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0x1003028C | MXM(TmmM,    0x00,    TmmM))/* ^ == -1 if true */\
        EMITW(0x10000184 | MXM(REG(XG), REG(XG), TmmM))                     \
        EMITW(0x10000184 | MXM(RYG(XG), RYG(XG), TmmM))

#else /* RT_ENDIAN == 1 */

#define shlcx_ld(XG, MS, DS) /* loads SIMD, uses 64-bit at given address */ \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C00008E | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0x1000028C | MXM(TmmM,    0x00,    TmmM))/* ^ == -1 if true */\
        EMITW(0x10000184 | MXM(REG(XG), REG(XG), TmmM))                     \
        EMITW(0x10000184 | MXM(RYG(XG), RYG(XG), TmmM))

#endif /* RT_ENDIAN == 1 */

#define svlcx_rr(XG, XS)     /* variable shift with per-elem count */       \
        EMITW(0x10000184 | MXM(REG(XG), REG(XG), REG(XS)))                  \
        EMITW(0x10000184 | MXM(RYG(XG), RYG(XG), RYG(XS)))

#define svlcx_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000619 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0x10000184 | MXM(REG(XG), REG(XG), TmmM))                     \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VYL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VYL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000619 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0x10000184 | MXM(RYG(XG), RYG(XG), TmmM))

/* shr (G = G >> S)
 * for maximum compatibility, shift count mustn't exceed elem-size */

#define shrcx_ri(XG, IS)                                                    \
        EMITW(0x1000038C | MXM(TmmM,    (0x1F & VAL(IS)), 0x00))            \
        EMITW(0x10000284 | MXM(REG(XG), REG(XG), TmmM))                     \
        EMITW(0x10000284 | MXM(RYG(XG), RYG(XG), TmmM))

#if RT_ENDIAN == 0

#define shrcx_ld(XG, MS, DS) /* loads SIMD, uses 64-bit at given address */ \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C00008E | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0x1003028C | MXM(TmmM,    0x00,    TmmM))/* ^ == -1 if true */\
        EMITW(0x10000284 | MXM(REG(XG), REG(XG), TmmM))                     \
        EMITW(0x10000284 | MXM(RYG(XG), RYG(XG), TmmM))

#else /* RT_ENDIAN == 1 */

#define shrcx_ld(XG, MS, DS) /* loads SIMD, uses 64-bit at given address */ \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C00008E | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0x1000028C | MXM(TmmM,    0x00,    TmmM))/* ^ == -1 if true */\
        EMITW(0x10000284 | MXM(REG(XG), REG(XG), TmmM))                     \
        EMITW(0x10000284 | MXM(RYG(XG), RYG(XG), TmmM))

#endif /* RT_ENDIAN == 1 */

#define svrcx_rr(XG, XS)     /* variable shift with per-elem count */       \
        EMITW(0x10000284 | MXM(REG(XG), REG(XG), REG(XS)))                  \
        EMITW(0x10000284 | MXM(RYG(XG), RYG(XG), RYG(XS)))

#define svrcx_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000619 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0x10000284 | MXM(REG(XG), REG(XG), TmmM))                     \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VYL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VYL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000619 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0x10000284 | MXM(RYG(XG), RYG(XG), TmmM))


#define shrcn_ri(XG, IS)                                                    \
        EMITW(0x1000038C | MXM(TmmM,    (0x1F & VAL(IS)), 0x00))            \
        EMITW(0x10000384 | MXM(REG(XG), REG(XG), TmmM))                     \
        EMITW(0x10000384 | MXM(RYG(XG), RYG(XG), TmmM))

#if RT_ENDIAN == 0

#define shrcn_ld(XG, MS, DS) /* loads SIMD, uses 64-bit at given address */ \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C00008E | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0x1003028C | MXM(TmmM,    0x00,    TmmM))/* ^ == -1 if true */\
        EMITW(0x10000384 | MXM(REG(XG), REG(XG), TmmM))                     \
        EMITW(0x10000384 | MXM(RYG(XG), RYG(XG), TmmM))

#else /* RT_ENDIAN == 1 */

#define shrcn_ld(XG, MS, DS) /* loads SIMD, uses 64-bit at given address */ \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C00008E | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0x1000028C | MXM(TmmM,    0x00,    TmmM))/* ^ == -1 if true */\
        EMITW(0x10000384 | MXM(REG(XG), REG(XG), TmmM))                     \
        EMITW(0x10000384 | MXM(RYG(XG), RYG(XG), TmmM))

#endif /* RT_ENDIAN == 1 */

#define svrcn_rr(XG, XS)     /* variable shift with per-elem count */       \
        EMITW(0x10000384 | MXM(REG(XG), REG(XG), REG(XS)))                  \
        EMITW(0x10000384 | MXM(RYG(XG), RYG(XG), RYG(XS)))

#define svrcn_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000619 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0x10000384 | MXM(REG(XG), REG(XG), TmmM))                     \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VYL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VYL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000619 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0x10000384 | MXM(RYG(XG), RYG(XG), TmmM))

/**************************   helper macros (SIMD)   **************************/

/* simd mask
 * compatibility with AVX-512 and ARM-SVE can be achieved by always keeping
 * one hidden SIMD register holding all 1s and using one hidden mask register
 * first in cmp (c**ps) to produce compatible result in target SIMD register
 * then in mkj**_** to facilitate branching on a given condition value */

#define RT_SIMD_MASK_NONE32_256  MN32_256   /* none satisfy the condition */
#define RT_SIMD_MASK_FULL32_256  MF32_256   /*  all satisfy the condition */

/* #define S0(mask)    S1(mask)            (defined in 32_128-bit header) */
/* #define S1(mask)    S##mask             (defined in 32_128-bit header) */

#define SMN32_256(xs, lb) /* not portable, do not use outside */            \
        EMITW(0xF0000497 | MXM(TmmM, xs,  xs+16))                           \
        EMITW(0x10000486 | MXM(TmmM, TmmM, TmmQ))                           \
        ASM_BEG ASM_OP2(beq, cr6, lb) ASM_END

#define SMF32_256(xs, lb) /* not portable, do not use outside */            \
        EMITW(0xF0000417 | MXM(TmmM, xs,  xs+16))                           \
        EMITW(0x10000486 | MXM(TmmM, TmmM, TmmQ))                           \
        ASM_BEG ASM_OP2(blt, cr6, lb) ASM_END

#define mkjcx_rx(XS, mask, lb)   /* destroys Reax, if S == mask jump lb */  \
        AUW(EMPTY, EMPTY, EMPTY, REG(XS), lb,                               \
        S0(RT_SIMD_MASK_##mask##32_256), EMPTY2)

/* cvt (D = fp-to-signed-int S)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: ROUNDZ is not supported on pre-VSX Power systems, use cvz
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rndcs_rr(XD, XS)                                                    \
        EMITW(0xF00002AF | MXM(REG(XD), 0x00,    REG(XS)))                  \
        EMITW(0xF00002AF | MXM(RYG(XD), 0x00,    RYG(XS)))

#define rndcs_ld(XD, MS, DS)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000619 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0xF00002AF | MXM(REG(XD), 0x00,    TmmM))/* ^ == -1 if true */\
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VYL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VYL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000619 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0xF00002AF | MXM(RYG(XD), 0x00,    TmmM))/* ^ == -1 if true */

#define cvtcs_rr(XD, XS)                                                    \
        rndcs_rr(W(XD), W(XS))                                              \
        cvzcs_rr(W(XD), W(XD))

#define cvtcs_ld(XD, MS, DS)                                                \
        rndcs_ld(W(XD), W(MS), W(DS))                                       \
        cvzcs_rr(W(XD), W(XD))

/* cvt (D = signed-int-to-fp S)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: only default ROUNDN is supported on pre-VSX Power systems */

#define cvtcn_rr(XD, XS)                                                    \
        EMITW(0xF00002E3 | MXM(REG(XD), 0x00,    REG(XS)))                  \
        EMITW(0xF00002E3 | MXM(RYG(XD), 0x00,    RYG(XS)))

#define cvtcn_ld(XD, MS, DS)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000619 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0xF00002E3 | MXM(REG(XD), 0x00,    TmmM))/* ^ == -1 if true */\
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VYL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VYL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000619 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0xF00002E3 | MXM(RYG(XD), 0x00,    TmmM))/* ^ == -1 if true */

/* cvr (D = fp-to-signed-int S)
 * rounding mode is encoded directly (cannot be used in FCTRL blocks)
 * NOTE: on targets with full-IEEE SIMD fp-arithmetic the ROUND*_F mode
 * isn't always taken into account when used within full-IEEE ASM block
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnrcs_rr(XD, XS, mode)                                              \
        EMITW(0x1000020A | MXM(REG(XD), 0x00,    REG(XS)) |                 \
        (RT_SIMD_MODE_##mode&3) << 6)                                       \
        EMITW(0x1000020A | MXM(RYG(XD), 0x00,    RYG(XS)) |                 \
        (RT_SIMD_MODE_##mode&3) << 6)

#define cvrcs_rr(XD, XS, mode)                                              \
        rnrcs_rr(W(XD), W(XS), mode)                                        \
        cvzcs_rr(W(XD), W(XD))

/******************************************************************************/
/********************************   INTERNAL   ********************************/
/******************************************************************************/

/* sregs */

#define sregs_sa() /* save all SIMD regs, destroys Reax */                  \
        movxx_ld(Reax, Mebp, inf_REGS)                                      \
        movcx_st(Xmm0, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        movcx_st(Xmm1, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        movcx_st(Xmm2, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        movcx_st(Xmm3, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        movcx_st(Xmm4, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        movcx_st(Xmm5, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        movcx_st(Xmm6, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        movcx_st(Xmm7, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        movcx_st(Xmm8, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        movcx_st(Xmm9, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        movcx_st(XmmA, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        movcx_st(XmmB, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        movcx_st(XmmC, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        movcx_st(XmmD, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        EMITW(0x7C000719 | MXM(TmmE,    0x00,    Teax))                     \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        EMITW(0x7C000719 | MXM(TmmE+16, 0x00,    Teax))                     \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        EMITW(0x7C000719 | MXM(TmmQ,    0x00,    Teax))                     \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        EMITW(0x7C000719 | MXM(TmmM,    0x00,    Teax))                     \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        EMITW(0x7C000718 | MXM(TmmQ,    0x00,    Teax))                     \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        EMITW(0x7C000718 | MXM(TmmM,    0x00,    Teax))

#define sregs_la() /* load all SIMD regs, destroys Reax */                  \
        movxx_ld(Reax, Mebp, inf_REGS)                                      \
        movcx_ld(Xmm0, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        movcx_ld(Xmm1, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        movcx_ld(Xmm2, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        movcx_ld(Xmm3, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        movcx_ld(Xmm4, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        movcx_ld(Xmm5, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        movcx_ld(Xmm6, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        movcx_ld(Xmm7, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        movcx_ld(Xmm8, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        movcx_ld(Xmm9, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        movcx_ld(XmmA, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        movcx_ld(XmmB, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        movcx_ld(XmmC, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        movcx_ld(XmmD, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        EMITW(0x7C000619 | MXM(TmmE,    0x00,    Teax))                     \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        EMITW(0x7C000619 | MXM(TmmE+16, 0x00,    Teax))                     \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        EMITW(0x7C000619 | MXM(TmmQ,    0x00,    Teax))                     \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        EMITW(0x7C000619 | MXM(TmmM,    0x00,    Teax))                     \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        EMITW(0x7C000618 | MXM(TmmQ,    0x00,    Teax))                     \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        EMITW(0x7C000618 | MXM(TmmM,    0x00,    Teax))

#endif /* RT_256 */

#endif /* RT_SIMD_CODE */

#endif /* RT_RTARCH_P32_256V2_H */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
