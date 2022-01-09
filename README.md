# MySQL 8.0.27 Review by adzfolc

## Notifications
* In this doc, I will make the rules of my code review and period summary for later review.
* This chapter may cotain comprehensive summary of MySQL/Innodb/MGR.
* Review format
    1. My Code review would be written in source code in format of annonation.
    2. Some key annonations would be begined with `__adzfolc__`.
    3. Some questions,which make me confused, would be denoted with `__adzfolc__ Q:`
    4. Some optimizaton would be denoted with `__adzfolc__ To Do: refractor`.

## Keypoint
1. Server
    1. Thread model
    2. MySQL Packet Decode
2. SQL
    1. Parser
    2. Optimizer
        1. SQL Plan
        2. SQL Optimizer
    3. Execution
3. InnoDB
    1. Transaction
        1. How ACID is implmented?
        2. GTID
        3. Locks
        4. Logs
            1. Undolog
            2. Redolog
    2. Binlog
4. Replication
    1. MGR
        1. Basic Paxos and Mutli Paxos
        2. Multi Paxos and Mencius
        3. How Mencius is implemented in MGR
5. Storage
    1. B+ Tree

## Notes
* 00_Summary  
[00_summary](./notes_by_adzfolc/00_summary.md)
* 01_Server  
[00_server_mmary](./notes_by_adzfolc/01_Server/00_server_summary.md)  
[01_thread_model](./notes_by_adzfolc/01_Server/01_thread_model.md)  
[02_memory_management](./notes_by_adzfolc/01_Server/02_memory_management.md)  
* 02_sql  
[00_sql_parser_and_optimizer](./notes_by_adzfolc/02_SQL/00_sql_parser_and_optimizer.md)
* 03_storage  
[00_storage_layer_intro](./notes_by_adzfolc/03_Storage/00_storage_layer_intro.md)  
[01_mysql_meta_data](./notes_by_adzfolc/03_Storage/01_mysql_meta_data.md)  
    * 01_InnoDB  
    [00_sumary](./notes_by_adzfolc/03_Storage/01_InnoDB/00_summary.md)  
    [01_buffer](./notes_by_adzfolc/03_Storage/01_InnoDB/01_buffer.md)  
    [02_file](./notes_by_adzfolc/03_Storage/01_InnoDB/02_file.md)  
    [04_table](./notes_by_adzfolc/03_Storage/01_InnoDB/04_table.md)  
    [05_index_and_algo](./notes_by_adzfolc/03_Storage/01_InnoDB/05_index_and_algo.md)  
    [06_lock_and_latch](./notes_by_adzfolc/03_Storage/01_InnoDB/06_lock_and_latch.md)
* 99_QA  
[99_QA](./notes_by_adzfolc/99_QA.md)

## Expandsion
* Based on tourtial, I implement a storage engine called Casablanca.Source code path is storage/casablanca.
* 基于教程,我实现了名为 Casablanca 的存储引擎.代码路径在 storage/casablanca.
* Ability
    * CRUD
    * Index
* To Do:
    * 优化索引结构

* **Casablanca is still under construction.**

## To Do List
1. MySQL核心内幕
    1. Chp 7 查询解析与优化器
    2. Chp 8 安全管理系统
    3. P230
2. MySQL运维内参
3. MySQL技术内幕-InnoDB存储引擎
4. MySQL内核-INNODB存储引擎 
5. 高性能MySQL
6. Introducing InnoDB Cluster Learning the MySQL High Availability Stack by Charles Bell
7. Expert MySQL
8. 深入理解MySQL核心技术/Understanding MySQL Internals Discovering and Improving a Great Database
9. MySQL是怎样运行的
10. MySQL Concurrency Locking and Transactions for MySQL Developers and DBAs
11. MySQL Group Replication_ The Magic Explained v1/v2
12. 数据库查询优化器的艺术 原理解析与SQL性能优化 (SQL 可看可不看...)