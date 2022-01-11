# 04_connection_lifecycle

* @see [01_thread_model](./01_thread_model.md)

1. Server connection 声明周期主要处理 报文格式解析/SQL解析/执行计划生成/...
    ```c++
    // Step 1. 为 handler 分配线程
    File path:  sql/conn_handler/connection_handler_manager.cc
    Function:   void Connection_handler_manager::process_new_connection(
                Channel_info *channel_info)
    Call:       m_connection_handler->add_connection(channel_info)

    // Step 2. conn_hanler 中添加新线程,处理新建连接
    File path:  sql/conn_handler/connection_handler_per_thread.cc
    Function:   bool Per_thread_connection_handler::add_connection(Channel_info *channel_info)
    Call:       mysql_thread_create(key_thread_one_connection, &id, &connection_attrib,
                          handle_connection, (void *)channel_info)
    
    // Step 3.  通过 pthread_create 创建线程,回调函数 handle_connection ,处理任务
    File path:  include/mysql/psi/mysql_thread.h
    Function:   static inline int inline_mysql_thread_create(
                PSI_thread_key key [[maybe_unused]],
                unsigned int sequence_number [[maybe_unused]], my_thread_handle *thread,
                const my_thread_attr_t *attr, my_start_routine start_routine, void *arg)
    Called by:  mysql_thread_create(key_thread_one_connection, &id, &connection_attrib,
                          handle_connection, (void *)channel_info)
        
    // Step 4.  
    File path:  sql/conn_handler/connection_handler_per_thread.cc
    Function:   static void *handle_connection(void *arg)
    Intro:      handle_connection 中一直循环等待线程上的 socket 请求,解析 socket 中 MySQL 报文.根据报文类型,调用不同的 handler 处理.循环直到线程退出.

    // Step 5.1.
    File path:  sql/sql_connect.cc
    Function:   rc = login_connection(thd)
    Intro:      用户首次建连接,负责鉴权.
    Call:       bool thd_prepare_connection(THD *thd) -> 负责鉴权

    // Step 5.2. core method
    File path:  sql/sql_parse.cc
    Function:   do_command(thd)
    Intro:      处理线程的 MySQL 报文
    Call:       bool do_command(THD *thd)
    Call:       int Protocol_classic::get_command(COM_DATA *com_data,
                                  enum_server_command *cmd)
    Call:       bool Protocol_classic::parse_packet(union COM_DATA *data,
                                    enum_server_command cmd)
    Call:       void dispatch_sql_command(THD *thd, Parser_state *parser_state)
    ```