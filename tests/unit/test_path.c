
// Smells like a Boolector bug, see playground.c for equivalent boolector prog
int error1(int i)
{
  if (i > 1000)
    i = 1000;
  return i + 100;
}
/*
int error2(int i)
{
  if (i < 1000)
    i = 1000;
  return i + 100;
}
*/
