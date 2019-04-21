#ifndef EXT2_STRUCTS_H
#define EXT2_STRUCTS_H
typedef struct {
  uint32_t s_inodes_count;
  uint32_t s_blocks_count;
  uint32_t s_r_blocks_count;
  uint32_t s_free_blocks_count;
  uint32_t s_free_inodes_count;
  uint32_t s_first_data_block;
  uint32_t s_log_blk_size;
  uint32_t s_log_frag_size;
  uint32_t s_blocks_per_group;
  uint32_t s_frags_per_group;
  uint32_t s_inodes_per_group;
  uint32_t s_mtime;
  uint32_t s_wtime;
  uint16_t s_mnt_count;
  uint16_t s_max_mnt_count;
  uint16_t s_magic;
  uint16_t s_state;
  uint16_t s_errors;
  uint16_t s_minor_rev_level;
  uint32_t s_lastcheck;
  uint32_t s_checkinterval;
  uint32_t s_creator_os;
  uint32_t s_rev_level;
  uint16_t s_def_resuid;
  uint16_t s_def_resgid;
  uint32_t s_first_ino;
  uint16_t s_inode_size;
  uint16_t s_block_group_nr;
  uint32_t s_feature_compat;
  uint32_t s_feature_incompat;
  uint32_t s_feature_rw_compat;
  char s_uuid[16];
  char s_volume_name[16];
  char s_last_mounted[64];
  uint32_t s_algo_bitmap;
  char s_prealloc_blocks;
  char s_prealloc_dir_blocks;
  char s_journal_uuid[16];
  uint32_t s_journal_inum;
  uint32_t s_journal_dev;
  uint32_t s_last_orphan;
} __attribute__((packed)) ext2_superblock;

typedef enum {
  EXT2_FEATURE_COMPAT_DIR_PREALLOC=1,
  EXT2_FEATURE_COMPAT_IMAGIC_INODES=2,
  EXT2_FEATURE_COMPAT_HAS_JOURNAL=4,
  EXT2_FEATURE_COMPAT_EXT_ATTR=8,
  EXT2_FEATURE_COMPAT_RESIZE_INO=16,
  EXT2_FEATURE_COMPAT_DIR_INDEX=32
}  s_feature_compat;

typedef enum {
  EXT2_FEATURE_INCOMPAT_COMPRESSION=1,
  EXT2_FEATURE_INCOMPAT_FILETYPE=2,
  EXT2_FEATURE_INCOMPAT_RECOVER=4,
  EXT2_FEATURE_INCOMPAT_JOURNAL_DEV=8
}  s_feature_incompat;

typedef enum {
  EXT2_FEATURE_RW_COMPAT_SPARSE_SUPER=1,
  EXT2_FEATURE_RW_COMPAT_LARGE_FILE=2,
  EXT2_FEATURE_RW_COMPAT_BTREE_DIR=4
}  s_feature_ro_compat;

typedef struct {
  uint32_t bg_blk_bitmap;
  uint32_t bg_inode_bitmap;
  uint32_t bg_inode_table;
  uint16_t bg_free_blocks_count;
  uint16_t bg_free_inodes_count;
  uint16_t bg_used_dirs_count;
  char unused[14];
} __attribute__((packed)) blk_grp;

typedef struct {
  uint16_t i_mode;
  uint16_t i_uid;
  uint32_t i_size;
  uint32_t i_atime;
  uint32_t i_ctime;
  uint32_t i_mtime;
  uint32_t i_dtime;
  uint16_t i_gid;
  uint16_t i_links_count;
  uint32_t i_blocks;
  uint32_t i_flags;
  uint32_t i_osd1;
  uint32_t i_block[15];
  uint32_t i_generation;
  uint32_t i_file_acl;
  uint32_t i_ext_size_or_dir_acl;
  uint32_t i_faddr;
  uint32_t i_osd2;
  char unused[8];
} __attribute__((packed)) inode;

typedef enum {
  EXT2_S_IFIFO=0x100,
  EXT2_S_IFCHR=0x2000,
  EXT2_S_IFDIR=0x4000,
  EXT2_S_IFBLK=0x6000,
  EXT2_S_IFREG=0x8000,
  EXT2_S_IFLNK=0xA000,
  EXT2_S_IFSOCK=0xC000
} i_mode;

typedef enum {
  EXT2_SECRM_FL=0x1,
  EXT2_UNRM_FL=0x2,
  EXT2_COMPR_FL=0x4,
  EXT2_SYNC_FL=0x8,
  EXT2_IMMUTABLE_FL=0x10,
  EXT2_APPEND_FL=0x20,
  EXT2_NODUMP_FL=0x40,
  EXT2_NOATIME_FL=0x80,
  EXT2_DIRTY_FL=0x100,
  EXT2_COMPRBLK_FL=0x200,
  EXT2_NOCOMPR_FL=0x400,
  EXT2_ECOMPR_FL=0x800,
  EXT2_BTREE_FL=0x1000,
  EXT2_INDEX_FL=0x1000,
  EXT2_IMAGIC_FL=0x2000,
  EXT2_JOURNAL_DATA_FL=0x4000,
  EXT2_RESERVED_FL=0x80000000
} i_flags;

typedef struct {
  uint32_t inode;
  uint16_t rec_len;
  char name_len;
  char file_type;
  char file_name[1];
} __attribute__((packed)) ext2_dir_entry;
#endif
