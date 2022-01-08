# 00_sql_parser_and_optimizer

1. MySQL Parser 称为解析器,熟悉SQL Parser的朋友会想到 Druid Parser,笔者之前研究过 Druid Parser,但是对于 MySQL 使用的 Lex 不太熟悉.与编译原理类似,SQL Parser由两个模块组成.
    1. 词法分析(Lexical Analysis or Scanner)
    2. 语法分析(Syntax Analysis or Parser)

2. 其实笔者对于 Server 层没有很大的兴趣,笔者更有志于存储模块,这个模块以后再看吧.