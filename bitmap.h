#ifndef BITMAP_H_INCLUDED
#define BITMAP_H_INCLUDED
#include "pub.h"

/** \brief
 *      胜比分与其他玩法互斥
 * \param psbf uint64_t*
 * \param rq int
 * \return void
 *
 */
void sbf_rqspf_jq_bqc(uint64_t* psbf, int rq);

/** \brief
 *      平比分与其他玩法互斥
 * \param ppbf uint64_t*
 * \param rq int
 * \return void
 *
 */
void pbf_rqspf_jq_bqc(uint64_t* ppbf, int rq);

/** \brief
 *      负比分与其他玩法互斥
 * \param pfbf uint64_t*
 * \param rq int
 * \return void
 *
 */
void fbf_rqspf_jq_bqc(uint64_t* pfbf, int rq);

/** \brief
 *      uint64转二进制字符串
 * \param a uint64_t
 * \return void
 *
 */
void longlongint_binarystr(uint64_t a);

/** \brief
 *      生成各玩法区间掩码
 * \param segmask uint64_t*
 * \return void
 *
 */
void make_segmask(uint64_t* segmask);

/** \brief
 *      生成54种玩法互斥位图
 * \param bitmap uint64_t*
 * \param rq int
 * \return void
 *
 */
void make_bitmap(uint64_t* bitmap, int rq);

#endif // BITMAP_H_INCLUDED
