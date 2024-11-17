seidel-2d算子优化记录  
规模n=2000，step=2000

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

- 前缀和：20.4s
  - cppflag:-O3
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
              v[0] = A[i - 1][j*8] + A[i - 1][j*8+1] + A[i - 1][j*8+2] + A[i][j*8] + A[i][j*8+1] + A[i][j*8+2] + A[i + 1][j*8] + A[i + 1][j*8+1] + A[i + 1][j*8+2];
              v[0]*=const1[0];
              for(int k=1;k<8;k++){
                  v[k] = A[i - 1][j*8+k] + A[i - 1][j*8+1+k] + A[i - 1][j*8+2+k] + A[i][j*8+1+k] + A[i][j*8+2+k] + A[i + 1][j*8+k] + A[i + 1][j*8+1+k] + A[i + 1][j*8+2+k];
                  v[k]*=const1[k];
              }
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
