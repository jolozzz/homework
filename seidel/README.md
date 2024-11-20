seidel-2d算子优化记录  
规模n=2000，step=2000
平台：thxy

- baseline: 109.0s  
  - cppflag:-O0
  ```
  int t, i, j;
    
  for (t = 0; t <= tsteps - 1; t++){
    for (i = 1; i <= n - 2; i++){
      for (j = 1; j <= n - 2; j++){
        A[i][j] = (A[i - 1][j - 1] + A[i - 1][j] + A[i - 1][j + 1] + A[i][j - 1] + A[i][j] + A[i][j + 1] + A[i + 1][j - 1] + A[i + 1][j] + A[i + 1][j + 1]) / SCALAR_VAL(9.0);
      }
    }
  }
  ```

- 前缀和：15.8s
  - cppflag:-O3 （这里O3自向量化了前缀和）(不知道为什么这里用-march=native自动上AVX2会比SSE慢)
  ```
  const double _1_d_9 = 1.0/9.0;
  double const1[8];
  double const2[8];
  double v[8];

  for(int i=0;i<8;i++){
      const1[i]=pow(9,i);
      const2[i]=pow(_1_d_9,i+1);
  }

  for(int t=0;t<tsteps;t++){
      for(int i=1;i<n-1;i++){
          int j=0;
          for(;(j+1)*8<n-2;j++){
              for(int k=0;k<8;k++){
                  v[k] = A[i - 1][j*8+k] + A[i - 1][j*8+1+k] + A[i - 1][j*8+2+k] + A[i][j*8+1+k] + A[i][j*8+2+k] + A[i + 1][j*8+k] + A[i + 1][j*8+1+k] + A[i + 1][j*8+2+k];
                  v[k]*=const1[k];
              }
              v[0]+=A[i][j*8];
              //计算前缀和
              double temp=0;
              for(int k=0;k<8;k++){
                  A[i][j*8+k+1] = (v[k] + temp)*const2[k];
                  temp += v[k];
              }
          }
          for(int jj=j*8+1;jj<=n-2;jj++){
              int rj = jj;
              A[i][rj] = (A[i - 1][rj - 1] + A[i - 1][rj] + A[i - 1][rj + 1] + A[i][rj - 1] + A[i][rj] + A[i][rj + 1] + A[i + 1][rj - 1] + A[i + 1][rj] + A[i + 1][rj + 1]) / SCALAR_VAL(9.0);
          }
      }
  }
  ```
- 前缀和 + avx512：22.0s
  - cppflag： —O3 -mavx512f
  ```
  const double _1_d_9 = 1.0/9.0;
  double const1[8];
  double const2[8];
  double v[8];

  __m512d src = _mm512_set1_pd(0.0);
  __mmask8 musk1 = 0b01010101;
  __mmask8 musk2 = 0b00001111;
  __mmask8 musk3 = 0b00110011;

  for(int i=0;i<8;i++){
      const1[i]=pow(9,i);
      const2[i]=pow(_1_d_9,i+1);
  }

  for(int t=0;t<tsteps;t++){
      for(int i=1;i<n-1;i++){
          int j=0;
          for(;(j+1)*8<n-2;j++){
              v[0] = A[i - 1][j*8] + A[i - 1][j*8+1] + A[i - 1][j*8+2] + A[i][j*8] + A[i][j*8+1] + A[i][j*8+2] + A[i + 1][j*8] + A[i + 1][j*8+1] + A[i + 1][j*8+2];
              v[0]*=const1[0];
              for(int k=1;k<8;k++){
                  v[k] = A[i - 1][j*8+k] + A[i - 1][j*8+1+k] + A[i - 1][j*8+2+k] + A[i][j*8+1+k] + A[i][j*8+2+k] + A[i + 1][j*8+k] + A[i + 1][j*8+1+k] + A[i + 1][j*8+2+k];
                  v[k]*=const1[k];
              }
              //计算前缀和
              // double temp=0;
              // for(int k=0;k<8;k++){
              //     A[i][j*8+k+1] = (v[k] + temp)*const2[k];
              //     temp += v[k];
              // }
              __m512d vec_v = _mm512_loadu_pd(v);//vec_v = 1,2,3,4,5,6,7,8
              __m512d vec_t1 = _mm512_permute_pd(vec_v,0b01010101);//vec_t1 = 2,1,4,3,6,5,8,7
              __m512d vec_t2 = _mm512_add_pd(vec_v,vec_t1);//vec_t2 = 12,12,34,34,56,56,78,78
              vec_v = _mm512_mask_add_pd(vec_v,musk1,vec_v,vec_t1);//vec_v = 1,12,3,34,5,56,7,78
              __m512d vec_t3 = _mm512_permutex_pd(vec_t2,musk2);//vec_t3 = 34,34,12,12,78,78,56,56
              __m512d vec_t4 = _mm512_add_pd(vec_t2,vec_t3);//vec_t4 = 1234,1234,1234,1234,5678,5678,5678,5678
              vec_v = _mm512_mask_add_pd(vec_v,musk3,vec_v,vec_t3);//vec_v = 1,12,123,1234,5,56,567,5678
              __m512d vec_t5 = _mm512_shuffle_f64x2(vec_t4,vec_t4,musk2);//vec_t5 = 5678,5678,5678,5678,1234,1234,1234,1234
              vec_v = _mm512_mask_add_pd(vec_v,musk4,vec_v,vec_t5);//vec_v = 1,12,123,1234,12345,123456,1234567,12345678
              vec_v = _mm512_mul_pd(vec_v,vec_const2);
              _mm512_storeu_pd(&A[i][j*8+1],vec_v);
          }
          for(int jj=j*8+1;jj<=n-2;jj++){
              int rj = jj;
              A[i][rj] = (A[i - 1][rj - 1] + A[i - 1][rj] + A[i - 1][rj + 1] + A[i][rj - 1] + A[i][rj] + A[i][rj + 1] + A[i + 1][rj - 1] + A[i + 1][rj] + A[i + 1][rj + 1]) / SCALAR_VAL(9.0);
          }
      }
  }
  ```
