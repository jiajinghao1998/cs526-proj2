int sadd_overflow(void)
{
  int i1 = 1;
  int i2 = 0x7FFFFFFF;
  return i1 + i2;
}

int sadd_no_overflow(void)
{
  int i1 = 1;
  int i2 = 0x7FFFFFFE;
  return i1 + i2;
}

unsigned int uadd_overflow(void)
{
  unsigned int i1 = 1;
  unsigned int i2 = 0xFFFFFFFF;
  return i1 + i2;
}


unsigned int uadd_no_overflow(void)
{
  unsigned int i1 = 1;
  unsigned int i2 = 0x7FFFFFFF;
  return i1 + i2;
}

int ssub_overflow(void)
{
  int i1 = -2;
  int i2 = 0x7FFFFFFF;
  return i1 - i2;
}

int ssub_no_overflow(void)
{
  int i1 = -2;
  int i2 = 0x7FFFFFFE;
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
  int i2 = 0x40000000;
  return i1 * i2;
}

int smul_no_overflow(void)
{
  int i1 = 2;
  int i2 = 0x3FFFFFFF;
  return i1 * i2;
}

unsigned int umul_overflow(void)
{
  unsigned int i1 = 2;
  unsigned int i2 = 0x80000000;
  return i1 * i2;
}


unsigned int umul_no_overflow(void)
{
  unsigned int i1 = 1;
  unsigned int i2 = 0x7FFFFFFF;
  return i1 * i2;
}

int shl_error(void)
{
  int i1 = 0x7FFFFFFF;
  int i2 = 32;
  return i1 << i2;
}

int shl_no_error(void)
{
  int i1 = 0x7FFFFFFF;
  int i2 = 31;
  return i1 << i2;
}

unsigned int lshr_error(void)
{
  unsigned int i1 = 0xFFFFFFFF;
  unsigned int i2 = 32;
  return i1 >> i2;
}

unsigned int lshr_no_error(void)
{
  unsigned int i1 = 0xFFFFFFFF;
  unsigned int i2 = 31;
  return i1 >> i2;
}

int ashr_error(void)
{
  int i1 = 0xFFFFFFFF;
  int i2 = 32;
  return i1 >> i2;
}

int ashr_no_error(void)
{
  int i1 = 0xFFFFFFFF;
  int i2 = 31;
  return i1 >> i2;
}
