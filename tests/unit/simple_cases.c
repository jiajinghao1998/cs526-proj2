int sadd_overflow(void)
{
  int i1 = 1;
  int i2 = 2147483647;
  return i1 + i2;
}

int sadd_no_overflow(void)
{
  int i1 = 1;
  int i2 = 2147483646;
  return i1 + i2;
}

unsigned int uadd_overflow(void)
{
  unsigned int i1 = 1;
  unsigned int i2 = 4294967295;
  return i1 + i2;
}


unsigned int uadd_no_overflow(void)
{
  unsigned int i1 = 1;
  unsigned int i2 = 2147483647;
  return i1 + i2;
}

int ssub_overflow(void)
{
  int i1 = -2;
  int i2 = 2147483647;
  return i1 - i2;
}

int ssub_no_overflow(void)
{
  int i1 = -2;
  int i2 = 2147483646;
  return i1 - i2;
}

unsigned int usub_overflow(void)
{
  unsigned int i1 = 1;
  unsigned int i2 = 2;
  return i1 - i2;
}


unsigned int usub_no_overflow(void)
{
  unsigned int i1 = 1;
  unsigned int i2 = 1;
  return i1 - i2;
}

int smul_overflow(void)
{
  int i1 = 2;
  int i2 = 1073741824;
  return i1 * i2;
}

int smul_no_overflow(void)
{
  int i1 = 2;
  int i2 = 1073741823;
  return i1 * i2;
}

unsigned int umul_overflow(void)
{
  unsigned int i1 = 2;
  unsigned int i2 = 2147483648;
  return i1 * i2;
}


unsigned int umul_no_overflow(void)
{
  unsigned int i1 = 1;
  unsigned int i2 = 2147483647;
  return i1 * i2;
}
