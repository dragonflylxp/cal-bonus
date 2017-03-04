#include "bitmap.h"

/**< Constants */
// 54种玩法: 胜平负(0~2)、让胜平负(3~5)、比分(6~36)、进球(37~44)、半全场(45~53)
int sbf[13][2] = {{1,0},{2,0},{2,1},{3,0},{3,1},{3,2},{4,0},{4,1},{4,2},{5,0},{5,1},{5,2},{7,0}};
int pbf[5][2]  = {{0,0},{1,1},{2,2},{3,3},{3,4}};
int fbf[13][2] = {{0,1},{0,2},{1,2},{0,3},{1,3},{2,3},{0,4},{1,4},{2,4},{0,5},{1,5},{2,5},{0,7}};

char playname[MAX_PLNUM][10]={
        "胜","平","负","让胜","让平","让负",
        "1:0","2:0","2:1","3:0","3:1","3:2","4:0","4:1","4:2","5:0","5:1","5:2","胜其他",
        "0:0","1:1","2:2","3:3","平其他",
        "0:1","0:2","1:2","0:3","1:3","2:3","0:4","1:4","2:4","0:5","1:5","2:5","负其他",
        "0","1","2","3","4","5","6","7+",
        "胜胜","胜平","胜负","平胜","平平","平负","负胜","负平","负负"
};

void sbf_rqspf_jq_bqc(uint64_t* psbf, int rq){
    int i,j,k;

    // 让球互斥
    if (rq > 0){       //受球必胜
        for (i=0; i<13; ++i){
            UNSET(*(psbf+i), 4);
            UNSET(*(psbf+i), 5);
        }
    }
    else if (rq < 0){  //让球
        rq = -rq;
        for (i=0; i<12; ++i){
            j = sbf[i][0] - rq;
            k = sbf[i][1];
            if (j > k){
                UNSET(*(psbf+i), 4);
                UNSET(*(psbf+i), 5);
            }
            else if (j == k){
                UNSET(*(psbf+i), 3);
                UNSET(*(psbf+i), 5);
            }
            else{
                UNSET(*(psbf+i), 3);
                UNSET(*(psbf+i), 4);
            }
        }
        if (rq == 1){
            UNSET(*(psbf+12), 5);
        }
    }

    // 进球互斥
    for (i=0; i<13; ++i){
        j = sbf[i][0] + sbf[i][1];
        for (k=37; k<45; ++k){
            if (k-37 != j){
                if (i==12 && k==43) continue; //6:0的情况
                UNSET(*(psbf+i), k);
            }
        }
    }


    // 半全场互斥
    for (i=0; i<13; ++i){
        UNSET(*(psbf+i), 46);
        UNSET(*(psbf+i), 47);
        UNSET(*(psbf+i), 49);
        UNSET(*(psbf+i), 50);
        UNSET(*(psbf+i), 52);
        UNSET(*(psbf+i), 53);

        //客队无进球，则与负胜互斥
        if (i !=12 && sbf[i][1] == 0){
            UNSET(*(psbf+i), 51);
        }
    }
}

void pbf_rqspf_jq_bqc(uint64_t* ppbf, int rq){
    int i,j,k;
    // 让球互斥
    if (rq > 0){ // 受球必胜
        for (i=0; i<5; ++i){
            UNSET(*(ppbf+i), 4);
            UNSET(*(ppbf+i), 5);
        }
    }
    else if (rq < 0){ // 让球必败
        for (i=0; i<5; ++i){
            UNSET(*(ppbf+i), 3);
            UNSET(*(ppbf+i), 4);
        }
    }

    // 进球互斥
    for (i=0; i<5; ++i){
        j = pbf[i][0] + pbf[i][1];
        for (k=37; k<45; ++k){
            if (k-37 != j){
                UNSET(*(ppbf+i), k);
            }
        }
    }

    // 半全场互斥
    for (i=0; i<5; ++i){
        UNSET(*(ppbf+i), 45);
        UNSET(*(ppbf+i), 47);
        UNSET(*(ppbf+i), 48);
        UNSET(*(ppbf+i), 50);
        UNSET(*(ppbf+i), 51);
        UNSET(*(ppbf+i), 53);

        //全场无进球，则与胜平、负平互斥
        if (i == 0){
            UNSET(*(ppbf+i), 46);
            UNSET(*(ppbf+i), 52);
        }
    }
}

void fbf_rqspf_jq_bqc(uint64_t* pfbf, int rq){
    int i,j,k;

    // 让球互斥
    if (rq < 0){  //让球必败
        for (i=0; i<13; ++i){
            UNSET(*(pfbf+i), 3);
            UNSET(*(pfbf+i), 4);
        }
    }
    else if (rq > 0){  //受球
        for (i=0; i<12; ++i){
            j = fbf[i][0];
            k = fbf[i][1] - rq;
            if (j > k){
                UNSET(*(pfbf+i), 4);
                UNSET(*(pfbf+i), 5);
            }
            else if (j == k){
                UNSET(*(pfbf+i), 3);
                UNSET(*(pfbf+i), 5);
            }
            else{
                UNSET(*(pfbf+i), 3);
                UNSET(*(pfbf+i), 4);
            }
        }
        if (rq == 1){
            UNSET(*(pfbf+12), 3);
        }
    }

    // 进球互斥
    for (i=0; i<13; ++i){
        j = fbf[i][0] + fbf[i][1];
        for (k=37; k<45; ++k){
            if (k-37 != j){
                if(i==12 && k==43) continue; // 0:6的情况
                UNSET(*(pfbf+i), k);
            }
        }
    }

    // 半全场互斥
    for (i=0; i<13; ++i){
        UNSET(*(pfbf+i), 45);
        UNSET(*(pfbf+i), 46);
        UNSET(*(pfbf+i), 48);
        UNSET(*(pfbf+i), 49);
        UNSET(*(pfbf+i), 51);
        UNSET(*(pfbf+i), 52);

        //主队无进球，则与胜负互斥
        if (i !=12 && fbf[i][0] == 0){
            UNSET(*(pfbf+i), 47);
        }
    }
}

void longlongint_binarystr(uint64_t a){
    char str[59];
    int i,j,k;
    for (i=0; i<59; ++i){
        if (i==3||i==7||i==39||i==48)
        {
            str[i] = ' ';
            continue;
        }
        str[i] = (a & 1) > 0 ? '1' : '0';
        a = a>>1 & MASK54;
    }
    str[58] = '\0';
    printf("%s\n", str);
}

void make_segmask(uint64_t* segmask){
    int i,j,k;
    uint64_t mask;

    // spf
    mask = 0;
    for (i=0; i<3; ++i) SET(mask, i);
    for (i=0; i<3; ++i) segmask[i] = mask;

    // rqspf
    mask = 0;
    for (i=3; i<6; ++i) SET(mask, i);
    for (i=3; i<6; ++i) segmask[i] = mask;


    // bf
    mask = 0;
    for (i=6; i<37; ++i) SET(mask, i);
    for (i=6; i<37; ++i) segmask[i] = mask;

    // jq
    mask = 0;
    for (i=37; i<45; ++i) SET(mask, i);
    for (i=37; i<45; ++i) segmask[i] = mask;

    // bqc
    mask = 0;
    for (i=45; i<54; ++i) SET(mask, i);
    for (i=45; i<54; ++i) segmask[i] = mask;
}

void make_bitmap(uint64_t* bitmap, int rq){
    int i,j,k;

    //胜
    uint64_t spf_s = MASK54;
    UNSET(spf_s, 1);
    UNSET(spf_s, 2);
    if (rq > 0) {
        UNSET(spf_s, 4);
        UNSET(spf_s, 5);
    }
    for (i=19; i<=36; ++i){
        UNSET(spf_s, i);
    }
    UNSET(spf_s, 37);
    UNSET(spf_s, 46);
    UNSET(spf_s, 47);
    UNSET(spf_s, 49);
    UNSET(spf_s, 50);
    UNSET(spf_s, 52);
    UNSET(spf_s, 53);

    //平
    uint64_t spf_p = MASK54;
    UNSET(spf_p, 0);
    UNSET(spf_p, 2);
    if (rq < 0){
        UNSET(spf_p, 3);
        UNSET(spf_p, 4);
    }
    else if (rq > 0){
        UNSET(spf_p, 4);
        UNSET(spf_p, 5);
    }
    for (i=6; i<=18; ++i){
        UNSET(spf_p, i);
    }
    for (i=24; i<=36; ++i){
        UNSET(spf_p, i);
    }
    UNSET(spf_p, 38);
    UNSET(spf_p, 40);
    UNSET(spf_p, 42);
    UNSET(spf_p, 45);
    UNSET(spf_p, 47);
    UNSET(spf_p, 48);
    UNSET(spf_p, 50);
    UNSET(spf_p, 51);
    UNSET(spf_p, 53);

    //负
    uint64_t spf_f = MASK54;
    UNSET(spf_f, 0);
    UNSET(spf_f, 1);
    if (rq < 0){
        UNSET(spf_f, 3);
        UNSET(spf_f, 4);
    }
    for (i=6; i<=23; ++i){
        UNSET(spf_f, i);
    }
    UNSET(spf_f, 37);
    UNSET(spf_f, 45);
    UNSET(spf_f, 46);
    UNSET(spf_f, 48);
    UNSET(spf_f, 49);
    UNSET(spf_f, 51);
    UNSET(spf_f, 52);

    //让胜
    uint64_t rqspf_s = MASK54;
    UNSET(rqspf_s, 4);
    UNSET(rqspf_s, 5);
    if (rq < 0){
        UNSET(rqspf_s, 1);
        UNSET(rqspf_s, 2);
        for (i=19; i<37; ++i){
            UNSET(rqspf_s, i);
        }

        //分差要大于rq数(排除其他比分)
        for (i=0; i<12; ++i){
            if (sbf[i][0] - sbf[i][1] <= -rq){
                UNSET(rqspf_s, i+6);
            }
        }

        // 进球数要大于rq数(排除7+)
        for (i=37; i<44; ++i){
            if ((i-37) <= -rq){
                UNSET(rqspf_s, i);
            }
        }

        UNSET(rqspf_s, 46);
        UNSET(rqspf_s, 47);
        UNSET(rqspf_s, 49);
        UNSET(rqspf_s, 50);
        UNSET(rqspf_s, 52);
        UNSET(rqspf_s, 53);
    }
    else if (rq > 0){
        // 受1球胜，则不可能为负
        if (rq == 1){
            UNSET(rqspf_s, 2);
            for (i=24; i<37; ++i){
                UNSET(rqspf_s, i);
            }
            UNSET(rqspf_s, 47);
            UNSET(rqspf_s, 50);
            UNSET(rqspf_s, 53);
        }
    }

    //让平
    uint64_t rqspf_p = MASK54;
    UNSET(rqspf_p, 3);
    UNSET(rqspf_p, 5);
    if (rq < 0){
        UNSET(rqspf_p, 1);
        UNSET(rqspf_p, 2);
        for (i=19; i<37; ++i){
            UNSET(rqspf_p, i);
        }

        // 分差要等于rq数(排除其他比分)
        int flag[7] = {0};
        for (i=0; i<12; ++i){
            j = sbf[i][0] - sbf[i][1];
            k = sbf[i][0] + sbf[i][1];
            if (j != -rq){
                UNSET(rqspf_p, i+6);
            }
            else{
                if (k<7){
                    flag[k] = 1;
                }
            }
        }
        for (i=0; i<7; ++i){
            if (flag[i] != 1){
                UNSET(rqspf_p, i+37);
            }
        }

        // 进球数要大于或等于rq数(排除7+)
        for (i=37; i<44; ++i){
            if ((i-37) < -rq){
                uint64_t sentry = 0ULL;
                SET(sentry,i);
                if (rqspf_p & sentry)
                    UNSET(rqspf_p, i);
            }
        }

        UNSET(rqspf_p, 46);
        UNSET(rqspf_p, 47);
        UNSET(rqspf_p, 49);
        UNSET(rqspf_p, 50);
        UNSET(rqspf_p, 52);
        UNSET(rqspf_p, 53);
    }
    else if (rq > 0){
        UNSET(rqspf_p, 0);
        UNSET(rqspf_p, 1);

         for (i=6; i<24; ++i){
            UNSET(rqspf_p, i);
        }

        // 分差要等于rq数(排除其他比分)
        int flag[7] = {0};
        for (i=0; i<12; ++i){
            j = fbf[i][1] - fbf[i][0];
            k = fbf[i][1] + fbf[i][0];
            if (j != rq){
                UNSET(rqspf_p, i+24);
            }
            else{
                if (k<7){
                    flag[k] = 1;
                }
            }
        }
        for (i=0; i<7; ++i){
            if (flag[i] != 1){
                UNSET(rqspf_p, i+37);
            }
        }

        // 进球数要大于或等于rq数(排除7+)
        for (i=37; i<44; ++i){
            if ((i-37) < rq){
                uint64_t sentry = 0ULL;
                SET(sentry,i);
                if (rqspf_p & sentry)
                    UNSET(rqspf_p, i);
            }
        }

        UNSET(rqspf_p, 45);
        UNSET(rqspf_p, 46);
        UNSET(rqspf_p, 48);
        UNSET(rqspf_p, 59);
        UNSET(rqspf_p, 51);
        UNSET(rqspf_p, 52);
    }

    //让负
    uint64_t rqspf_f = MASK54;
    UNSET(rqspf_f, 3);
    UNSET(rqspf_f, 4);
    if (rq > 0){
        UNSET(rqspf_f, 0);
        UNSET(rqspf_f, 1);
        for (i=6; i<24; ++i){
            UNSET(rqspf_f, i);
        }

        //分差要大于rq数(排除其他比分)
        for (i=0; i<12; ++i){
            if (fbf[i][1] - fbf[i][0] <= rq){
                UNSET(rqspf_f, i+24);
            }
        }

        // 进球数要大于rq数(排除7+)
        for (i=37; i<44; ++i){
            if ((i-37) <= rq){
                UNSET(rqspf_f, i);
            }
        }

        UNSET(rqspf_f, 45);
        UNSET(rqspf_f, 46);
        UNSET(rqspf_f, 48);
        UNSET(rqspf_f, 59);
        UNSET(rqspf_f, 51);
        UNSET(rqspf_f, 52);
    }
    else if (rq < 0){
        if (rq == -1){ // 让1球负，则不可能为胜
            UNSET(rqspf_f, 0);
            for (i=6; i<18; ++i){
                UNSET(rqspf_f, i);
            }
            UNSET(rqspf_s, 45);
            UNSET(rqspf_s, 48);
            UNSET(rqspf_s, 51);
        }
    }

    uint64_t bf[31];
    for (i=0; i<31; ++i){
        bf[i] = MASK54;
        for (j=0; j<31; ++j){
            if (i!=j){
                UNSET(bf[i], j+6);
            }
        }
    }

    // 胜比分
    for (i=0; i<13; ++i){
        UNSET(bf[i], 1);
        UNSET(bf[i], 2);
    }
    sbf_rqspf_jq_bqc(&bf[0], rq);

    // 平比分
    for (i=13; i<18; ++i){
        UNSET(bf[i], 0);
        UNSET(bf[i], 2);
    }
    pbf_rqspf_jq_bqc(&bf[13], rq);

    // 负比分
    for (i=18; i<31; ++i){
        UNSET(bf[i], 0);
        UNSET(bf[i], 1);
    }
    fbf_rqspf_jq_bqc(&bf[18], rq);

    // 进球
    uint64_t jq[8];
    for (i=0; i<8; ++i){
        jq[i] = MASK54;
        for (j=0; j<8; ++j){
            if (i!=j){
                UNSET(jq[i], j+37);
            }
        }

        //进球为0，不可能分胜负
        if (i==0){
            UNSET(jq[i], 0);
            UNSET(jq[i], 2);
            UNSET(jq[i], 45);
            UNSET(jq[i], 46);
            UNSET(jq[i], 47);
            UNSET(jq[i], 48);
            UNSET(jq[i], 50);
            UNSET(jq[i], 51);
            UNSET(jq[i], 52);
            UNSET(jq[i], 53);

            // 不可能让平
            UNSET(jq[i], 4);
            if (rq > 0){
                UNSET(jq[i], 5);
            }
            else if (rq < 0){
                UNSET(jq[i], 3);
            }
        }
        else{
            // 让球+进球=奇数，不可能让平
            if (rq > 0){
                if ((i+rq) & 1){
                    UNSET(jq[i], 4);
                }
            }
            else if (rq < 0){
                if ((i-rq) & 1){
                    UNSET(jq[i], 4);
                }
            }
        }

        //进球为奇，不可能为平局
        if (i & 1){
            UNSET(jq[i], 1);
            UNSET(jq[i], 46);
            UNSET(jq[i], 49);
            UNSET(jq[i], 52);
        }

        //比分之和必须与进球相等
        for (j=0; j<13; ++j){
            if (sbf[j][0] + sbf[j][1] != i){
                UNSET(jq[i], j+6);
            }
            if (j < 5 && pbf[j][0] + pbf[j][1] != i){
                UNSET(jq[i], j+19);
            }
            if (fbf[j][0] + fbf[j][1] != i){
                UNSET(jq[i], j+24);
            }
        }
    }

    // 半全场
    uint64_t bqc[9];
    for (i=0; i<9; ++i){
        bqc[i] = MASK54;
        for (j=0; j<9; ++j){
            if (i!=j){
                UNSET(bqc[i], j+45);
            }
        }

        if (i%3 == 0){ // 整场胜
            UNSET(bqc[i], 1);
            UNSET(bqc[i], 2);

            if (rq > 0) {
                UNSET(bqc[i], 4);
                UNSET(bqc[i], 5);
            }

            for (j=19; j<37; ++j){
                UNSET(bqc[i], j);
            }

            UNSET(bqc[i], 37);

            if(i==6){ // 半场负，则客队必有进球
                for (j=0; j<13; ++j){
                    if (j != 12 && sbf[j][1] == 0){
                        UNSET(bqc[i], j+6);
                    }
                }
            }
        }
        else if (i%3 == 1){ // 整场平
            UNSET(bqc[i], 0);
            UNSET(bqc[i], 2);

            if (rq < 0){
                UNSET(bqc[i], 3);
                UNSET(bqc[i], 4);
            }
            else if (rq > 0){
                UNSET(bqc[i], 4);
                UNSET(bqc[i], 5);
            }

            for (j=6; j<19; ++j){
                UNSET(bqc[i], j);
            }
            for (j=24; j<37; ++j){
                UNSET(bqc[i], j);
            }

            if (i==1 || i==7){ // 半场有胜负，则全场必有进球
                UNSET(bqc[i], 19);
                UNSET(bqc[i], 37);
            }
        }
        else{               // 整场负
            UNSET(bqc[i], 0);
            UNSET(bqc[i], 1);

            if (rq < 0){
                UNSET(bqc[i], 3);
                UNSET(bqc[i], 4);
            }

            for (j=6; j<24; ++j){
                UNSET(bqc[i], j);
            }

            UNSET(bqc[i], 37);

            if (i==2){ // 半场胜，则主队必有进球
                for (j=0; j<13; ++j){
                    if (j != 12 && fbf[j][0] == 0){
                        UNSET(bqc[i], j+24);
                    }
                }
            }
        }
    }

    bitmap[0] = spf_s;
    bitmap[1] = spf_p;
    bitmap[2] = spf_f;
    bitmap[3] = rqspf_s;
    bitmap[4] = rqspf_p;
    bitmap[5] = rqspf_f;
    for (i=0; i<31; ++i){
        bitmap[6+i] = bf[i];
    }
    for (i=0; i<8; ++i){
        bitmap[37+i] = jq[i];
    }
    for (i=0; i<9; ++i){
        bitmap[45+i] = bqc[i];
    }
}

/*
int main()
{
    uint64_t res[54];
    make_bitmap(&res[0], -1);
    for (i=0; i<54; ++i){
        printf("mutex with [%s]:", playname[i]);
        for (j=0; j<54; ++j){
            if (i!=j){
                if (test_mutex(res[i],res[j])==0)
                    printf("%s ", playname[j]);
            }
        }
        printf("\n");
    }
    return 0;
}
*/
