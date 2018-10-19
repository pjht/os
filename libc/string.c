int strlen(char* str) {
  int i;
  for (i=0;str[i]!='\0';i++) {
    continue;
  }
  return i;
}

void reverse(char* str) {
    char chr;
    int j;
    for (int i=0,j=strlen(str)-1;i<j;i++,j--) {
      chr=str[i];
      str[i]=str[j];
      str[j]=chr;
    }
}

void int_to_ascii(int n,char* str) {
    int i;
    int sign;
    if ((sign = n)<0) {
      n=-n;
    }
    i=0;
    do {
      str[i++]=n%10+'0';
    } while ((n /= 10) > 0);
    if (sign < 0) {
      str[i++] = '-';
    }
    str[i]='\0';
    reverse(str);
}
