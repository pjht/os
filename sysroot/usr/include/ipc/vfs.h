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
  char flags;
  vfs_message_type type;
  uint8_t id;
  char mode[10];
  uint32_t fd;
  uint32_t pos;
  int data;
  char in_progress;
  uint32_t orig_mbox;
  void* fs_data;
  char path[4096];
} __attribute__((packed)) vfs_message;

#endif
