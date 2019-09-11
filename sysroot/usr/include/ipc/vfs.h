#ifndef IPC_VFS_H
#define IPC_VFS_H

typedef enum {
  VFS_OPEN,
  VFS_PUTS,
  VFS_GETS,
  VFS_CLOSE,
  VFS_REGISTER_FS,
  VFS_MOUNT,
  VFS_UMOUNT,
  VFS_SEEK
} vfs_message_type;

typedef struct {
  vfs_message_type type;
  uint8_t id;
  char mode[10];
  uint32_t fd;
  char path[4096];
  uint32_t pos;
  char flags;
  int data;
  char in_progress;
  uint32_t orig_mbox;
  void* fs_data;
} vfs_message;

#endif
