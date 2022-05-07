LEVEL = ../..

## change or use make TARGET=inputfilename
TARGET=int_add

## replace LLVMROOT and SROALIB as appropriate
NETID = `whoami`
LLVMROOT ?= /home/jinghao/course/cs526/proj2/llvm-project/build
SROALIB  ?= $(LEVEL)/build/src/libKINT.so

LLVMGCC = clang
LLVMAS  = $(LLVMROOT)/bin/llvm-as
LLVMDIS = $(LLVMROOT)/bin/llvm-dis
LLVMOPT = $(LLVMROOT)/bin/opt


## Other choices: test or comparecfe (these will be provided later)
default: debug
#DEBUGOPTS =  -verify -targetlibinfo -tbaa -basic-aa -globalopt -ipsccp -deadargelim \
#	-simplifycfg -basic-aa -prune-eh -inline -function-attrs -argpromotion \
#	-mem2reg -sroa -early-cse -lazy-value-info \
#	-jump-threading -correlated-propagation -simplifycfg \
#	-strip-dead-prototypes -globaldce -constmerge -adce -simplifycfg -O2
DEBUGOPTS = -verify -mem2reg
testsimple: debug
debug: $(TARGET).debug.bc
disassembly: $(TARGET).llvm.ll $(TARGET).debug.ll

## test (implement yourself) can be used to test out your solution with a known comparison target.
## I will use different pass pipelines during the actual testing.
## Please test your solution with different pass pipelines when using the test target.
test: $(TARGET).test.bc


.PRECIOUS: %.ll


%.ll: %.bc
	$(LLVMDIS) -f $<


%.llvm.bc: %.c
	$(LLVMGCC) -Xclang -disable-O0-optnone -S -emit-llvm -o - $< | $(LLVMAS) -o=$@

%.debug.bc: %.llvm.bc
	$(LLVMOPT) $(DEBUGOPTS) -stats -enable-new-pm=0 < $< | \
	$(LLVMOPT) -load $(SROALIB) -kint-check-insertion -print-module -verify -kint-smt-query -enable-new-pm=0 -o=$@

clean:
	$(RM) -f *.debug.bc *.test.bc *.llvm.bc *.ll
