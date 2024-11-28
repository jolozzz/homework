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
  - 如何编译
