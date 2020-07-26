#ifndef VFS_H
#define VFS_H

void register_fs(const char* name,uint32_t mbox);
void mount(char* file,char* type,char* path);

#endif
