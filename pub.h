#ifndef PUB_H_INCLUDED
#define PUB_H_INCLUDED

/**< Macros */
#define  _DEBUG    1
//#define  _TIME     1
#define int64_t    long long int           //lld
#define uint64_t   unsigned long long int  //llu
#define UNSET(x,y) ((x) ^= (1ULL << (y)))
#define SET(x,y)   ((x) |= (1ULL << (y)))
#define MASK64     0xffffffffffffffff      //64注彩票掩码
#define MASK54     0x3fffffffffffff        //54种玩法掩码
#define MAX_TZNUM  64
#define MAX_PLNUM  54
#define MAX_MCNUM  15
#define MAX_SPNUM  256

#define ERR_UNPACK_MCLIST    -1
#define ERR_UNPACK_MCINFO    -2
#define ERR_UNPACK_PLLIST    -3
#define ERR_UNPACK_PLINFO    -4
#define ERR_UNPACK_TZLIST    -5
#define ERR_UNPACK_TZINFO    -6
#define ERR_PLAY_NOFOUND     -7

//玩法数学模型
#define SPF_S(x,y)  ((x) >  (y))
#define SPF_P(x,y)  ((x) == (y))
#define SPF_F(x,y)  ((x) <  (y))
#define RQSPF_S(x,y,rq) ((x) + (rq) >  (y))
#define RQSPF_P(x,y,rq) ((x) + (rq) == (y))
#define RQSPF_F(x,y,rq) ((x) + (rq) <  (y))
#define BF_SQT(x,y)    ((x) >  (y) && ((x) > 5 || (y) > 2))
#define BF_PQT(x,y)    ((x) == (y) && (x) > 3)
#define BF_FQT(x,y)    ((x) <  (y) && ((x) > 2 || (y) > 5))
#define JQ(x,y,jq) ((jq) > 6 ? (x) + (y) >= (jq ): (x) + (y) == (jq))
#define BQC_SS(x,y) ((x) >  (y))
#define BQC_SP(x,y) ((x) == (y) && (y) > 0)
#define BQC_SF(x,y) ((x) <  (y) && (x) > 0)
#define BQC_PS(x,y) ((x) >  (y))
#define BQC_PP(x,y) ((x) == (y))
#define BQC_PF(x,y) ((x) <  (y))
#define BQC_FS(x,y) ((x) >  (y) && (y) > 0)
#define BQC_FP(x,y) ((x) == (y) && (y) > 0)
#define BQC_FF(x,y) ((x) <  (y))

/**< Structures */
//比赛内容
typedef struct mcctx{
    int no;                 //比赛编号(0-14)
    int dan;                //是否为胆(0/1)
    int rq;                 //让球数( >0主让客  <0客让主)
    int pn;                 //playnum
    int pl[MAX_PLNUM];       //playlist
    double odds[MAX_MCNUM];  //赔率列表
    char mid[32];           //赛事id
}MCCTX;

//投注内容
typedef struct tzctx{
    int    no;                 //投注编号(0-63)
    int    match[MAX_MCNUM];   //比赛列表
    int    play[MAX_MCNUM];    //玩法列表
    double odds[MAX_MCNUM];    //赔率列表
    double sprize;             //总奖金
    double uprize;             //单注奖金
    int    beishu;             //投注倍数
    int    expired;            //投注是否失效(0有效 1失效)
}TZCTX;

// 投注记录节点
typedef struct tznode{
    TZCTX  ctx;                     //投注内容
    int    visited;                 //是否被访问过
//  int    group[MAX_MCNUM][5];     //各场比赛玩法分组列表
//  int    groupnum[MAX_MCNUM];     //各场比赛玩法分组数
    double mprize;                  //当前节点至叶子节点的最大奖金
    struct tznode* chld[MAX_TZNUM]; //子节点列表
    int    chldnum;                 //子节点数
}TZNODE,*pTZNODE;

/**< Functions */
int  judge(int x, int y, int bf, int bqc, int jq, int spf, int rqspf, int rq);
int  test_mutex2(uint64_t play, uint64_t mask, uint64_t merge, uint64_t pm);
int  test_mutex3(int bf, int bqc, int jq, int spf, int rqspf, int rq);
void check_allin(int* pl, int pn, int mi);
void make_groups(int* pl, int pn, int rq, int mi);
void make_tzmap();
void expire_allin_zone();
void make_inclusion_map();
int  count_of_1(uint64_t a);
void cal_max_prize(uint64_t tz);
void dfs_max_prize(int m, uint64_t tz);
int  test_inclusion(int m, int p1, int p2);
void dfs_min_prize(pTZNODE tz, int times);
void prize_optimizer(int type);
void display_bitmap();
int  play_map(char* playid, char* pctx);
int  _split(char *str, char delim, char *res[MAX_SPNUM]);
int  init(char* mctx, char* tctx, double usermoney);
int  api_prize_optimizer(char* mctx, char* tctx, double usermoney, int type, char* res);

#endif // PUB_H_INCLUDED
