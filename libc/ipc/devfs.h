#ifndef DEVFS_H
#define DEVFS_H

typedef enum {
  DEVFS_REGISTER
} devfs_msg_type;

typedef enum {
  DEV_GLOBAL,
  DEV_INDIVIDUAL
} devfs_dev_type;


typedef struct {
  int msg_type;
  int dev_type;
  uint32_t mbox;
  char name[128];
} devfs_message;

#endif
