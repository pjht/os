#ifndef IPC_VFS_H
#define IPC_VFS_H

typedef enum {
  VFS_OPEN,
  VFS_PUTC,
  VFS_GETC,
  VFS_CLOSE,
  VFS_REGISTER_FS,
  VFS_MOUNT,
  VFS_UMOUNT
} vfs_message_type;

typedef struct {
  vfs_message_type type;
  uint32_t id;
  char mode[10];
  uint32_t fd;
  char path[4096];
  char flags;
  char data;
} vfs_message;

#endif
