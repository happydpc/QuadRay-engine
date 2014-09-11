/******************************************************************************/
/* Copyright (c) 2013-2014 VectorChief (at github, bitbucket, sourceforge)    */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#ifndef RT_SCN_TEST09_H
#define RT_SCN_TEST09_H

#include "format.h"

#include "all_mat.h"
#include "all_obj.h"

namespace scn_test09
{

/******************************************************************************/
/**********************************   BASE   **********************************/
/******************************************************************************/

rt_PLANE pl_floor01 =
{
    {      /*   RT_I,       RT_J,       RT_K    */
/* min */   {   -5.0,       -5.0,      -RT_INF  },
/* max */   {   +5.0,       +5.0,      +RT_INF  },
        {
/* OUTER        RT_U,       RT_V    */
/* scl */   {    1.0,        1.0    },
/* rot */              0.0           ,
/* pos */   {    0.0,        0.0    },

/* mat */   &mt_plain01_gray01,
        },
        {
/* INNER        RT_U,       RT_V    */
/* scl */   {    1.0,        1.0    },
/* rot */              0.0           ,
/* pos */   {    0.0,        0.0    },

/* mat */   &mt_plain01_gray02,
        },
    },
};

rt_PARABOLOID pb_clip01 =
{
    {      /*   RT_I,       RT_J,       RT_K    */
/* min */   {  -RT_INF,    -RT_INF,     -3.0    },
/* max */   {  +RT_INF,    +RT_INF,     +3.0    },
        {
/* OUTER        RT_U,       RT_V    */
/* scl */   {    1.0,        1.0    },
/* rot */              0.0           ,
/* pos */   {    0.0,        0.0    },

/* mat */   &mt_plain01_gray01,
        },
        {
/* INNER        RT_U,       RT_V    */
/* scl */   {    1.0,        1.0    },
/* rot */              0.0           ,
/* pos */   {    0.0,        0.0    },

/* mat */   &mt_plain01_gray01,
        },
    },
/* par */   1.0,
};

rt_HYPERPARABOLOID hp_clip01 =
{
    {      /*   RT_I,       RT_J,       RT_K    */
/* min */   {   -2.0,       -2.0,       -5.0    },
/* max */   {   +2.0,       +2.0,       +5.0    },
        {
/* OUTER        RT_U,       RT_V    */
/* scl */   {    1.0,        1.0    },
/* rot */              0.0           ,
/* pos */   {    0.0,        0.0    },

/* mat */   &mt_plain01_gray01,
        },
        {
/* INNER        RT_U,       RT_V    */
/* scl */   {    1.0,        1.0    },
/* rot */              0.0           ,
/* pos */   {    0.0,        0.0    },

/* mat */   &mt_plain01_gray01,
        },
    },
/* pr1 */   1.0,
/* pr2 */   1.0,
};

/******************************************************************************/
/*********************************   CAMERA   *********************************/
/******************************************************************************/

#if 0
rt_OBJECT ob_camera01[] =
{
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    1.0,        1.0,        1.0    },
/* rot */   { -142.14,       0.0,      114.42   },
/* pos */   {    2.637202,   2.535235,   0.0    },
        },
        RT_OBJ_CAMERA(&cm_camera01)
    },
};
#elif 1
rt_OBJECT ob_camera01[] =
{
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    1.0,        1.0,        1.0    },
/* rot */   {  -18.270023,   0.000000,   89.639969},
/* pos */   {    1.784209,   0.010557,   0.0    },
        },
        RT_OBJ_CAMERA(&cm_camera01)
    },
};
#else
rt_OBJECT ob_camera01[] =
{
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    1.0,        1.0,        1.0    },
/* rot */   {-91.559982, 0.000000, 12.240015},
/* pos */   {-2.526821, -3.969936,   0.0    },
        },
        RT_OBJ_CAMERA(&cm_camera01)
    },
};
#endif

/******************************************************************************/
/*********************************   LIGHTS   *********************************/
/******************************************************************************/

rt_OBJECT ob_light01[] =
{
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    1.0,        1.0,        1.0    },
/* rot */   {    0.0,        0.0,        0.0    },
/* pos */   {    0.0,        0.0,        0.0    },
        },
        RT_OBJ_LIGHT(&lt_light01)
    },
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    1.0,        1.0,        1.0    },
/* rot */   {    0.0,        0.0,        0.0    },
/* pos */   {    0.0,        0.0,        0.0    },
        },
        RT_OBJ_SPHERE(&sp_bulb01)
    },
};

/******************************************************************************/
/**********************************   TREE   **********************************/
/******************************************************************************/

rt_OBJECT ob_tree[] =
{
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    1.0,        1.0,        1.0    },
/* rot */   {    0.0,        0.0,        0.0    },
/* pos */   {    0.0,        0.0,        0.0    },
        },
        RT_OBJ_PLANE(&pl_floor01)
    },
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    1.0,        1.0,        1.0    },
/* rot */   {  180.0,        0.0,        0.0    },
/* pos */   {    0.0,        0.0,       10.0    },
        },
        RT_OBJ_PLANE(&pl_floor01)
    },
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    1.0,        1.0,        1.0    },
/* rot */   {  +90.0,        0.0,        0.0    },
/* pos */   {    0.0,       +5.0,       +5.0    },
        },
        RT_OBJ_PLANE(&pl_floor01)
    },
#if 0
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    0.5,        0.5,        1.0    },
/* rot */   {    0.0,        0.0,        0.0    },
/* pos */   {    0.0,        0.0,        0.8    },
        },
        RT_OBJ_PARABOLOID(&pb_clip01)
    },
#else
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    0.5,        0.5,       -1.0    },
/* rot */   {    0.0,        0.0,        0.0    },
/* pos */   {    0.0,        0.0,        0.8    },
        },
        RT_OBJ_HYPERPARABOLOID(&hp_clip01)
    },
#endif
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    1.0,        1.0,        1.0    },
/* rot */   {    0.0,        0.0,        0.0    },
/* pos */   {    0.0,        0.79,       3.3    },
        },
        RT_OBJ_ARRAY(&ob_light01),
    },
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    1.0,        1.0,        1.0    },
/* rot */   {    0.0,        0.0,        0.0    },
/* pos */   {    0.0,       -0.81,       3.3    },
        },
        RT_OBJ_ARRAY(&ob_light01),
    },
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    1.0,        1.0,        1.0    },
/* rot */   {    0.0,        0.0,        0.0    },
/* pos */   {    0.0,        0.0,        5.0    },
        },
        RT_OBJ_ARRAY(&ob_camera01)
    },
};

/******************************************************************************/
/**********************************   SCENE   *********************************/
/******************************************************************************/

rt_SCENE sc_root =
{
    RT_OBJ_ARRAY(&ob_tree),
};

} /* namespace scn_test09 */

#endif /* RT_SCN_TEST09_H */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
