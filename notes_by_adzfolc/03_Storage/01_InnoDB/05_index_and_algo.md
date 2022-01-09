# 05_index_and_algo

* ToDo: 6.830 写DB
* ToDo: sqlite 再看看存储模块
* ToDo: Modern B-Tree Techniques

1. 聚簇索引
    * 聚簇索引==主键索引
    * 聚簇索引是逻辑连续的,在物理存储上可能是不连续的.通过链式结构实现.
        * 页通过双向链表连接,页按照主键的顺序排序.(在 InnoDB 中,所有的数据都在 B+ Tree 的叶子节点.)
        * 每个页的记录都通过双向链表维护,物理存储上不按照主键存储.
    * 优:按照主键查询非常快
    * 稠密索引
2. 非聚簇索引
    * 非聚簇索引==二级索引
    * 叶子节点包含 键值,以及指向主键的指针
    * 稀疏索引
3. 如果表上存在很多索引,大批量 DML 操作会对所有索引进行修改.
4. Cardinality
    * 表示索引中不重复记录数量的预估值
    * 对于 Cardinality 更新触发
        * 表中 1/16 的数据已经发生变化
        * stat_modified_counter > 2 000 000 000
    * 更新策略
        1. 取得 B+ 树索引中叶子节点的数量,记为 A
        2. 随机取得 B+ 树索引中8个叶子节点,统计每个页不同记录的个数,记为 P1,P2,...,P8
        3. 根据采样信息给出预估值, Cardinality=(P1+P2+...+P8)*A/8
5. 参数

    |参数|说明|默认|
    |-|-|-|
    |innodb_stats_persistent|是否将 Cardinality 持久化到磁盘|OFF|
    |innodb_stats_on_metadata|Show Table Index,Show Index,访问Information_Schema.Tables,访问Information_Schema.Statics 是否触发重新计算|OFF|
    |innodb_stats_persistent_sample_pages|当innodb_stats_persistent=on,ANALYZE TABLE更新 Cardinality 值时每次采样的数量|20|
    |innodb_stats_traisent_sample_pages|取代innodb_stats_persistent_sample_pages|20|

6. 联合索引
    * 对多个列进行索引
    * Using filesort -> 需要额外的排序操作才能完成查询
    * 最左原则匹配,因为联合索引按照(a,b,c)的顺序进行排序

7. 覆盖索引
    * 索引中包含主键信息,不需要二次回表

8. 索引提示
    * FORCE INDEX 强制索引
    * USE INDEX 建议索引

9. Multi-Range Read 优化
    * Why? -> 减少磁盘的随机读
    * 优:
        * 对于 range, ref, eq_ref 查询:
            * MRR 使数据访问变得较为顺序.在查询辅助索引时,先根据查到的结果对主键排序,并按照主键的顺序进行书签查找.
            * 减少缓冲池中页被替换的次数.
            * 批量处理对键值的查询操作.
        * 对于 InnoDB, MyISAM 的范围查询 和 Join 查询
            * 将查询得到的辅助索引键值放在一个缓存中,缓冲中数据根据辅助索引键值排序
            * 将缓存中的键值根据 RowID 进行排序
            * 根据 RowID 的排序顺序来访问实际的数据文件
        
10. Index Condition Pushdown(ICP) 优化
    * ICP 优化支持 range, ref, eq_ref, ref_or_null 类型查询
    * How? -> 取出索引的同时,判断是否可以进行 where 条件的过滤.部分 where 条件的过滤放到存储引擎层.

11. 对于不包含主键/唯一索引的表, InnoDB 自动创建 隐藏列 GEN_CLUST_INDEX(包含 row ID 值).表中数据按照 rowID 排序.