seidel-2d算子优化记录  
规模n=2000，step=2000

- baseline: 109.0s
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
