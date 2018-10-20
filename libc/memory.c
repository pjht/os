void memory_copy(char *source,char *dest,int nbytes) {
    int i;
    for(i=0;i<nbytes;i++) {
      *(dest+i)=*(source+i);
    }
}

void memory_set(char *dest, char val, int len) {
    char *temp = (char *)dest;
    for ( ; len != 0; len--) *temp++ = val;
}
