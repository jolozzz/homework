- fortran调用c代码
  - fortran如何调用c
    ```
    interface
      subroutine func_c(x_d,x_d_dim1,x_d_dim2,k_step,j_step,i_step,my_ngrid,temp,my_wgt,wgt_dim1)
        use ISO_C_BINDING
        real(c_double) :: x_d
        integer(c_int) :: x_d_dim1
        integer(c_int) :: x_d_dim2
        integer(c_int) :: k_step
        integer(c_int) :: j_step
        integer(c_int) :: i_step
        integer(c_int) :: my_ngrid
        real(c_double) :: temp
        real(c_double) :: my_wgt
        integer(c_int) :: wgt_dim1 
      end subroutine
    end interface
    ```
    定义一个接口，通过接口传递fortran中变量的指针
    ```
    call func_c(x%d(is,js,ks),x_d_dim1,x_d_dim2,k_step,j_step,i_step,filter%ngrid_lon(js),tmp(is),filter%wgt_lon(1,js),wgt_dim1)
    ```
    直接调用子程序
  - c代码怎么写
    ```
    #include<stdio.h>
    void func_c_(double *x_d,int *x_d_dim1,int *x_d_dim2,
                 int *k_step,int *j_step,int *i_step,
                 int *ngrid,
                 double *temp,
                 double* wgt,int *wgt_dim1){
        //printf("c : %lld  %03.15lE\n",(long long int)(x_d),*(x_d));
    
        int n,hn;
    
        for (int k=0;k<(*k_step);k++){
            for(int j=0;j<(*j_step);j++){
                n = *(ngrid+j);
                if(n>1){
                    hn = (n - 1) / 2;
                    for(int i=0;i<(*i_step);i++){
                        *(temp+i) = 0;
                        for(int h=0;h<n;h++){
                            //*(temp+i) += wgt[j][h]*x_d[k][j][i-hn+h];
                            *(temp+i) += (*(wgt + j*(*wgt_dim1) + h))*(*(x_d + k*(*x_d_dim2)*(*x_d_dim1) + j*(*x_d_dim1) + i+h-hn));
                        }
                    }
                    for(int i=0;i<(*i_step);i++){
                        //x_d[k][j][i] = temp[i];
                        *(x_d + k*(*x_d_dim2)*(*x_d_dim1) + j*(*x_d_dim1) + i) = *(temp+i);
                    }
                }
            }
        }
    }
    ```
  - 如何编译  
    fortran和c的库之间可以直接链接，直接更改cmake使其支持c语言即可（gomars的cmake已经把c编译器检查及使用相关给写好了）
    ```
    target_sources(gmcore PRIVATE
      ...
      src/dynamics/filter_run_2d.c
    )
    ```
    ```
    set(CMAKE_C_COMPILER mpicc)
    set(CMAKE_C_FLAGS "-O3 -xHost")
    ```
- baseline:66s  
  c移植:53s
- 向量化：56s
  ```
  for (int k=0;k<(*k_step);k++){
    for(int j=0;j<(*j_step);j++){
        n = *(ngrid+j);
        if(n>1){
            hn = (n - 1) / 2;
            for(int i=0;i<*i_step;i++){
                int h=0;
                __m512d temp0 = _mm512_setzero_pd();
                for(;h+8<n;h+=8){
                    __m512d temp1 = _mm512_loadu_pd(wgt + j*(*wgt_dim1) + h);
                    __m512d temp2 = _mm512_loadu_pd(x_d + k*(*x_d_dim2)*(*x_d_dim1) + j*(*x_d_dim1) + i+h-hn);
                    temp0 = _mm512_fmadd_pd(temp1,temp2,temp0);
                }
                *(temp+i)=_mm512_reduce_add_pd(temp0);
                for(;h<n;h++){
                    *(temp+i) += (*(wgt + j*(*wgt_dim1) + h))*(*(x_d + k*(*x_d_dim2)*(*x_d_dim1) + j*(*x_d_dim1) + i+h-hn));
                }
            }
            for(int i=0;i<(*i_step);i++){
                *(x_d + k*(*x_d_dim2)*(*x_d_dim1) + j*(*x_d_dim1) + i) = *(temp+i);
            }
        }
    }
  }
  ```
  ①这里采用的方法是沿h进行向量话展开。若沿i展开，因x_d的非连续排布，需要用到_mm512_set_pd这一低性能指令来读取数据，导致速度变慢。
  ②若要进一步提升性能，可以从64位对齐部分开始使用load取数
