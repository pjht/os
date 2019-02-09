// #include "../kernel/vfs.h"
// #include "../cpu/halt.h"
// #include "../libc/stdlib.h"
// #include "../libc/stdio.h"
// #include "../libc/string.h"
//
// char** names;
// uint32_t* offsets;
// uint32_t* sizes;
// uint32_t num_files;
// FILE* initrd_fd;
//
// char initrd_drv(fs_op op,FILE* stream,void* data1,void* data2) {
//   if (op==FSOP_MOUNT) {
//     return 1;
//   }
//   if (op==FSOP_OPEN) {
//     char file_exists=0;
//     for (int i=0;i<num_files;i++) {
//       if (strcmp(names[i],stream->path)==0) {
//         file_exists=1;
//       }
//     }
//     return file_exists;
//   }
//   if (op==FSOP_GETC) {
//     int i;
//     for (i=0;i<num_files;i++) {
//       if (strcmp(names[i],stream->path)==0) {
//         break;
//       }
//     }
//     if (stream->pos>=sizes[i]) {
//       *((int*)data1)=EOF;
//       stream->eof=1;
//       return 1;
//     }
//     fseek(initrd_fd,offsets[i]+stream->pos,SEEK_SET);
//     *((int*)data1)=fgetc(initrd_fd);
//     stream->pos+=1;
//     return 1;
//   }
//   if (op==FSOP_CLOSE) {
//     return 1;
//   }
//   return 0;
// }
//
// void init_initrd() {
//   initrd_fd=fopen("/dev/initrd","r");
//   if (!initrd_fd) {
//     printf("PANIC: Cannot open initrd!");
//     halt();
//   }
//   uint32_t max_files=32;
//   num_files=0;
//   names=malloc(sizeof(char*)*32);
//   offsets=malloc(sizeof(uint32_t)*32);
//   sizes=malloc(sizeof(uint32_t)*32);
//   for (uint32_t i=0;;i++) {
//       if (i==max_files) {
//         names=realloc(names,sizeof(char*)*(max_files+32));
//         offsets=realloc(offsets,sizeof(uint32_t)*(max_files+32));
//         sizes=realloc(sizes,sizeof(uint32_t)*(max_files+32));
//         max_files+=32;
//       }
//       uint32_t name_size;
//       fread(&name_size,4,1,initrd_fd);
//       if (name_size==0) {
//         break;
//       }
//       char* name=malloc(sizeof(char)*(name_size+1));
//       fread(name,1,name_size+1,initrd_fd);
//       long contents_size;
//       fread(&contents_size,4,1,initrd_fd);
//       long datapos=ftell(initrd_fd);
//       fseek(initrd_fd,contents_size,SEEK_CUR);
//       names[i]=name;
//       offsets[i]=datapos;
//       sizes[i]=contents_size;
//       num_files++;
//   }
//   fseek(initrd_fd,0,SEEK_SET);
//   register_fs(initrd_drv,"initrd");
// }
