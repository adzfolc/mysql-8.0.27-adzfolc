# 01_thread_model

* This chapter may cotain summary of code review of server layer.

1. Reactor 线程模型  
初步看下来, MySQL 前端连接是采用了 Multi-Reactor 线程模型,并且 one-loop-one-thread,其中 Main Reactor(Acceptor), Sub Ractor(Listener)

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
    File path: include/mysql/psi/mysql_socket.h
    Function:   static inline MYSQL_SOCKET inline_mysql_socket_accept(
                #ifdef HAVE_PSI_SOCKET_INTERFACE
                    const char *src_file, uint src_line, PSI_socket_key key,
                #endif
                    MYSQL_SOCKET socket_listen, struct sockaddr *addr, socklen_t *addr_len)

    // Step 4.  将 MYSQL_SOCKET 包装成 Channel_info_tcpip_socket
    File path:  sql/conn_handler/socket_connection.cc
    Definition: /**
                This class abstracts the info. about TCP/IP socket mode of communication with
                the server.
                */
                class Channel_info_tcpip_socket : public Channel_info  
    Called:     Channel_info_tcpip_socket *channel_info =
                new (std::nothrow) Channel_info_tcpip_socket(connect_sock, true)

    // Step 5.  由 Connection_handler_manager 处理 socket IO
    File path:  sql/conn_handler/connection_handler_manager.h
    Definition: /**
                This is a singleton class that provides various connection management
                related functionalities, most importantly dispatching new connections
                to the currently active Connection_handler.
                */
                class Connection_handler_manager
    Function:   void Connection_handler_manager::process_new_connection

    // Step 6.  把 channel_info 注册到 Connection_handler_manager
    File path:  sql/conn_handler/connection_handler_per_thread.cc
    Function:   bool Per_thread_connection_handler::add_connection(Channel_info *channel_info)
    Call:       mysql_thread_create(key_thread_one_connection, &id, &connection_attrib,
                          handle_connection, (void *)channel_info)

    // Step 7.  对 channel_info 创建一个线程,即 MySQL 是 one thread loop per connection
    File path:  include/mysql/psi/mysql_thread.h
    Function:   static inline int inline_mysql_thread_create(
                PSI_thread_key key [[maybe_unused]],
                unsigned int sequence_number [[maybe_unused]], my_thread_handle *thread,
                const my_thread_attr_t *attr, my_start_routine start_routine, void *arg)
    Call:       result = my_thread_create(thread, attr, start_routine, arg);
    File path:  int my_thread_create(my_thread_handle *thread, const my_thread_attr_t *attr,
                     my_start_routine func, void *arg)
    Call:       pthread_create(&thread->thread, attr, func, arg);

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