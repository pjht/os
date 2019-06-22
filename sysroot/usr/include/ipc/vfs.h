#ifndef IPC_VFS_H
#define IPC_VFS_H

typedef enum {
  VFS_OPEN
} vfs_message_type;

typedef struct {
  vfs_message_type type;
  uint32_t id;
  char mode[10];
  uint32_t fd;
  char path[0];
} vfs_message;

#endif
