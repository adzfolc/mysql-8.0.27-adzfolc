/*****************************************************************************

Copyright (c) 1994, 2021, Oracle and/or its affiliates.

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License, version 2.0, as published by the
Free Software Foundation.

This program is also distributed with certain software (including but not
limited to OpenSSL) that is licensed under separate terms, as designated in a
particular file or component or in included license documentation. The authors
of MySQL hereby grant you an additional permission to link the program and
your derivative works with the separately licensed software that they have
included with MySQL.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU General Public License, version 2.0,
for more details.

You should have received a copy of the GNU General Public License along with
this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA

*****************************************************************************/

/** @file include/page0types.h
 Index page routines

 Created 2/2/1994 Heikki Tuuri
 *******************************************************/

#ifndef page0types_h
#define page0types_h

#include "dict0types.h"
#include "mtr0types.h"
#include "univ.i"
#include "ut0new.h"

#include <map>

/*			PAGE HEADER
                        ===========

Index page header starts at the first offset left free by the FIL-module */

typedef byte page_header_t;

// 页面头信息从偏移为 38 的位置开始
#define PAGE_HEADER                                  \
  FSEG_PAGE_DATA /* index page header starts at this \
         offset */
/*-----------------------------*/
// 长度 2 偏移 0
// 存储 Slot(槽) 的个数
#define PAGE_N_DIR_SLOTS 0 /* number of slots in page directory */
// 长度 2 偏移 2
// 存储当前页面,还没使用的空间的最小位置
#define PAGE_HEAP_TOP 2    /* pointer to record heap top */
// 长度 2 偏移 4
// 存储当前页面堆管理空间中存储的记录数目,包含 最大记录 最小记录 的管理
#define PAGE_N_HEAP                                      \
  4                    /* number of records in the heap, \
                       bit 15=flag: new-style compact page format */
// 长度 2 偏移 6
// 存储当前页面中,已经被删除的记录所占用的空间组成二点链表首指针
#define PAGE_FREE 6    /* pointer to start of page free record list */
// 长度 2 偏移 8
// 当前页面中已经被删除的记录数 -> 还未被真正 purge 的记录数
// when purge one record -> PAGE_GARBAGE-- PAGE_FREE++
#define PAGE_GARBAGE 8 /* number of bytes in deleted records */
// 长度 2 偏移 10
// 指向当前页面最后被插入的位置
#define PAGE_LAST_INSERT                                                \
  10                      /* pointer to the last inserted record, or    \
                          NULL if this info has been reset by a delete, \
                          for example */
// 长度 2 偏移 12
// 表示上次插入的方向
#define PAGE_DIRECTION 12 /* last insert direction: PAGE_LEFT, ... */
// 长度 2 偏移 14
// 表示以同一个方向连续插入记录的条数
#define PAGE_N_DIRECTION                                            \
  14                   /* number of consecutive inserts to the same \
                       direction */
// 长度 2 偏移 16
// 存储当前页面中存储了多少条记录
#define PAGE_N_RECS 16 /* number of user records on the page */
// 长度 8 偏移 18
// 存储当前页面所有事务中值最大的事务号 -> 只在 二级索引 和 插入缓冲 中定义
#define PAGE_MAX_TRX_ID                             \
  18 /* highest id of a trx which may have modified \
     a record on the page; trx_id_t; defined only   \
     in secondary indexes and in the insert buffer  \
     tree */
#define PAGE_HEADER_PRIV_END                      \
  26 /* end of private data structure of the page \
     header which are set in a page create */
/*----*/
// 长度 2 偏移 26
// 存储当前节点在 B+ Tree 第几层
// 叶子节点 -> 0
#define PAGE_LEVEL                                 \
  26 /* level of the node in an index tree; the    \
     leaf level is the level 0.  This field should \
     not be written to after page creation. */
// 长度 8 偏移 28
// 当前页面属于哪个索引,存储对应的索引 ID 值
#define PAGE_INDEX_ID                          \
  28 /* index id where the page belongs.       \
     This field should not be written to after \
     page creation. */

// 只有根页面的 PAGE_BTR_SEG_LEAF/PAGE_BTR_SEG_TOP 两个变量有意义,在其他页面忽略不计
// 长度 10 偏移 36
// 存储 B+ Tree 叶子段的段头地址
#define PAGE_BTR_SEG_LEAF                         \
  36 /* file segment header for the leaf pages in \
     a B-tree: defined only on the root page of a \
     B-tree, but not in the root of an ibuf tree */
#define PAGE_BTR_IBUF_FREE_LIST PAGE_BTR_SEG_LEAF
#define PAGE_BTR_IBUF_FREE_LIST_NODE PAGE_BTR_SEG_LEAF
/* in the place of PAGE_BTR_SEG_LEAF and _TOP
there is a free list base node if the page is
the root page of an ibuf tree, and at the same
place is the free list node if the page is in
a free list */
// 存储 B+ Tree 内节点段的地址
#define PAGE_BTR_SEG_TOP (36 + FSEG_HEADER_SIZE)
/* file segment header for the non-leaf pages
in a B-tree: defined only on the root page of
a B-tree, but not in the root of an ibuf
tree */
/*----*/
#define PAGE_DATA (PAGE_HEADER + 36 + 2 * FSEG_HEADER_SIZE)
/* start of data on the page */

#define PAGE_OLD_INFIMUM (PAGE_DATA + 1 + REC_N_OLD_EXTRA_BYTES)
/* offset of the page infimum record on an
old-style page */
#define PAGE_OLD_SUPREMUM (PAGE_DATA + 2 + 2 * REC_N_OLD_EXTRA_BYTES + 8)
/* offset of the page supremum record on an
old-style page */
#define PAGE_OLD_SUPREMUM_END (PAGE_OLD_SUPREMUM + 9)
/* offset of the page supremum record end on
an old-style page */
#define PAGE_NEW_INFIMUM (PAGE_DATA + REC_N_NEW_EXTRA_BYTES)
/* offset of the page infimum record on a
new-style compact page */
#define PAGE_NEW_SUPREMUM (PAGE_DATA + 2 * REC_N_NEW_EXTRA_BYTES + 8)
/* offset of the page supremum record on a
new-style compact page */
#define PAGE_NEW_SUPREMUM_END (PAGE_NEW_SUPREMUM + 8)
/* offset of the page supremum record end on
a new-style compact page */
/*-----------------------------*/

/* Heap numbers */
/** Page infimum */
constexpr ulint PAGE_HEAP_NO_INFIMUM = 0;
/** Page supremum */
constexpr ulint PAGE_HEAP_NO_SUPREMUM = 1;

/** First user record in creation (insertion) order, not necessarily collation
order; this record may have been deleted */
constexpr ulint PAGE_HEAP_NO_USER_LOW = 2;

/* Directions of cursor movement */
#define PAGE_LEFT 1
#define PAGE_RIGHT 2
#define PAGE_SAME_REC 3
#define PAGE_SAME_PAGE 4
#define PAGE_NO_DIRECTION 5

/** Eliminates a name collision on HP-UX */
#define page_t ib_page_t
/** Type of the index page */
typedef byte page_t;
/** Index page cursor */
struct page_cur_t;

/** Compressed index page */
typedef byte page_zip_t;

/* The following definitions would better belong to page0zip.h,
but we cannot include page0zip.h from rem0rec.ic, because
page0*.h includes rem0rec.h and may include rem0rec.ic. */

/** Number of bits needed for representing different compressed page sizes */
#define PAGE_ZIP_SSIZE_BITS 3

/** Maximum compressed page shift size */
#define PAGE_ZIP_SSIZE_MAX \
  (UNIV_ZIP_SIZE_SHIFT_MAX - UNIV_ZIP_SIZE_SHIFT_MIN + 1)

/* Make sure there are enough bits available to store the maximum zip
ssize, which is the number of shifts from 512. */
#if PAGE_ZIP_SSIZE_MAX >= (1 << PAGE_ZIP_SSIZE_BITS)
#error "PAGE_ZIP_SSIZE_MAX >= (1 << PAGE_ZIP_SSIZE_BITS)"
#endif

/* Page cursor search modes; the values must be in this order! */
enum page_cur_mode_t {
  PAGE_CUR_UNSUPP = 0,
  PAGE_CUR_G = 1,
  PAGE_CUR_GE = 2,
  PAGE_CUR_L = 3,
  PAGE_CUR_LE = 4,

  /*      PAGE_CUR_LE_OR_EXTENDS = 5,*/ /* This is a search mode used in
                                   "column LIKE 'abc%' ORDER BY column DESC";
                                   we have to find strings which are <= 'abc' or
                                   which extend it */

  /* These search mode is for search R-tree index. */
  PAGE_CUR_CONTAIN = 7,
  PAGE_CUR_INTERSECT = 8,
  PAGE_CUR_WITHIN = 9,
  PAGE_CUR_DISJOINT = 10,
  PAGE_CUR_MBR_EQUAL = 11,
  PAGE_CUR_RTREE_INSERT = 12,
  PAGE_CUR_RTREE_LOCATE = 13,
  PAGE_CUR_RTREE_GET_FATHER = 14
};

/** Compressed page descriptor */
struct page_zip_des_t {
  /** Compressed page data */
  page_zip_t *data;

#ifdef UNIV_DEBUG
  /** Start offset of modification log */
  uint16_t m_start;
  /** Allocated externally, not from the buffer pool */
  bool m_external;
#endif /* UNIV_DEBUG */

  /** End offset of modification log */
  uint16_t m_end;

  /** Number of externally stored columns on the page; the maximum is 744
  on a 16 KiB page */
  uint16_t n_blobs;

  /** true if the modification log is not empty.  */
  bool m_nonempty;

  /** 0 or compressed page shift size; the size in bytes is:
  (UNIV_ZIP_SIZE_MIN * >> 1) << ssize. */
  uint8_t ssize;
};

/** Compression statistics for a given page size */
struct page_zip_stat_t {
  /** Number of page compressions */
  ulint compressed;
  /** Number of successful page compressions */
  ulint compressed_ok;
  /** Number of page decompressions */
  ulint decompressed;
  /** Duration of page compressions in microseconds */
  ib_uint64_t compressed_usec;
  /** Duration of page decompressions in microseconds */
  ib_uint64_t decompressed_usec;
  page_zip_stat_t()
      : /* Initialize members to 0 so that when we do
        stlmap[key].compressed++ and element with "key" does not
        exist it gets inserted with zeroed members. */
        compressed(0),
        compressed_ok(0),
        decompressed(0),
        compressed_usec(0),
        decompressed_usec(0) {}
};

/** Compression statistics types */
typedef std::map<index_id_t, page_zip_stat_t, std::less<index_id_t>,
                 ut::allocator<std::pair<const index_id_t, page_zip_stat_t>>>
    page_zip_stat_per_index_t;

/** Statistics on compression, indexed by page_zip_des_t::ssize - 1 */
extern page_zip_stat_t page_zip_stat[PAGE_ZIP_SSIZE_MAX];
/** Statistics on compression, indexed by dict_index_t::id */
extern page_zip_stat_per_index_t page_zip_stat_per_index;

/** Write the "deleted" flag of a record on a compressed page.  The flag must
 already have been written on the uncompressed page. */
void page_zip_rec_set_deleted(
    page_zip_des_t *page_zip, /*!< in/out: compressed page */
    const byte *rec,          /*!< in: record on the uncompressed page */
    ulint flag);              /*!< in: the deleted flag (nonzero=TRUE) */

/** Write the "owned" flag of a record on a compressed page.  The n_owned field
 must already have been written on the uncompressed page.
@param[in,out] page_zip Compressed page
@param[in] rec Record on the uncompressed page
@param[in] flag The owned flag (nonzero=true) */
void page_zip_rec_set_owned(page_zip_des_t *page_zip, const byte *rec,
                            ulint flag);

/** Shift the dense page directory when a record is deleted.
@param[in,out]	page_zip	compressed page
@param[in]	rec		deleted record
@param[in]	index		index of rec
@param[in]	offsets		rec_get_offsets(rec)
@param[in]	free		previous start of the free list */
void page_zip_dir_delete(page_zip_des_t *page_zip, byte *rec,
                         dict_index_t *index, const ulint *offsets,
                         const byte *free);

/** Add a slot to the dense page directory.
@param[in,out]  page_zip      Compressed page
@param[in]      is_clustered  Nonzero for clustered index, zero for others */
void page_zip_dir_add_slot(page_zip_des_t *page_zip, bool is_clustered);

#endif
