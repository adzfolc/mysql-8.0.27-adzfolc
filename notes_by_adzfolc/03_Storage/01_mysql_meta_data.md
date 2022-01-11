# 01_mysql_meta_data

* @see [sql_base](../../sql/sql_base.cc)

1. MySQL 中的表,在磁盘上均有一个 .frm 扩展名的文件与之对应. frm 在所有平台上的格式是一样的. 在 [sql/table.cc](../../sql/table.cc) 文件中包含了 MySQL 对 frm 二进制文件读取的函数代码.