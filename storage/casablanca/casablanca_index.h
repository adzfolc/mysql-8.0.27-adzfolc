/**
 * @file casablanca_index.h
 * @author adzfolc
 * @brief 该头文件定义了一个简单的索引类,用来存储文件指针索引.这个类保存整个索引在内存中,以利于快速访问.内部内存结构是一个连接列表,但是性能不如 B-Tree.它不能用于大多数的测试环境.允许最大的键长度,这被用于索引中的所有节点.
 * @version 0.1
 * @date 2022-01-08
 * 
 * @copyright Copyright (c) 2022
 * 
 */

/**
 * @brief 接口设计
 * SOF                                  max_key_len     (int)
 * SOF + sizeof(int)                    crashed         (bool)
 * SOF + sizeof(int)+sizeof(bool)       DATA BEGINS HERE
 */


#include "my_sys.h"
const long METADATA_SIZE = sizeof(int) + sizeof(bool);

// 这个节点存储键值和数据行在文件中的位置
struct CAS_INDEX {
  byte *key;
  long long pos;
  int length;
};

// 为内部列表定义连接列表
struct CAS_IDX_NODE {
  CAS_INDEX key_idx;
  CAS_IDX_NODE *prev;
  CAS_IDX_NODE *next;
};

class Casablanca_index {
public:
  Casablanca_index();
  Casablanca_index(int keylen);
  ~Casablanca_index();
  int open_index(char *path);
  int create_index(char *path, int keylength);
  int insert_key(CAS_INDEX *idx, bool allow_dupes);
  int delete_key(byte *buf, long long pos, int key_len);
  long long get_index_pos(byte *buf, int key_len);
  long long get_first_pos();
  byte *get_first_key();
  byte *get_last_key();
  byte *get_prev_key();
  byte *get_next_key();
  int close_index();
  // 将整个索引文件以双向链表的形式加载到内存
  // 因此所有链表查找/插入或反序操作都在内存中进行,大大减少了对硬盘读写的压力
  // called when opening table
  int load_index();
  int destroy_index();
  CAS_INDEX *seek_index(byte *key, int key_len);
  CAS_IDX_NODE *seek_index_pos(byte *key, int key_len);
  // 将内存中的索引结构回写到磁盘
  // called when closing table
  int save_index();
  int trunc_index();

private:
  File index_file;
  int max_key_len;
  // 索引结构根节点
  CAS_IDX_NODE *root;
  CAS_IDX_NODE *range_ptr;
  int block_size();
  bool crashed;
  int read_header();
  int write_header();
  long long write_row(CAS_INDEX *idx);
  CAS_INDEX *read_row(long long position);
  long long curfpos();
};