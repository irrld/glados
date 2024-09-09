//
// Created by irrl on 9/4/24.
//

#ifndef GLADOS_SUPERBLOCK_H
#define GLADOS_SUPERBLOCK_H

#include "glados/stddef.h"

enum ext2_fs_state {
  EXT2_FS_STATE_CLEAN      = 1,
  EXT2_FS_STATE_HAS_ERRORS = 2
};
typedef uint16_t ext2_fs_state_t;

enum ext2_error_handling {
  EXT2_ERROR_HANDLING_IGNORE       = 1,
  EXT2_ERROR_HANDLING_REMOUNT_RO   = 2,
  EXT2_ERROR_HANDLING_KERNEL_PANIC = 3
};
typedef uint16_t ext2_error_handling_t;

enum ext2_optional_feature {
  EXT2_OPTIONAL_FEATURE_PREALLOCATE         = 0x0001,
  EXT2_OPTIONAL_FEATURE_HAS_AFS_INODES      = 0x0002,
  EXT2_OPTIONAL_FEATURE_HAS_JOURNAL         = 0x0004,
  EXT2_OPTIONAL_FEATURE_EXTENDED_INODE_ATTR = 0x0008,
  EXT2_OPTIONAL_FEATURE_FS_RESIZE           = 0x0010,
  EXT2_OPTIONAL_FEATURE_DIR_USE_HASH_INDEX  = 0x0020,
};
typedef uint32_t ext2_optional_feature_t;

enum ext2_required_feature {
  EXT2_REQUIRED_FEATURE_HAS_COMPRESSION         = 0x0001,
  EXT2_REQUIRED_FEATURE_DIR_HAS_TYPE            = 0x0002,
  EXT2_REQUIRED_FEATURE_JOURNAL_REPLAY_REQUIRED = 0x0004,
  EXT2_REQUIRED_FEATURE_USES_JOURNAL_DEVICE     = 0x0008
};
typedef uint32_t ext2_required_feature_t;

enum ext2_readonly_feature {
  EXT2_READONLY_FEATURE_SPARSE_SB_AND_GDT    = 0x0001, // sparse superblocks and group descriptor tables
  EXT2_READONLY_FEATURE_64_BIT_FS            = 0x0002,
  EXT2_READONLY_FEATURE_DIRS_USE_BINARY_TREE = 0x0004
};
typedef uint32_t ext2_readonly_feature_t;

// each block should be 1024 bytes in length
// todo check size
typedef struct ext2_superblock {
  uint32_t inodes;
  uint32_t blocks;
  uint32_t reserved_blocks;
  uint32_t unallocated_blocks;
  uint32_t unallocated_inodes;
  uint32_t block_number;
  uint32_t block_size;
  uint32_t fragment_size;
  uint32_t block_group_size;
  uint32_t block_fragment_size;
  uint32_t block_inode_size;
  uint32_t last_mount;
  uint32_t last_written;
  uint16_t mounts_since_last_fsck;
  uint16_t max_mounts_since_last_fsck;
  uint16_t signature; // always 0xEF53
  ext2_fs_state_t fs_state;
  ext2_error_handling_t error_handling;
  uint16_t minor_version;
  uint32_t last_fsck_time; // posix time
  uint32_t fsck_interval; // posix time
  uint32_t os_id;
  uint32_t major_version;
  uint16_t user_id;
  uint16_t group_id;
  // Rest of this is only used when major_version > 0
  uint32_t first_non_reserved_inode;
  uint16_t inode_size;
  uint16_t block_group; // if backup
  ext2_optional_feature_t optional_features;
  ext2_required_feature_t required_features;
  ext2_readonly_feature_t readonly_features; // if any of these set, system is mounted as read-only
  uint64_t fs_id_low; // blkid
  uint64_t fs_id_high;
  char volume_name[16];
  char mount_path[64];
  uint32_t compression_algorithm;
  uint8_t file_preallocate_blocks;
  uint8_t dir_preallocate_blocks;
  uint16_t unused_1;
  uint64_t journal_id_low; // same style fs_id above
  uint64_t journal_id_high;
  uint32_t journal_inode;
  uint32_t journal_device;
  uint32_t orphan_inode_head;
  char unused_2[788];
} __attribute__((packed)) ext2_superblock_t;

#endif  //GLADOS_SUPERBLOCK_H
