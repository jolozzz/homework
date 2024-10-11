- part A
  - 代码及思路见csim.c
  - 正确性验证
    <img src="./csim验证long.trace.png">
- part B
  - 代码见tran.c
  - 思路：
      
    首先分析32\*32的转置。cache的大小为（5，1，5）,先大致确认分块大小为 8\*8，简单分块的代码如下
    ```
    for(int i = 0;i<N;i+=8){
        for(int j=0;j<M;j+=8){
            for(int ii=i;ii<i+8;ii++){
                for(int jj=j;jj<j+8;jj++){
                    B[jj][ii] = A[ii][jj];
                }
            }
        }
    }
    //cache miss数量为344
    ```
    对于每一个8\*8的分块，理论上的预期的cache miss数量为16，但实际上的数量远超于此，发现是32\*32矩阵转置在处理对角线上的分块时会发生高达36次的cache miss。其原因是在这种情况下，A[i][j\*8:j+8\*8]与B[i][j\*8:j+8\*8]共用同一个set。
    基于此对代码进行改进。
    使用中间变量减少块间set冲突，使对角快的cache miss数量变为22，其余任然为16
    ```
    int temp1,temp2,temp3,temp4,
    temp5,temp6,temp7,temp8;

    for(int i = 0;i<N;i+=8){
        for(int j=0;j<M;j+=8){
            for(int ii=i;ii<i+8;ii++){
                temp1 = A[ii][j];
                temp2 = A[ii][j+1];
                temp3 = A[ii][j+2];
                temp4 = A[ii][j+3];
                temp5 = A[ii][j+4];
                temp6 = A[ii][j+5];
                temp7 = A[ii][j+6];
                temp8 = A[ii][j+7];
                B[j][ii] = temp1;
                B[j+1][ii] = temp2;
                B[j+2][ii] = temp3;
                B[j+3][ii] = temp4;
                B[j+4][ii] = temp5;
                B[j+5][ii] = temp6;
                B[j+6][ii] = temp7;
                B[j+7][ii] = temp8;
            }
        }
    } 
    //cache miss为288
    ```

    对于64/*64的矩阵转置，还需考虑分块内set冲突的问题。使用4\*4的分块可以消除块内set冲突的问题。
    （还想到一种方法可以降低cache miss，有时间我会写下来┑ (￣Д ￣)┍）
    ```
    int temp1,temp2,temp3,temp4;

    for(int i = 0;i<N;i+=4){
        for(int j=0;j<M;j+=4){
            for(int ii=i;ii<i+4;ii++){
                temp1 = A[ii][j];
                temp2 = A[ii][j+1];
                temp3 = A[ii][j+2];
                temp4 = A[ii][j+3];
                B[j][ii] = temp1;
                B[j+1][ii] = temp2;
                B[j+2][ii] = temp3;
                B[j+3][ii] = temp4;
            }
        }
    }
    //cache miss为1700
    ```

    对于61\*67的矩阵转置，直接分块即可达成要求
    ```
    const int blockN = 16;
    const int blockM = 16;

    for(int i=0;i<N;i+=blockN){
        for(int j=0;j<M;j+=blockM){
            for(int ii=i;ii<(N<i+blockN ? N : i+blockN);ii++){
                for(int jj=j;jj<(M<j+blockM ? M : j+blockM);jj++){
                    B[jj][ii] = A[ii][jj];
                }
            }
        }
    }
    //cache miss为1993
    ```
