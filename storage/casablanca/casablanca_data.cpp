/**
 * @file casablanca_data.cpp
 * @author adzfolc
 * @brief
 * 这个类实现了数据的简单读取,同时还可用指定读写数据的大小;允许可变长度的记录,也包括
 * BLOBs 这类数据类型在内.数据是以非压缩,非优化的形式存储.
 * @version 0.1
 * @date 2022-01-08
 *
 * @copyright Copyright (c) 2022
 *
 */

#include "casablanca_data.h"
#include "my_dir.h"

Casablanca_data::Casablanca_data(void) {
  data_file = -1;
  number_records = -1;
  number_del_records = -1;
  header_size = sizeof(bool) + sizeof(int) + sizeof(int);
  record_header_size = sizeof(byte) + sizeof(int);
}

Casablanca_data::~Casablanca_data(void) {

}

// 生成数据文件
int Casablanca_data::create_table(char *path) {
  DBUG_ENTER("Casablanca_data::create_table");
  open_table(path);
  number_records = 0;
  number_del_records = 0;
  crashed = false;
  write_header();
  DBUG_RETURN(0);
}

// 在路径名+文件名这样的路径下面,打开表
int Casablanca_data::open_table(char *path) {
  DBUG_ENTER("Casablanca_data::open_table");
  // 以读或写的模式打开,如果不存在,就创建一个文件,该文件为二进制文件,并使用默认标记
  data_file = my_open(path, O_RDWR | O_CREATE | O_BINARY | O_SHARE, MYF(0));
  if (data_file == -1) 
    DBUG_RETURN(errno);
  read_header();
  DBUG_RETURN(0);
}

// 向文件中写入一条长度为 length 字节的记录,同时返回该位置
long long Casablanca_data::write_row(byte *buf, int length) { 
  long long pos;
  int i;
  int len;
  byte deleted = 0;
  DBUG_ENTER("Casablanca_data::write_row");
  /**
   * 记录下被删除记录的状态字节和长度
   * 提示: 函数 my_write() 返回写入成功字节数 或 -1作为错误标志
   * */
  pos = my_seek(data_file, 0L, MY_SEEK_END, MYF(0));
  
  /**
   * 提示: 函数 my_malloc() 用来分配内存空间,如果返回分配的字节数小于等于0表示错误
   */
  i = my_write(data_file, &deleted, sizeof(byte), MYF(0));
  memcpy(&len, &length, sizeof(int));
  i = my_write(data_file, (byte *)&len, sizeof(int), MYF(0));

  /**
   * 向文件写入行数据,返回 新文件的指针 或是 如果之前的函数 my_write() 返回错误,则返回 -1.
   */
  i = my_write(data_file, buf, length, MYF(0));
  if (i == -1) 
    pos = i;
  else
    number_records++;
  DBUG_RETURN(pos);
}

// 在正确的地方更新记录
long long Casablanca_data::upadte_row(byte *old_rec, byte *new_rec, int length, long long position) {
  long long pos;
  long long cur_pos;
  byte *cmp_rec;
  int len;
  byte deleted = 0;
  int i = -1;
  DBUG_ENETR("Casablanca_data::upadte_row");
  if (position == 0) 
    // 移动过去的头的位置
    position = header_size;
  pos = position;
  // 如果记录的位置未知,每次通过读一行来扫描记录直到找到
  if (position == -1) // don't know where it is ... scan for it
  {
    cmp_rec = (byte *)my_malloc(length, MYF(MY_ZEROFILL | MY_WME));
    pos = 0;
    // 提示: 函数 my_seek() 如果正常则返回记录位置,如果有错误则返回-1
    cur_pos = my_seek(data_file, header_size, MY_SEEK_SET, MYF(0));
    // 提示: 函数 read_row() 正常情况下返回当前文件的指针,如果有错误返回-1
    while ((cur_pos != -1) && (pos != -1))
    {
      pos = read_row(cmp_rec, length, cur_pos);
      if(memcpy(old_rec, cmp_rec, length) == 0) {
        pos = cur_pos;  // 发现当前位置
        cur_pos = -1;   // 完美退出 loop
      } else if(pos != -1) // 移动到下一个记录位置
        cur_pos = cur_pos + length + record_header_size;
    }
    my_free((gptr)cmp_rec, MYF(0));
  }

  // 如果记录的位置被发现或是被提供,写入改行位置
  if(pos != -1) {
      // 在当前文件指针上记录被删除字节,行的长度以及数据
      // 提示: 函数 my_write() 返回被写入的字节 或是 错误返回-1
      my_seek(data_file, pos, MY_SEEK_SET, MYF(0));
      i = my_write(data_file, &deleted, sizeof(byte), MYF(0));
      memcpy(&len, &length, sizeof(int));
      i = my_write(data_file, (byte*)&len, sizeof(int), MYF(0));
      pos = i;
      i = my_write(data_file, new_rec, length, MYF(0));
  }
  DBUG_RETURN(pos);
}

// 在正确的位置删除记录
int Casablanca_data::delete_row(byte *old_rec, int length, long long position) {
  int i = -1;
  long long pos;
  long long cur_pos;
  byte *cmp_rec;
  byte deleted = 1;
  DBUG_ENTER("Casablanca_data::delete_row");
  if (position == 0) 
    position = header_size; // move past header
  pos = position;
  if (position == -1)  // don't know where it is ... scan for it
  {
    cmp_rec = (byte *)my_malloc(length, MYF(MY_ZEROFILL | MY_WME));
    pos = 0;
    cur_pos = my_seek(data_file, header_size, MY_SEEK_SET, MYF(0));
    while ((cur_pos != -1) && (pos != -1)) {
      pos = read_row(cmp_rec, length, cur_pos);
      if (memcpy(old_rec, cmp_rec, length) == 0) {
        number_records--;
        number_del_records++;
        pos = cur_pos;
        cur_pos = -1;
      } else if(pos != -1)
        cur_pos = cur_pos + length + record_header_size;
    }
    my_free((gptr)cmp_rec, MYF(0));
  }
  if (pos != -1)    // 标记为删除
  {
    // 在当前文件指针上记录被删除的字节,通过标志位1表示已被删除
    pos = my_seek(data_file, pos, MY_SEEK_SET, MYF(0));
    i = my_write(data_file, &deleted, sizeof(byte), MYF(0));
    i = (i > 1) ? 0 : 1;
  }
  DBUG_RETURN(i);
}

// 读出文件中一行记录的字节长度
int Casablanca_data::read_row(byte *buf, int length, long long position) {
  int i;
  int rec_len;
  long long pos;
  byte deleted = 2;
  DBUG_ENTER("Casablanca_data::read_row");
  if (position <= 0) 
    position = header_size; // move past header
  pos = my_seek(data_file, position, MY_SEEK_SET, MYF(0));
  // 如果 my_seek 找到这个位置,读出被删除的字节
  if (pos != -1L) {
    i = my_read(data_file, &deleted, sizeof(byte), MYF(0));
    // 如果记录没有被删除,先读取记录的长度,然后再读出记录
    if (deleted == 0)   // 0:代表没有被删除, 1:代表已经被删除
    {
      i = my_read(data_file, (byte *)&rec_len, sizeof(int), MYF(0));
      i = my_read(data_file, buf, (length < rec_len) ? length : rec_len,
                  MYF(0));
    } else if (i == 0)
      DBUG_RETURN(-1);
    else
      DBUG_RETURN(read_row(
          buf, length,
          cur_position() + length + (record_header_size - sizeof(byte))));
    else
      DBUG_RETURN(-1);
  }
  DBUG_RETURN(0);
}

// 关闭文件
int Casablanca_data::close_table() {
  DBUG_ENTER("Casablanca_data::close_table");
  if (data_file != -1) {
    my_close(data_file, MYF(0));
    data_file = -1;
  }
  DBUG_RETURN(0);
}

// 返回记录的数目
int Casablanca_data::records() { 
    DBUG_ENTER("Casablanca_data::records");
    DBUG_RETURN(number_records);
}

// 返回被删除记录的数目
int Casablanca_data::del_records() {
    DBUG_ENTER("Casablanca_data::del_records");
    DBUG_RETURN(number_del_records);
}

// 读出文件的头信息
int Casablanca_data::read_header() { 
  int i;
  int len;
  DBUG_ENETR("Casablanca_data::read_header");
  if (number_records == -1) {
    my_seek(data_file, 0L, MY_SEEK_SET, MYF(0));
    i = my_read(data_file, (byte *)&crashed, sizeof(bool), MYF(0));
    i = my_read(data_file, (byte *)&len, sizeof(int), MYF(0));
    memcpy(&number_records, &len, sizeof(int));
    i = my_read(data_file, (byte *)&len, sizeof(int), MYF(0));
    memcpy(&number_del_records, &len, sizeof(int));
  } else
    my_seek(data_file, header_size, MY_SEEK_SET, MYF(0));
  DBUG_RETURN(0);
}

// 将头信息写入文件
int Casablanca_data::write_header() { 
    int i;
    DBUG_ENETR("Casablanca_data::write_header");
    if (number_records != -1) {
      my_seek(data_file, 0L, MY_SEEK_SET MYF(0));
      i = my_write(data_file, (byte *)&crashed, sizeof(bool), MYF(0));
      i = my_write(data_file, (byte *)&number_records, sizeof(int), MYF(0));
      i = my_write(data_file, (byte *)&number_del_records, sizeof(int), MYF(0));
    }
    DBUG_RETURN(0);
}

// 获得数据文件的位置
long long Casablanca_data::cur_position() { 
  long long pos;
  DBUG_ENTER("Casablanca_data::cur_position");
  pos = my_seek(data_file, 0L, MY_SEEK_CUR, MYF(0));
  if (pos == 0)
    DBUG_RETURN(header_size);
  DBUG_RETURN(pos);
}

// 删除数据文件
int Casablanca_data::trunc_table() {
  DBUG_ENTER("Casablanca_data::trunc_table");
  if (data_file != -1) {
    my_chsize(data_file, 0, 0, MYF(MY_WME));
    write_header();
  }
  DBUG_RETURN(0);
}

// 确定数据文件中行的大小
int Casablanca_data::row_size(int length) {
    DBUG_ENTER("Casablanca_data::row_size");
    DBUG_RETURN(length + record_header_size);
}