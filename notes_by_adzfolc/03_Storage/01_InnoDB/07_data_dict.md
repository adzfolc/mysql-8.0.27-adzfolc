# 07_data_dict

1. InnoDB 最基本的系统表,用来存储用户定义的 表/列/索引/索引列 等信息,分别是 SYS_TABLES/SYS_COLUMNS/SYS_INDEXED/SS_FIELDS.
    * SYS_TABLES -> 存储所有以 InnoDB 为引擎的表,每条记录对应已经定义的一个表(8.0中改名 INNODB_TABLES )
        * |Column|Intro|
            |-|-|
            |NAME|表名|
            |ID|表的ID(8bytes)|
            |N_COLS|表的列数(4bytes)|
            |TYPE|表的存储类型,记录的格数/压缩等信息(4bytes)|
            |SPACE|表所在的表空间ID(4bytes)|
            |Primary Key|(NAME)|
            |Unique Index|(ID)|
    * SYS_COLUMNS -> 存储 InnoDB 中定义的所有表的列信息,每一条对应这个表中的一条记录(8.0中改名 INNODB_COLUMNS )
        * |Column|Intro|
            |-|-|
            |TABLE_ID|列所属的表ID(8bytes)|
            |POS|列在表中第几列(4bytes)|
            |NAME|列的列名|
            |MTYPE|列的主数据类型(4bytes)|
            |PRTYPE|列的精确数据类型,组合值(NULL标志,是否有符号数的标志,是否是二进制字符串的标志,列是否 varchar )(2bytes)|
            |LEN|列的数据长度,不包括 varchar 类型,因为 varchar 在类型中存储了长度|
            |PREC|列数据的精度,未使用(4bytes)|
            |Primary Key|(TABLE_ID,POS)|
    * SYS_INDEXES -> 存储 InnoDB 中所有表的所有索引信息,每条记录对应一个索引.(8.0中改名 INNODB_INDEXES )
        * |Column|Intro|
            |-|-|
            |TABLE_ID|索引所属的表ID(8bytes)|
            |ID|索引的索引ID(8bytes)|
            |NAME|索引的索引名|
            |N_FIELDS|索引包含的列数(4bytes)|
            |TYPE|索引的类型,包含 聚簇索引/唯一索引/DICT_UNIVERSAL/DICT_IBUF(插入缓冲区 B+ 树)(4bytes)|
            |SPACE|索引数据所在的表空间 ID 号(4bytes)|
            |PAGE_NO|索引对应的 B+ 树的根页面(4bytes)|
            |Primary Key|(TABLE_ID,ID)|
    * SYS_FIELDS -> 存储所有索引中定义的索引列,每条记录对应一个索引列.(8.0中改名 INNODB_FIELDS )
        *   |Column|Intro|
            |-|-|
            |INDEX_ID|列所在的索引(8bytes)|
            |POS|列在某个索引中是第几个索引列(4bytes)|
            |COL_NAME|索引列的列名|
            |Primary Key|(INDEX_ID,POS)|

2. 数据字典表的加载
    * 数据字典表根页面位置的存储方式, InnoDB 采用了一个专门的页面(0号表空间0号文件7号页面)来管理字典信息,用来存储上面四张表的5个根页面号(有5个索引, SYS_TABLES 有2个索引),以及下一个表的 ID 值(全局变量,创建新表时顺序递增分配,全局唯一),下一个索引 ID,下一个表空间 ID, row id.
    * 上述操作通过 dict_hdr_create 函数完成, btr_create 函数完成索引( INNODB_TABLES\*2,INNODB_COLUMNS\*1,INNODB_INDEXES\*1,INNODB_FIELDS\*1 )创建.
    * InnoDB 完成 B+ 树及一些其他信息初始化操作后,通过函数 @see `dberr_t dict_boot(void)` in [dict_boot](../../../storage/innobase/dict/dict0boot.cc) 加载常驻内存的4给系统表及读取其他信息.
    * 数据字典表保存在一个全局的字典结构中 @see `struct dict_sys_t` in [struct dict_sys_t](../../../storage/innobase/include/dict0dict.h)
    * 普通用户表的加载过程
        1. 当用户访问表 tbl 时,系统会首先从表对象缓存池中查找表的 SHARE 对象  
            -> 找到了               -> 直接从其实例化表对象空间链表拿一个空闲的实例化的表对象使用  
            -> 没有可用的实例化对象  -> 需要重新打开(实例化)表 tbl .在实例化表 tbl 时,需要找到表的字典信息,包括表本身/列信息/索引信息.  
            @see `inline dict_table_t *dict_table_get_low` in [dict0priv.ic](../../../storage/innobase/include/dict0priv.ic)
        2. 加载表信息 ->  
            寻找 SYS_TABLES ,过程与普通表过程一致.先找缓存,找不到缓存再从系统表中加载.找到之后通过 NAME 查询 SYS_TABLES 表( NAME 是主键).从B+树中查找,找不到则报错;找到了,解析表的记录,取出 ID/N_COLS/TYPE/MIX_LEN/SPACE ,根据这些信息创建表的内存对象. ->  
            至此,表自身加载完成.  
            @see `inline dict_table_t *dict_table_get_low(const char *table_name, const std::string *prev_table)` in [dict0priv.ic](../../../storage/innobase/include/dict0priv.ic)
        3. 加载列信息 ->  
            在 InnoDB 中,表的列包含两个部分
            * 用户创建表时指定的列 
            * 系统列(Rowid, Trxid, Rollptr)
                * |系统列|含义|
                    |-|-|
                    |Rowid|记录的行号|
                    |Trxid|这条记录最后一次被修改的事务号,主要用于 MVCC 管理|
                    |Rollptr|回滚指针,指向回滚段|
            * @see `static void dict_load_columns(dict_table_t *table, mem_heap_t *heap)` in [dict0load.cc](../../../storage/innobase/dict/dict0load.cc)
            * @see `static void dict_load_virtual(dict_table_t *table, mem_heap_t *heap)` in [dict0load.cc](../../../storage/innobase/dict/dict0load.cc)
        4. 加载索引信息
            加载索引从 SYS_INDEXES 中查询的,原理与 SYS_COLUMNS 一样. SYS_INDEXES 主键是 (TABLE_ID,ID) ,所以具有相同 TABLE_ID 的索引都是按照 ID 进行排序的.每条记录对应一个索引,需要加载 ID/NAME/N_FIELDS/TYPE/PAGENO/SPACE 等信息. 
            * @see `static dberr_t dict_load_indexes` in [dict0load.cc](../../../storage/innobase/dict/dict0load.cc)

3. Rowid 管理
    * 当表没有定义主键,需要 Rowid 作为聚簇索引列的时候,才会分配给表. Rowid 的管理分配,不是一个表独享一个 ID 空间,而是全局的,所有的表都共享这个 ID 号.
    * 优化:
        1. InnoDB 没分配一个 Rowid ,系统只在内存中 +1,不会修改页面.只有 Rowid 是256的倍数才会写入一次.
        2. 为了防止 Rowid 因为系统崩溃没有即使更新,启动后 dict_boot() 函数会将上次写入的 Rowid 值向上对齐256再加上256.缺点是,可能会跳过很多 id ,导致 Rowid 增长过快.