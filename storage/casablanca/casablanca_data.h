/**
 * @file casablanca_data.h
 * @author adzfolc
 * @brief build a wheel Storage engine, just called casablanca
 * @version 0.1
 * @date 2022-01-08
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#pragma once
#pragma unmanaged

#include "my_sys.h"
class Casablanca_data {
  Casablanca_data(void);
  ~Casablanca_data(void);
  int create_table(char *path);
  int open_table(char *path);
  long long write_row(byte *buf, int length);
  long long upadte_row(byte *old_rec, byte *new_rec, int length,
                       long long position);
  int read_row(byte *buf, int length, long long position);
  int delete_row(byte *old_rec, int length, long long position);
  int close_table();
  long long cur_position();
  int records();
  int del_records();
  int trunc_table();
  int row_size(int length);
private:
  File data_file;
  int header_size;
  int record_header_size;
  bool crashed;
  int number_records;
  int number_del_records;
  int read_header();
  int write_header();
};