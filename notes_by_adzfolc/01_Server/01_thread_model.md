# 01_thread_model

* This chapter may cotain summary of code review of server layer.

1. Reactor 线程模型  
初步看下来, MySQL 前端连接是采用了 Multi-Reactor 线程模型,其中 Main Reactor(Acceptor), Sub Ractor(Reactor)

2. 初始化
    ```C++
    Listener -> Acceptor  
    @See sql/mysqld.cc static bool network_init  (void)
    ```

3. 运行
    ```C++
    @see
    File path:  sql/mysqld.cc
    Called in:  int mysqld_main(int argc, char **argv)
                mysqld_socket_acceptor->connection_event_loop()
    Line:       7773
    ```

4. 处理新建连接
    1. MySQL 启动后, Acceptor 线程会启动监听连接请求,
    ```C++
    File path:  sql/conn_handler/connection_acceptor.h
    /**
        Connection acceptor loop to accept connections from clients.
    */
    void connection_event_loop() {
        Connection_handler_manager *mgr =
            Connection_handler_manager::get_instance();
        while (!connection_events_loop_aborted()) {
        Channel_info *channel_info = m_listener->listen_for_connection_event();
        if (channel_info != nullptr) mgr->process_new_connection(channel_info);
        }
    }
    ```
    2. Listern 将 Socket 包装成 Channel_info
    ```C++
    // Step 1.  监听等待 Socket 连接事件
    File path:  class Mysqld_socket_listener
                sql/conn_handler/socket_connection.cc
    Function:   Channel_info *Mysqld_socket_listener::listen_for_connection_event()

    // Step 2.  从就绪的 socket 创建新的连接
    File path:  sql/conn_handler/socket_connection.cc
    Function:   static bool accept_connection(MYSQL_SOCKET listen_sock,
                              MYSQL_SOCKET *connect_sock)
    Parameter:  In: listen_sock In&Out: connect_sock

    // Step 3.  调用 accept 建立新连接
    File pathL: include/mysql/psi/mysql_socket.h
    Function:   static inline MYSQL_SOCKET inline_mysql_socket_accept(
                #ifdef HAVE_PSI_SOCKET_INTERFACE
                    const char *src_file, uint src_line, PSI_socket_key key,
                #endif
                    MYSQL_SOCKET socket_listen, struct sockaddr *addr, socklen_t *addr_len)
    ```

99. 代码位置  
    ```C++
    Main Reactor in MySQL ==> Acceptor
    @see  
    File path:  sql/conn_handler/connection_acceptor.h  
    Definition: template <typename Listener>
                class Connection_acceptor
    Sub Reactor in MySQL  ==> Listener
    @see
    File path:  sql/conn_handler/socket_connection.h
    Definition: class Mysqld_socket_listener
    ```