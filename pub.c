#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "pub.h"
#include "bitmap.h"
#include "heapsort.h"

// 54种玩法互斥位图
uint64_t bitmap[MAX_MCNUM][MAX_PLNUM];
uint64_t segmask[MAX_PLNUM];

// 15场比赛互斥分组
int group[MAX_MCNUM][3000][5]={0};
int groupnum[MAX_MCNUM]={0};
int playnum[MAX_MCNUM][3000]={0};
uint64_t tzmap[MAX_MCNUM][3000];
uint64_t allin[MAX_MCNUM];

//比赛
MCCTX match[MAX_MCNUM];
int   matchnum=0;

//投注记录
TZNODE  touzhu[MAX_TZNUM];
pTZNODE heap[MAX_TZNUM];
int     touzhunum = 0;

//最大最小奖金、用户投注金额
double max_prize   = -1.0;
double min_prize   = 9999999999.0;
double total_money = 0.0;

//比分玩法
int bfs[31][2] = {{1,0},{2,0},{2,1},{3,0},{3,1},{3,2},{4,0},{4,1},{4,2},{5,0},{5,1},{5,2},{7,0},\
                  {0,0},{1,1},{2,2},{3,3},{3,4}, \
                  {0,1},{0,2},{1,2},{0,3},{1,3},{2,3},{0,4},{1,4},{2,4},{0,5},{1,5},{2,5},{0,7}};

//playid映射表
char playmap[128][16] = {"354","3","354","1","354","0","269","3","269","1","269","0", \
                         "271","1:0","271","2:0","271","2:1","271","3:0","271","3:1","271","3:2", \
                         "271","4:0","271","4:1","271","4:2","271","5:0","271","5:1","271","5:2", \
                         "271","999:0","271","0:0","271","1:1","271","2:2","271","3:3","271","999:999", \
                         "271","0:1","271","0:2","271","1:2","271","0:3","271","1:3","271","2:3", \
                         "271","0:4","271","1:4","271","2:4","271","0:5","271","1:5","271","2:5", "271","0:999", \
                         "270","0","270","1","270","2","270","3","270","4","270","5","270","6","270","7", \
                         "272","33","272","31","272","30","272","13","272","11","272","10", \
                         "272","03","272","01","272","00"};

/** \brief
 *      根据比分来判断互斥关系
 * \param x int
 * \param y int
 * \param bf int
 * \param bqc int
 * \param jq int
 * \param spf int
 * \param rqspf int
 * \param rq int
 * \return int
 *
 */
int judge(int x, int y, int bf, int bqc, int jq, int spf, int rqspf, int rq){
    // 半全场判断
    if (bqc == 45 && !BQC_SS(x,y)) return 1;
    if (bqc == 46 && !BQC_SP(x,y)) return 1;
    if (bqc == 47 && !BQC_SF(x,y)) return 1;
    if (bqc == 48 && !BQC_PS(x,y)) return 1;
    if (bqc == 49 && !BQC_PP(x,y)) return 1;
    if (bqc == 50 && !BQC_PF(x,y)) return 1;
    if (bqc == 51 && !BQC_FS(x,y)) return 1;
    if (bqc == 52 && !BQC_FP(x,y)) return 1;
    if (bqc == 53 && !BQC_FF(x,y)) return 1;

    // 比分其他判断
    if (bf == 18 && !BF_SQT(x,y)) return 1;
    if (bf == 23 && !BF_PQT(x,y)) return 1;
    if (bf == 36 && !BF_FQT(x,y)) return 1;

    // 进球数判断
    if (jq != -1 && !JQ(x,y,jq-37)) return 1;

    // 胜平负判断
    if (spf == 0 && !SPF_S(x,y)) return 1;
    if (spf == 1 && !SPF_P(x,y)) return 1;
    if (spf == 2 && !SPF_F(x,y)) return 1;

    // 让球胜平负判断
    if (rqspf == 0 && !RQSPF_S(x,y,rq)) return 1;
    if (rqspf == 1 && !RQSPF_P(x,y,rq)) return 1;
    if (rqspf == 2 && !RQSPF_F(x,y,rq)) return 1;

    // 不互斥
    return 0;
}

/** \brief
 *      测试3种以上玩法组合是否互斥
 * \param bf int
 * \param bqc int
 * \param jq int
 * \param spf int
 * \param rqspf int
 * \param rq int
 * \return int  1互斥  0不互斥
 *
 */
int test_mutex3(int bf, int bqc, int jq, int spf, int rqspf, int rq){
    int i,j,k;

    // 判断三者或以上的互斥关系
    if (bf != -1 &&  bf != 18 && bf != 23 && bf != 36){
        i = bfs[bf-6][0];
        j = bfs[bf-6][1];
        return judge(i, j, bf, bqc, jq, spf, rqspf, rq);
    }
    else{
        for (i=0; i<=10; ++i){
            for(j=0; j<=10; ++j){
                if (!judge(i, j, bf, bqc, jq, spf, rqspf, rq))
                    return 0;
            }
        }
    }

    //在有限范围内无解
    return 1;
}

/** \brief
 *      测试两种玩法是否互斥
 * \param play uint64_t
 * \param mask uint64_t
 * \param merge uint64_t
  * \param pm uint64_t
 * \return int  1互斥  0不互斥
 *
 */
int test_mutex2(uint64_t play, uint64_t mask, uint64_t merge, uint64_t pm){
    int i,j,k;
    if (!(play & mask & merge))
        return 1;

    //双向判断
    j = 0;
    while(pm){
        if (pm & 1){
            if (!(merge & segmask[j] & play)) return 1;
        }
        pm = pm >> 1;
        j++;
    }

    return 0;
}


/** \brief
 *      检测5大玩法区间，剔除掉各区间全包时赔率较大的玩法所在的投注
 * \return void
 *
 */
 void expire_allin_zone(){
    int i,j,k,mi,ti;
    double min_odds;
    for (mi=0; mi<matchnum; ++mi){
        uint64_t ap = 0;
        for (i=0; i<match[mi].pn; ++i)
            SET(ap, match[mi].pl[i]);

        int pidx[5], pnum=0;
        //spf
        if ((ap & segmask[0]) == segmask[0]){
            min_odds = 99999;
            pnum++;
            for (i=0; i<3; ++i){
                if (min_odds > match[mi].odds[i]){
                    min_odds = match[mi].odds[i];
                    pidx[pnum-1] = i;
                }
            }
        }
        for (ti=0; ti<touzhunum; ++ti){
            if (touzhu[ti].ctx.match[mi] != -1){
                for (i=0; i<pnum; ++i){
                    if (touzhu[ti].ctx.play[mi] == pidx[i]) break;
                }
                if (i==pnum) touzhu[ti].ctx.expired = 1;
            }
        }
    }

    int offset = 0;
    for (ti=0; ti<touzhunum; ++ti){
        if (touzhu[ti].ctx.expired == 1){
            offset++;
            continue;
        }
        touzhu[ti-offset] = touzhu[ti];
    }
    touzhunum -= offset;
    printf("=============%d\n",touzhunum);
 }

/** \brief
 *      检测一场比赛所有玩法是否存在全包的情况
 * \param pl int*
 * \param pn int
 * \param rq int
 * \return void
 *
 */
void check_allin(int* pl, int pn, int mi){
    int i,j,k;
    uint64_t p,ap = 0;
    for (i=0; i<pn; ++i)
        SET(ap, pl[i]);

    //5个玩法区间检测allin
    for (i=0; i<pn; ++i){
        //spf
        p = bitmap[mi][pl[i]] & segmask[0];
        j = count_of_1(p);
        if (!(pl[i]>=0 && pl[i]<3) && j>1)
            if ((p & ap) == p){
                SET(allin[mi], pl[i]);
                continue;
            }
        //rqspf
        p = bitmap[mi][pl[i]] & segmask[3];
        j = count_of_1(p);
        if (!(pl[i]>=3 && pl[i]<6) && j>1)
            if ((p & ap) == p){
                SET(allin[mi], pl[i]);
                continue;
            }
        //bf
        p = bitmap[mi][pl[i]] & segmask[6];
        j = count_of_1(p);
        if (!(pl[i]>=6 && pl[i]<37) && j>1)
            if ((p & ap) == p){
                SET(allin[mi], pl[i]);
                continue;
            }
        //jq
        p = bitmap[mi][pl[i]] & segmask[37];
        j = count_of_1(p);
        if (!(pl[i]>=37 && pl[i]<45) && j>1)
            if ((p & ap) == p){
                SET(allin[mi], pl[i]);
                continue;
            }
        //bqc
        p = bitmap[mi][pl[i]] & segmask[45];
        j = count_of_1(p);
        if (!(pl[i]>=45 && pl[i]<54) && j>1)
            if ((p & ap) == p){
                SET(allin[mi], pl[i]);
                continue;
            }
    }
}

/** \brief
 *      一场比赛所有玩法做互斥分组
 * \param pl int*
 * \param pn int
 * \param rq int
 * \return void
 *
 */
void make_groups(int* pl, int pn, int rq, int mi){
    int i,j,k,a,b,c,d,e;

    // 5大类玩法
    int spf[4],rqspf[4],bf[32],jq[9],bqc[10];
    int nspf, nrqspf, nbf, njq, nbqc;

    // 设置空选项，表示无该玩法
    spf[0] = rqspf[0] = bf[0] = jq[0] = bqc[0] = -1;
    nspf   = nrqspf   = nbf   = njq   = nbqc   =  1;

    // 用户选择玩法分类
    for (i=0; i<pn; ++i){
        if (pl[i]>=0 && pl[i]<3){
            spf[nspf++] = pl[i];
        }
        else if (pl[i]>=3 && pl[i]<6){
            rqspf[nrqspf++] = pl[i];
        }
        else if (pl[i]>=6 && pl[i]<37){
            bf[nbf++] = pl[i];
        }
        else if (pl[i]>=37 && pl[i]<45){
            jq[njq++] = pl[i];
        }
        else if (pl[i]>=45 && pl[i]<54){
            bqc[nbqc++] = pl[i];
        }
    }

    // 遍历笛卡尔积，输出不互斥组合(约2000+)
    uint64_t ma,mb,mc,md,me,pm;
    for (a=0; a<nbf; ++a){
        //两两互斥减枝
        ma = MASK54;
        pm = 0;
        if (bf[a] != -1 && test_mutex2(bitmap[mi][bf[a]],segmask[bf[a]],ma,pm)) continue;
        for (b=0; b<nbqc; ++b){
            //两两互斥减枝
            if (bf[a]  != -1) {mb = ma & bitmap[mi][bf[a]]; SET(pm, bf[a]);} else {mb = ma;}
            if (bqc[b] != -1 && test_mutex2(bitmap[mi][bqc[b]],segmask[bqc[b]],mb,pm)) continue;
            for (c=0; c<njq; ++c){
                //两两互斥减枝
                if (bqc[b]!= -1) {mc = mb & bitmap[mi][bqc[b]]; SET(pm, bqc[b]);} else {mc = mb;}
                if (jq[c] != -1 && test_mutex2(bitmap[mi][jq[c]],segmask[jq[c]],mc,pm)) continue;
                for (d=0; d<nspf; ++d){
                    //两两互斥减枝
                    if (jq[c]  != -1) {md = mc & bitmap[mi][jq[c]]; SET(pm, jq[c]);} else {md = mc;}
                    if (spf[d] != -1 && test_mutex2(bitmap[mi][spf[d]],segmask[spf[d]],md,pm)) continue;
                    for (e=0; e<nrqspf; ++e){
                        //两两互斥减枝
                        if (spf[d] != -1) {me = md & bitmap[mi][spf[d]]; SET(pm, spf[d]);} else {me = md;}
                        if (rqspf[e] != -1 && test_mutex2(bitmap[mi][rqspf[e]],segmask[rqspf[e]],me,pm)) continue;

                        //多玩法互斥检测
                        if (-1 == bf[a] && -1 == bqc[b] && -1 == jq[c] \
                                && -1 == spf[d] && -1 == rqspf[e]) continue;
                        if(!test_mutex3(bf[a], bqc[b], jq[c], spf[d], rqspf[e], rq)){
                            if (bf[a]  != -1) group[mi][groupnum[mi]][playnum[mi][groupnum[mi]]++] = bf[a];
                            if (bqc[b] != -1) group[mi][groupnum[mi]][playnum[mi][groupnum[mi]]++] = bqc[b];
                            if (jq[c]  != -1) group[mi][groupnum[mi]][playnum[mi][groupnum[mi]]++] = jq[c];
                            if (spf[d] != -1) group[mi][groupnum[mi]][playnum[mi][groupnum[mi]]++] = spf[d];
                            if (rqspf[e]!= -1)group[mi][groupnum[mi]][playnum[mi][groupnum[mi]]++] = rqspf[e];
                            groupnum[mi]++;
#ifdef _DEBUG
                            printf("spf=%d rqspf=%d bf=%d jq=%d bqc=%d ", spf[d], rqspf[e], bf[a], jq[c], bqc[b]);
                            printf("mi=%d gn=%d pn=%d\n", mi, groupnum[mi], playnum[mi][groupnum[mi]-1]);
#endif // _DEBUG
                        }
                    }
                }
            }
        }
    }
}

/** \brief
 *      互斥分组到投注列表的映射
 * \return void
 *
 */
void make_tzmap(){
    int i,j,k,m,p,t;
    for (i=0; i<MAX_MCNUM; ++i)
        for (j=0; j<3000; ++j)
            tzmap[i][j] = 0;
    for (i=0; i<touzhunum; ++i){
        for (j=0; j<matchnum; ++j){
            m = touzhu[i].ctx.match[j];
            p = touzhu[i].ctx.play[j];
#ifdef _DEBUG
            printf("m=%d, p=%d  gn=%d\n", m,p,groupnum[m]);
#endif // _DEBUG
            for (k=0; k<groupnum[j]; ++k){
                if (m == -1){ //第i注没有选择第j场比赛
                    SET(tzmap[j][k], i);
                    continue;
                }
                for (t=0; t<playnum[j][k]; ++t){
                    if (p == group[j][k][t]){
                        SET(tzmap[j][k], i);
                    }
                }
            }
        }
    }
}

/** \brief
 *      生成兼容关系图
 * \return void
 *
 */
void make_inclusion_map(){
    int i,j,k,m1,m2;
    uint64_t p1,p2;
    for (i=0; i<touzhunum; ++i){
        for (j=0; j<touzhunum; ++j){
            if (i!=j){
                int inclusion = 1;
                for (k=0; k<matchnum; ++k){
                    m1 = touzhu[i].ctx.match[k];
                    m2 = touzhu[j].ctx.match[k];
                    if (m2 == -1) continue;
                    if (m1 == -1){
                        inclusion = 0;
                        break;
                    }
                    p1 = bitmap[k][touzhu[i].ctx.play[k]];
                    p2 = bitmap[k][touzhu[j].ctx.play[k]];
                    if (!test_inclusion(k, touzhu[i].ctx.play[k], touzhu[j].ctx.play[k])){
                        inclusion = 0;
                        break;
                    }
                }
                if (inclusion){
                    touzhu[i].chld[touzhu[i].chldnum++] = &touzhu[j];
                }
            }

        }
    }
}

/** \brief
 *      计算二进制中1的个数
 * \param a uint64_t
 * \return int
 *
 */
int count_of_1(uint64_t a){
    int n = 0;
    while(a){
        n++;
        a = a & (a-1);
    }
    return n;
}

/** \brief
 *      计算最大奖金
 * \param tz uint64_t
 * \return void
 *
 */
void cal_max_prize(uint64_t tz){
    double p = 0.0;
    int    n = 0;
    while(tz){
        if (tz & 1){
            p += touzhu[n].ctx.sprize;
        }
        tz = tz >> 1;
        n++;
    }
    if (max_prize < p) max_prize = p;
}

/** \brief
 *      深度递归投注记录，计算最大奖金
 * \param m int
 * \param tz uint64_t
 * \return void
 *
 */
void dfs_max_prize(int m, uint64_t tz){
    int i,j,k;

    // 最多15场或只剩一注
    if (m==matchnum || count_of_1(tz) == 1){
        cal_max_prize(tz);
        return;
    }

    // 对每一个分组做聚类
    for (i=0; i<groupnum[m]; ++i){
#ifdef _DEBUG
        printf("map=");longlongint_binarystr(tzmap[m][i]);
        printf("tz=");longlongint_binarystr(tz);
#endif // _DEBUG
        dfs_max_prize(m+1, tzmap[m][i] & tz);
    }
}

/** \brief
 *      测试第m场比赛玩法p1是否兼容玩法p2
 * \param m int
 * \param p1 int
 * \param p2 int
 * \return int
 *
 */
int test_inclusion(int m, int p1, int p2){
    uint64_t x,y;
    x = bitmap[m][p1] & segmask[p2];
    y = bitmap[m][p2] & segmask[p2];
    if (x ^ y) return 0;
    return 1;
}

/** \brief
 *      深度递归投注记录，计算最小奖金
 * \param tz pTZNODE
 * \param times int
 * \return void
 *
 */
void dfs_min_prize(pTZNODE tz, int times){
    int i,j,k;

    //如果tz节点已访问过，直接返回
    if (tz->visited > times) return;
    tz->visited = times + 1;

    //不同路径奖金之和为当前节点最小奖金
    double mp = 0.0;
    for (i=0; i<tz->chldnum; ++i){
        dfs_min_prize(tz->chld[i], times);
        mp += tz->chld[i]->mprize;
    }

    // 最小奖金 = 下游奖金 + 自身奖金
    tz->mprize = mp + tz->ctx.sprize;
}

/** \brief
 *      奖金优化：平均(0)、搏冷(1)、搏热(2)
 * \param type=0 int
 * \return void
 *
 */
void prize_optimizer(int type){
    double left_money = total_money - touzhunum*2.0;
    while(left_money > 0){
        // 大于本金的部分用于搏冷或搏热
        if (type > 0 && heap[0]->ctx.sprize >= total_money){
            break;
        }

        heap[0]->ctx.sprize += heap[0]->ctx.uprize;
        heap[0]->ctx.beishu++;
        min_heapfy(&heap[0],0,touzhunum);
        left_money -= 2.0;
    }

    int i,j,k;
    if (left_money > 0){
        pTZNODE minpt,maxpt;
        /*
        minpt = maxpt = heap[0];
        for (i=1; i<touzhunum; ++i){
            if (heap[i]->ctx.uprize > maxpt->ctx.uprize) maxpt = heap[i];
            if (heap[i]->ctx.uprize < minpt->ctx.uprize) minpt = heap[i];
        }
        */
        minpt = &touzhu[0];
        maxpt = &touzhu[touzhunum-1];
        if (type == 1){    //搏冷
            maxpt->ctx.beishu += ((int)left_money)/2;
            maxpt->ctx.sprize = maxpt->ctx.uprize*maxpt->ctx.beishu;
        }
        else if(type == 2){//搏热
            minpt->ctx.beishu += ((int)left_money)/2;
            minpt->ctx.sprize = minpt->ctx.uprize*minpt->ctx.beishu;
        }
    }
}

/** \brief
 *      打印bitmap
 * \return void
 *
 */
void display_bitmap(){
    int i,j,k;
    for (i=0; i<matchnum; ++i){
        for (j=0; j<MAX_PLNUM; ++j){
            longlongint_binarystr(bitmap[i][j]);
        }
        printf("\n");
    }
}

/** \brief
 *      按分隔符切分字符串
 * \param str char*
 * \param delim char
 * \param res[MAX_SPLIT] char*
 * \return int
 *
 */
int _split(char *str, char delim, char *res[MAX_SPNUM]){
	int cnt;
	char *p;
	for(p=str, cnt=0; *str != 0x00 && cnt < MAX_SPNUM; ++str) {
		if (*str == delim) {
			*str=0x00;
			if (p != str) {
				res[cnt++] = p;
			}
			p=str+1;
		}
	}

	if (p != str) {
		res[cnt++] = p;
	}

	return cnt;
}

/** \brief
 *      将玩法映射成编号
 * \param playid char*
 * \param pctx char*
 * \return int
 *
 */
int play_map(char* playid, char* pctx){
    int i;
    for(i=0; i<127; i+=2){
        if (strcmp(playmap[i], playid)==0 \
         && strcmp(playmap[i+1], pctx)==0)
            return (int)(i/2);
    }
    printf("%s  %s\n", playid, pctx);
    return -1;
}

/** \brief
 *      初始化全局变量
 * \param mctx char*
 * \param tctx char*
 * \param usermoney double
 * \return void
 *
 */
int init(char* mctx, char* tctx, double usermoney){
    int i,j,k,sz,tznum;

    // 全局变量初始化
    max_prize   = 0.0;
    min_prize   = 9999999999.0;
    total_money = 0.0;
    matchnum    = 0;
    touzhunum   = 0;
    for (i=0; i<MAX_MCNUM; ++i){
        groupnum[i] = 0;
        for (j=0; j<3000; ++j){
            playnum[i][j] = 0;
        }
        allin[i] = 0;
    }

    //解包比赛信息:
    // 场次ID|让球(分)|是否胆码(0或1)|玩法#选择#赔率/玩法#选择#赔率/...;场次ID|让球(分)....
    //"001|6.5|0|275#0#1.62/275#3#1.78/274#0#1.21/274#3#2.83/276#6#3.80/276#7#5/276#8#11.5;
    // 002|-3.5|0|275#0#1.65/275#3#1.75/274#0#2.18/274#3#1.39"
    char* smatch[MAX_SPNUM] = {0};
    if((matchnum=_split(mctx, ';', smatch)) <= 0)
        return ERR_UNPACK_MCLIST;

    for (i=0; i<matchnum; ++i){
        char* minfo[MAX_SPNUM] = {0};
        if(_split(smatch[i], '|', minfo) !=4)
            return ERR_UNPACK_MCINFO;

        strcpy(match[i].mid, minfo[0]);
        match[i].rq = atoi(minfo[1]);
        match[i].no = i;
        char* splay[MAX_SPNUM] = {0};
        if((match[i].pn=_split(minfo[3], '/', splay)) <= 0)
            return ERR_UNPACK_PLLIST;

        for (j=0; j<match[i].pn; ++j){
            char* pinfo[MAX_SPNUM] = {0};
            if(_split(splay[j], '#', pinfo) != 3)
                return ERR_UNPACK_PLINFO;
            if((match[i].pl[j]=play_map(pinfo[0],pinfo[1])) == -1)
                return ERR_PLAY_NOFOUND;
            match[i].odds[j] = atof(pinfo[2]);
        }
    }

    //解包投注信息
    //"83743#354#3#2.90;83744#354#3#4.60;1;100.00|
    // 83743#354#3#2.90;83745#354#3#2.85|
    // 83743#354#3#2.90;83745#354#1#2.95|
    // 83743#354#3#2.90;83745#354#0#2.31|
    // 83744#354#3#4.60;83745#354#3#2.85|
    // 83744#354#3#4.60;83745#354#1#2.95|
    // 83744#354#3#4.60;83745#354#0#2.31|
    // 83743#354#3#2.90;83744#354#3#4.60;83745#354#3#2.85|
    // 83743#354#3#2.90;83744#354#3#4.60;83745#354#1#2.95|
    // 83743#354#3#2.90;83744#354#3#4.60;83745#354#0#2.31"
    char* stouzhu[MAX_SPNUM] = {0};
    if ((tznum=_split(tctx, '|', stouzhu)) <= 0)
        return ERR_UNPACK_TZLIST;

    for(i=0; i<tznum; ++i){
        char* tinfo[MAX_SPNUM] = {0};
        if ((sz=_split(stouzhu[i], ';', tinfo)) <= 0)
            return ERR_UNPACK_TZINFO;

        // 剔除投注倍数为0的
        if (atof(tinfo[sz-2]) == 0) {continue;}

        touzhu[touzhunum].ctx.no = touzhunum;
        char* ssmatch[MAX_MCNUM][MAX_SPNUM];
        for (j=0; j<sz-2; ++j){
            if (_split(tinfo[j], '#', ssmatch[j]) != 4)
                return ERR_UNPACK_MCLIST;
        }
        for (j=0,k=0; j<matchnum; ++j){
            if (k<sz-2 && strcmp(match[j].mid, ssmatch[k][0]) == 0){
                touzhu[touzhunum].ctx.match[j] = j;
                touzhu[touzhunum].ctx.odds[j]  = atof(ssmatch[k][3]);
                if((touzhu[touzhunum].ctx.play[j]=play_map(ssmatch[k][1], ssmatch[k][2]))==-1)
                    return ERR_PLAY_NOFOUND;
                k++;
            }
            else{
                touzhu[touzhunum].ctx.match[j] = -1;  //补齐
            }
        }

        touzhu[touzhunum].ctx.uprize = atof(tinfo[sz-1]);
        touzhu[touzhunum].ctx.beishu = atoi(tinfo[sz-2]);
        touzhu[touzhunum].ctx.sprize = touzhu[touzhunum].ctx.uprize*touzhu[touzhunum].ctx.beishu;
        touzhu[touzhunum].ctx.expired    = 0;
        touzhu[touzhunum].visited    = 0;
        touzhu[touzhunum].chldnum    = 0;
        touzhu[touzhunum].mprize     = 0;
        touzhunum++;
    }

    //用户投注金额初始化
    if (usermoney > touzhunum*2){
        total_money = usermoney;
    }
    else{
        total_money = touzhunum*2;
    }
    return 0;
}

/** \brief
 *       奖金优化api接口
 * \param mctx char*    比赛列表
 * \param tctx char*    投注列表
 * \param usermoney double  用户投注金额
 * \param type int          优化类型(-1不优化/0平均/1搏冷/2搏热)
 * \param res char*         返回结果: 最小奖金#最大奖金#倍数列表 (21.35#178.99#1;1;1;1)
 * \return int              0成功， 其他失败
 *
 */
int api_prize_optimizer(char* mctx, char* tctx, double usermoney, int type, char* res){

    int i,j,k,m,p,jump,ret;
    uint64_t p64;
    char beishu[64*1024] = {'\0'};
    char maxprize[64]    = {'\0'};
    char minprize[64]    = {'\0'};

#ifdef _TIME
    clock_t t1, t2, t3, t4, t5;
    t1 = clock();
#endif // _TIME

    //初始化
    if ((ret=init(mctx, tctx, usermoney))!=0)
        return ret;

#ifdef _TIME
    t2 = clock();
#endif // _TIME

    //奖金优化
    if(type >= 0){
        for(i=0; i<touzhunum; ++i)
            heap[i] = &touzhu[i];
        build_min_heap(&heap[0], touzhunum);
        prize_optimizer(type);
        for (i=0; i<touzhunum; ++i){
            char buf[10];
            sprintf(buf, "%d", touzhu[i].ctx.beishu);
            strcat(beishu, buf);
            strcat(beishu, ";");
#ifdef _DEBUG
            printf("uprize=%f beishu=%d sprize=%f\n",touzhu[i].ctx.uprize,\
                touzhu[i].ctx.beishu,touzhu[i].ctx.sprize);
#endif // _DEBUG
        }
        beishu[strlen(beishu)-1] = '\0';
    }
#ifdef _TIME
    t3 = clock();
#endif // _TIME

    //投注倍数全为0
    if (touzhunum == 0) max_prize = min_prize = 0.0;

    //最大奖金
    make_segmask(&segmask[0]);
    for (i=0; i<matchnum; ++i){
        make_bitmap(&bitmap[i][0], match[i].rq);
        make_groups(&match[i].pl[0], match[i].pn, match[i].rq, i);
    }

#ifdef _DEBUG
    //display_bitmap();
#endif // _DEBUG

    make_tzmap();

#ifdef _DEBUG
    for (i=0; i<matchnum; ++i){
        for (k=0; k<groupnum[i]; ++k){
            longlongint_binarystr(tzmap[i][k]);
        }
        printf("\n");
    }
#endif // _DEBUG

    uint64_t tz = 0;
    for (i=0; i<touzhunum; ++i)
        SET(tz, i);
    dfs_max_prize(0, tz);
#ifdef _TIME
    t4 = clock();
#endif // _TIME

    //最小奖金
    //expire_allin_zone();   
    make_inclusion_map();
    for (i=0; i<matchnum; ++i)
        check_allin(&match[i].pl[0], match[i].pn, i);
    for (i=0; i<touzhunum; ++i){
        //跳过含有全包玩法的投注
        for (j=0,jump=0; j<matchnum; ++j){
            if (touzhu[i].ctx.match[j] != -1){
                m = touzhu[i].ctx.match[j];
                p = touzhu[i].ctx.play[j];
                p64 = 0; SET(p64, p);
                if (allin[m] & p64){
                    jump = 1;
                    break;
                }
            }
        }
        if (jump) continue;
        dfs_min_prize(&touzhu[i], i);
        if (min_prize > touzhu[i].mprize){
            min_prize = touzhu[i].mprize;
        }
    }
#ifdef _TIME
    t5 = clock();
    printf("======init cost:%lf\n", (double)(t2-t1)*1000/CLOCKS_PER_SEC);
    printf("======opt  cost:%lf\n", (double)(t3-t2)*1000/CLOCKS_PER_SEC);
    printf("======max  cost:%lf\n", (double)(t4-t3)*1000/CLOCKS_PER_SEC);
    printf("======min  cost:%lf\n", (double)(t5-t4)*1000/CLOCKS_PER_SEC);
#endif // _TIME

    //输出结果
    sprintf(minprize,"%.2lf", min_prize);
    sprintf(maxprize,"%.2lf", max_prize);
    strcat(res, minprize);
    strcat(res, "#");
    strcat(res, maxprize);
    if (type >= 0){
        strcat(res, "#");
        strcat(res, beishu);
    }
    return 0;
}
