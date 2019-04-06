float ceilf(float num) {
  int inum=(int)num;
  if (num==(float)inum) {
    return (float)inum;
  }
  return (float)(inum+1);
}

double ceil(double num) {
  int inum=(int)num;
  if (num==(double)inum) {
    return (double)inum;
  }
  return (double)(inum+1);
}
