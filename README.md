# MySQL 8.0.27 Review by adzfolc

## Notifications
* In this doc, will make the rules of my code review and period summary for later review.
* This chapter may cotain comprehensive summary of MySQL/Innodb/MGR.
* Review format
    1. My Code review would be written in source code in format of annonation.
    2. Some key annonations would be begined with `__adzfolc__`.
    3. Some questions,which makes me confused, would be denoted with `__adzfolc__ Q:`
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
    2. Log System
    3. Storage
4. Replication
    1. MGR
        1. Paxos Analysis
        2. Paxos and Mencius
        3. How Mencius is implemented in MGR

## Transaction
1. How ACID is implmented?
2. Timestamp and GTID
3. Locks
4. Logs
    1. Undolog
    2. Redolog

## Consensus
1. Paxos
    1. Basic Paxos
    2. Multi Paxos
        * Mencius
    3. Egalitarian Paxos
2. Raft
    1. Raft
    2. Multi Raft
    3. Parallel Raft

## Notes
* 00_Summary  
[00_summary](./notes_by_adzfolc/00_summary.md)
* 01_Server  
[00_server_mmary](./notes_by_adzfolc/01_Server/00_server_summary.md)  
[01_thread_model](./notes_by_adzfolc/01_Server/01_thread_model.md)  
[02_memory_management](./notes_by_adzfolc/01_Server/02_memory_management.md)
* 02_sql  
[00_sql_parser_and_optimizer.md](./notes_by_adzfolc/02_SQL/00_sql_parser_and_optimizer.md)
[02_sql_execution_dml_delete](./notes_by_adzfolc/02_sql_execution_dml_delete.md)
* 99_QA  
[99_QA](./notes_by_adzfolc/99_QA.md)

**This project is still under construction.**