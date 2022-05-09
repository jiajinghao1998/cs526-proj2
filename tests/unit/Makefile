LEVEL = ../..

## replace LLVMROOT and SROALIB as appropriate
NETID = `whoami`
LLVMROOT ?= /home/jinghao/course/cs526/proj2/llvm-project/build
SROALIB  ?= $(LEVEL)/build/src/libKINT.so

LLVMGCC = clang
LLVMAS  = $(LLVMROOT)/bin/llvm-as
LLVMDIS = $(LLVMROOT)/bin/llvm-dis
LLVMOPT = $(LLVMROOT)/bin/opt


## Other choices: test or comparecfe (these will be provided later)
default: test_single_op
#DEBUGOPTS =  -verify -targetlibinfo -tbaa -basic-aa -globalopt -ipsccp -deadargelim \
#	-simplifycfg -basic-aa -prune-eh -inline -function-attrs -argpromotion \
#	-mem2reg -sroa -early-cse -lazy-value-info \
#	-jump-threading -correlated-propagation -simplifycfg \
#	-strip-dead-prototypes -globaldce -constmerge -adce -simplifycfg -O2
PASSES = -verify -mem2reg

## test (implement yourself) can be used to test out your solution with a known comparison target.
## I will use different pass pipelines during the actual testing.
## Please test your solution with different pass pipelines when using the test target.


.PRECIOUS: %.ll

test_single_op: test_single_op.c
	$(LLVMGCC) -Xclang -disable-O0-optnone -S -emit-llvm -o - $< | \
	$(LLVMOPT) -load $(SROALIB) $(PASSES) -kint-check-insertion -verify -kint-smt-query -enable-new-pm=0 -o=/dev/null

test_path: test_path.c
	$(LLVMGCC) -Xclang -disable-O0-optnone -S -emit-llvm -o - $< | \
	$(LLVMOPT) -load $(SROALIB) $(PASSES) -print-module -kint-check-insertion -verify -print-module -kint-smt-query -enable-new-pm=0 -o=/dev/null

clean:
	$(RM) -f *.debug.bc *.test.bc *.llvm.bc *.ll
