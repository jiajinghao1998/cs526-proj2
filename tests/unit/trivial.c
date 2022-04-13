#include <stdio.h>
#include <stdlib.h>

/* Although this is a trivial case of scalar replacement, it is
 * instructive to study the unoptimized code.  Run "make trivial.llvm.bc",
 * disassemble the resulting file, and study it carefully to understand the
 * LLVM code that is generated.
 */
int main(int argc, char* argv[])
{
  int i1 = atoi(argv[1]);
  int i2 = atoi(argv[2]);

  long l1 = i1;
  long l2 = i2;
  long l3 = l1 + l2;

  return i1 + i2;
}
