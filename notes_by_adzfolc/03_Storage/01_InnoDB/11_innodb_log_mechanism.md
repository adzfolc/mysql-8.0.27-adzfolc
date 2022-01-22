# 11_innodb_log_mechanism

## InnnoDB Buffer Pool

### 背景
1. buffer pool 大小可在文件中配置,由 innodb_buffer_pool_size 决定,默认大小128MB.
2. MySQL 5.7.4 前 buffer pool 启动后不可调整, 5.7.4可根据参数 innodb_buffer_pool_size 动态调整 buffer pool 大小.
3. 若 innodb_buffer_pool_size > 1GB,应通过参数 innodb_buffer_pool_instances=N,将它分成若干 instance ,提升 MySQL 并发请求能力.

### 实现原理

* @see [struct buf_pool_t in buf0buf.h](../../../storage/innobase/include/buf0buf.h)
* @see [buf0buf.cc](../../../storage/innobase/buf/buf0buf.cc)