# 06_lock_latch_and_trx

* 看下 MySQL 8.0 Reference Manual_Chp 15.7_InnoDB Locking and Transaction Model
* MySQL技术内幕-InnoDB存储引擎 Chp6 对照着看下

1. lock and latch
    * latch
        * 闩锁,轻量级
        * 在 InnoDB 中有 mutex(互斥锁) rwlock(读写锁)
    * lock
        * 锁定数据库中的对象(表/页/行)
    * ![lock_and_latch](./lock_and_latch.png)

2. 事务中锁信息监控
    * SHOW FULL PROCESSLIST;
    * SHOW ENGINE INNODB STATUS;
    * Information_Schema.Innodb_trx
    * Information_Schema.Innodb_locks
    * Information_Schema.Innodb_lock_waits
    * @see [innodb_trx_fields_info](../../../storage/innobase/handler/i_s.cc)
    * @see [trx0i_s](../../../storage/innobase/trx/trx0i_s.cc)
    * @see [lock_mode 锁的模式(s,x,is,ix,...)](../../../storage/innobase/include/lock0types.h)
    * @see [lock_type 锁的类型(table,record,...)](../../../storage/innobase/include/lock0lock.h)

### InnoDB Locking

#### Shared,Exclusive and Intention Locks
1.  InnoDB 实现了两种锁机制, shared(S) locks 以及 exclusive(X) locks
    * 共享锁(S Lock),读锁
    * 排他锁(X Lock),写锁 `LOCK TABLES ... WRITE    `
    * 意向锁(IS, IX)
        * 意向共享锁(IS Lock),事务想要获得一张表中某几行的共享锁 `SELECT ... FOR SHARE`
        * 意向排他锁(IX Lock),事务想要获得一张表中某几行的排他锁 `SELECT ... FOR UPDATE`
    * 兼容性
    * ![lock_compatition](./lock_compatition.png)
        * Intention Locks 只会全表扫描(例如 `LOCK TABLES ... WRITE`)

#### Record Locks
1. 行锁(单个行记录上的锁),防止其他事务更改正在操作的数据
2. 行锁会锁定索引数据,如果表没有索引,InnoDB[自动创建主键(隐藏列,聚簇索引)](./05_index_and_algo.md)

#### Gap Locks
1. Definition:
    * a lock on a gap between index records
    * a lock on the gap before the first or after the last index record
    * 间隙锁,锁定一个范围,但不包含记录本身
2. InnoDB Gap Locks 是防止其他事务插入间隙.间隙锁可以共存.一个事务采用的间隙锁不会阻止另一个事务在同一间隙上采用间隙锁.共享和排他间隙锁之间没有区别.彼此不冲突,并且执行相同的功能.允许间隙锁冲突的原因是,如果从索引中清除记录,则必须合并由不同事务保留在记录上的间隙锁.
3. 事务隔离级别降低到 READ COMMITTED 可以禁用 Gap Lock.
4. InnoDB 中 where 条件判断后,不匹配的记录会释放 行锁.对于 Update 语句, InnoDB 采用 半一致性读(semi-consistent) , update 会返回最新版本,便于 InnoDB 判断行是否满足 update where conditions.
5. 触发条件: 不通过唯一索引锁定唯一行

#### Next-Key Locks
1. Netxt-Key Locks = Record Lock on the index record + Gap Lock on the gap before the index record
2. InnoDB 对于行级锁,当他搜索扫描索引时,会对遇到的索引添加 共享锁 或 排他锁.行级锁实际是索引记录锁.
3. 通过 Next-Key Locks 防止幻读. If one session has a shared or exclusive lock on record R in an index, another session cannot insert a new index record in the gap immediately before R in the index order.

#### Insert Intention Locks
1. 对于 Insert row 操作, InnoDB 会采用 Insert Intention Locks ,这种锁对于事务并发 insert 相同索引的相同位置时,两个事务之间不会互相等待.

#### AUTO-INC Locks
1. 事务中对于自增列的特殊类型锁. @see Section 15.6.1.6, "AUTO_INCREMENT Handling in InnoDB".

#### Predicate Locks for Spatial Indexes
1. 空间数据(略).

### InnoDB ACID

1. Repeatable Read
    * 对于 Locking Reads(锁定读,select for update/select for share,update,delete),锁取决于SQL使用 具有唯一搜索条件的唯一索引 还是 范围类型搜索条件 .
        * 对于 具有唯一搜索条件的唯一索引 , InnoDB 只会锁定这一行数据,对于前后的区间不会加锁
        * 对于 其他搜索条件, InnoDB 会采用 Gap Locks or Next-Key Locks.
2. Read Committed
    * Each consistent read, even within the same transaction, sets and reads its own fresh snapshot.
    * 每个一致性读的事务(包括同一个事务),都有事务自己的 snapshot (快照).
    * 对于 Locking Reads(锁定读,select for update/select for share,update,delete), InnoDB 只会锁定 index records ,不会升级为 Gap Locks.所以对于被锁数据的相邻位置可以自由 insert . Gap Locks 只会用来检查 外键约束 以及 duplicate-key 约束.
    * RC 可能幻读.
    * RC 的额外影响:
        1. UPDATE/DELETE SQL , InnoDB 持有数据的锁.不符合 where 条件数据的锁不会持有.减少死锁,不可避免.
        2. UPDATE 数据如果已经被锁定, InnoDB 会进行 semi-consistent 读,通过返回该数据的最新版本,由 InnoDB 根据 where 条件判定可见性.如果可见, InnoDB 会加锁 或者 等待获取锁.

3. RR 与 RC 区别
    * 根据文档, RR 对于 DML 会获取 Gap Locks or Next-Key Locks ,获取 x-lock on each row that it reads and does not release any of them. RC 只会获取 x-lock on each row that it reads and releases those for rows that it does not modify (只获取修改行的锁,释放其他). RC Update 采用 semi-consistent read ,判断 Update where 可见性.

4. Read Uncommitted
    * Dirty read

5. Serializable
    * autocommit=false, InnoDB 隐式对所有 select 改写为 select for share.
    * autocommit=true, Select 采用当前 session 的事务.
    * 如果事务只读,并且采用一致性读,则事务可以串行化.

6. Autocommit
    * 注意 set autocommit=false 隐式提交上个事务.

7. Consistent Nonlocking Reads(一致性非锁定读)
    * MVCC , RC 每次获取最新的快照, RR 的快照在事务开始时确定.

8. Locking Reads
    * SELECT ... FOR SHARE
    * SELECT ... FOR UPDATE
    * NOWAIT -> A locking read that uses NOWAIT never waits to acquire a row lock. The query executes immediately, failing with an error if a requested row is locked.
    * SKIP LOCKED -> A locking read that uses SKIP LOCKED never waits to acquire a row lock. The query executes immediately, removing locked rows from the result set.

9. Locks Set by Different SQL Statements in InnoDB
    1. select ... from -> 非快照隔离,锁定读,没有锁. 快照隔离,对数据 set shared next-key locks .
    2. select ... for update/share -> 使用唯一索引,符合 where 的数据加锁,其他不加锁.
    3. For a unique index with a unique search condition -> InnoDB only locks the indexs record found
    4. For other search conditions, and for non-unique indexes -> InnoDB locks the index range scanned, using gap locks or next-key locks to block insertions by other sessions into the gaps covered by the range
    5. SELECT ... FOR UPDATE 阻塞其他事务.一致性读忽略当前 read view 中记录上的锁.
    6. UPDATE ... WHERE ... 对匹配的数据加 exclusive next-key lock. 但是, 对于 a unique index with a unique search condition , InnoDB 只会添加 index record lock .
    7. 对于主键的 update,需要在相关的二级索引隐式加锁. 对于二级索引 Update , InnoDB 会加 shared locks 进行 duplicate check ,防止 insert .
    8. DELETE FROM ... WHERE ... 对数据加 exclusive next-key lock .但是,对于 using a unique index to search for a unique row 只加 index record lock .
    9. INSERT 只会对插入的记录加写锁(index record lock),而非 next-key lock (在这里,没有 gap lock). INSERT 不会防止其他 session 对相邻的位置 insert .

    ToDo:  MySQL Manual continue from Page 3033 .

## 死锁
1. innodb_lock_wait_timeout 设置超时的时间
2. wait-for-graph(等待图) 进行死锁检测,要求数据库存储 1.锁的信息链表 2.事务等待链表

## 锁升级
1. SQL Server
    1. 单独一个SQL在一个对象上持有的锁数量超过阈值,默认5000.如果不同对象,不会锁升级.
    2. 锁资源占用的内存资源超过激活内存的40%发生锁升级.
2. InnoDB
    1. InnoDB 没有锁升级,对事物采用 bitmap 进行锁管理.不管一个事务锁住一个或多个记录,开销通常一致.

## 事务模型

1. 事务模型比较熟悉了,后面补充  
    ToDo: 补充事务模型
2. 事务的实现,在 InnoDB 中, ACD 通过 redo+undo 完成, redolog 叫做重做日志,保证事务AC; undolog 保证事务一致性.
3. redo/undo 都可以视为一种恢复操作, redo 恢复提交事务的页修改操作, undo 回滚行记录到某个特定版本.
4. redo 物理日志,记录页的物理修改操作. undo 逻辑日志,根据每行记录进行记录.

### redolog

* @see [Redo log constants and functions](../../../storage/innobase/include/log0log.h)
* @see [Redo log types](../../../storage/innobase/include/log0types.h)

1. redolog = redolog buffer(重做日志缓冲 内存 易失) + redolog file(重做日志文件 持久)
2. InnoDB 通过 Force Log at Commit 实现事务的持久性,当事务提交(COMMIT)时,先将事务的重做日志(redolog+undolog)持久化,事务COMMIT才算完成. -> redolog 顺序写 undolog 随机写

3.  |参数|值|意义|
    |-|-|-|
    |innodb_flush_log_at_trx_commit|0|事务提交时不写入 redolog buffer ,由 master thread 定时任务每秒 fsync|
    |innodb_flush_log_at_trx_commit|1|事务提交时 fysnc|
    |innodb_flush_log_at_trx_commit|2|事务提交时将重做日志写入重做文件,写入文件系统缓存,不执行 fsync .依赖OS不宕机.|

4. log block
    1. InnoDB 中, redolog 按照 512bytes 存储 -> redolog buffer/redolog file 按照 block 存储,称为 重做日志块(redolog block)
    2. 磁盘每个页是 512bytes , redolog 需要划分多个 block 存储.
    3. 由于 redolog block 与 磁盘扇区 大小一致,所以 redolog 支持原子写入,不需要 double write .
    4. redolog block = log block header(12bytes) + log block body + log block tailer(8bytes). 每个 redolog block 实际大小是 512-12-8=492bytes
    5. ![redolog_buffer](./redolog_buffer.png)
    6. ![log_block_header](./log_block_header.png)
    7. ![log_block_tailer](./log_block_tailer.png)

5. log group
    1. log group 重做日志组,包含多个重做日志文件. InnoDB 存储引擎只支持一个 log group(源码限制), log group 是逻辑概念,没有实际存储的物理文件表示 log group 信息. log group 由多个重做日志文件组成,每个 log group 中的日志文件大小是相同的.
    2. 重做日志文件存储的是 redolog buffer 中保存的 redolog block(512bytes) ,也是根据块的方式进行物理存储管理.
    3. redolog buffer 刷盘规则
        1. 事务提交时
        2. 当 redolog buffer 中有一半的内存空间被使用
        3. log checkpoint
    4. log block 采用追加写入(append)在 redo log file 的最后部分,当一个 redo log file 被写满时,会接着写入写一个 redo log file ,使用方式为 round-bin.

6. redolog 格式
    1. |redo_log_type|space|page_no|redo log body|
        |-|-|-|-|
        |redo log 类型|表空间 ID|页的偏移量|根据 redo log 类型不同,存储内容不同|
    
7. lsn
    1. lsn = Log Sequence Number ,代表日志序列号
    2. InnoDB 中 LSN 占 8bytes ,且单调递增
    3. LSN含义
        * 重做日志写入的总量
        * checkpoint 的位置
        * 页的版本
8. 恢复

### undolog

* @see [undolog](../../../storage/innobase/srv/srv0srv.cc)

1. 作用
    1. 逻辑日志,存储在表空间中
    2. 回滚
    3. MVCC
    4. undolog 会产生 redolog , undolog 需要持久化

2. 事务提交触发 InnoDB
    * undo log 放入列表,触发 purge 操作
    * 判断 undo log 所在页是否可以重用,若可重用分配给下个事务使用

3. undo log 格式
    * insert undo log - 在提交时可以删除
    * update undo log - 在提交时不可删除,需要提供 MVCC 机制.提交时放入 undo log 链表,等待 purge 线程删除.

4. purge
    * history 列表
    * undo log

5. 参数
    * innodb_purge_batch_size -> 每次 purge 操作需要清理的 undo page 数量
    * innodb_max_purge_lag -> 控制 history list 长度,若 history list 长度超过 innodb_max_purge_lag 延缓 DML 行操作的执行.(delay 行的修改)
        * 延缓算法: delay = ((length(history_list) - innodb_max_purge_lag)*10)-5
    
6. group commit
    * 为了保证 存储引擎层的事务和二进制日志一致性,两者之间采用两阶段事务
    * 2PC
        1. 事务提交时, InnoDB 进行 prepare 操作
        2. MySQL Server 层写入 binlog -> 参数 sync_binlog
        3. InnoDB 存储引起层将日志写入重做日志文件 -> fsync 由参数 innodb_flush_log_at_trx_commit
            1. 修改内存中事务对应的信息,并将日志文件写入 redolog buffer
            2. 调用 fsync 确保日志从 redolog buffer 写入磁盘
    * ![innodb_2pc.png](./innodb_2pc.png)

### binlog
1. 二进制日志,用来进行 POINT-IN-TIME(PIT) 的恢复以及主从复制(Replication).

2. InnoDB 采用 rollback segment ,每个回滚段种记录了1024个 undo log segment, 每个 undo log segment 进行 undo 页的申请.

3. binlog group commit
    * ![binlog group commit](./binlog_group_commit.png)

### redolog and binlog
1. |difference|binlog|redolog|
   |-|-|-|
   |产生|MySQL数据库上层,任何存储引擎的更改都会产生binlog|InnoDB存储引擎层|
   |内容形式|逻辑日志,记录SQL语句|物理日志,记录对每个页的修改|
   |写入|仅在事务提交时写入,事务级唯一|每个事务多条redolog,并发写入|

### 内部 XA 事务