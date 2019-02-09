float ceilf(float num) {
  int inum=(int)num;
  if (num==(float)inum) {
    return (float)inum;
  }
  return (float)(inum+1);
}
