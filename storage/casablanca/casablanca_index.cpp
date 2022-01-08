/**
 * @file casablanca_index.cpp
 * @author adzfolc
 * @brief 该类是索引文件读取类,索引文件结构 @see Casablanca_index::CAS_IDX_NODE 结构体的二进制存储.索引大小在构建类时指定.
 * @version 0.1
 * @date 2022-01-08
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include "casablanca_index.h"
#include <my_dir.h>

// 构造函数接收所有索引的最大长度作为参数
Casablanca_index::Casablanca_index(int keyLen) { 
  root = nullptr;
  crashed = false;
  max_key_len = keyLen;
  index_file = -1;
  block_size = max_key_len + sizeof(long long) + sizeof(int);
}

// constructor (overloaded) assumes existing file
Casablanca_index::Casablanca_index() {
  root = nullptr;
  crashed = false;
  max_key_len = keyLen;
  index_file = -1;
  block_size = -1;
}

// destructor
Casablanca_index::~Casablanca_index(void) {

}

// create the index file
int Casablanca_index::create_index(char *path, int keyLen) {
  DBUG_ENTER("Casablanca_index::create_index");
  open_index(path);
  max_key_len = keyLen;

  // Block size is the key length plus the size of the index length varaible.
  block_size = max_key_len + sizeof(long long);
  write_header();
  DBUG_RETURN(0);
}

// open index specified as path (pat+filename)
int Casablanca_index::open_index(char *path) {
  DBUG_ENTER("Casablanca_index::open_index");
  /**
   * @brief Open the file with read/write mode,
   *    create the file if not found,
   *    treat file as binary, ans use the default flags.
   */
  index_file = my_open(path, O_RDWR | O_CREAT | O_BINARY | O_SHARE, MYF(0));
  if (index_file == -1) 
    DBUG_RETURN(errno);
  read_header();
  DBUG_RETURN(0);
}

// 读取文件头部信息
int Casablanca_index::read_header() { 
  int i;
  byte len;
  DBUG_ENTER("Casablanca_index::read_header");
  if (block_size == -1) {
      /*
       * Seek the start of the file.
       * Read the maximum key length value.
       */
      my_seek(index_file, 0L, MY_SEEK_CET, MYF(0));
      i = my_read(index_file, &len, sizeof(int), MYF(0));
      memcpy(&max_key_len, &len, sizeof(int));

      /*
       * Calculate block size as maximum key length plus
       * the size of the key plus crashed status byte. 
       */
      block_size = max_key_len + sizeof(long long) + sizeof(int);
      i = my_read(index_file, &len, sizeof(bool), MYF(0));
      memcpy(&crashed, &len, sizeof(bool));
  } else {
      i = (int)my_seek(index_file, sizeof(int) + sizeof(bool), MY_SEEK_SET,
                     MYF(0));
  }
  DBUG_RETURN(0);
}

P230