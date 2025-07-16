# Makefile

# --- tools -------------------------------------------------
CLANG          ?= clang
LLVM_LINK      ?= llvm-link
BPFTOOL        ?= bpftool
CC             ?= gcc

# --- BPF paths ---------------------------------------------
BPF_SRCS       := xdp_entry.bpf.c xdp_prog_1.bpf.c xdp_prog_2.bpf.c
BPF_BCS        := $(BPF_SRCS:.c=.bc)
BPF_BC_LINKED  := xdp_tailcall.linked.bc
BPF_OBJ_FINAL  := xdp_tailcall.bpf.o
SKEL_HEADER    := xdp_tailcall.skel.h

# --- user-space paths ---------------------------------------
USER_BIN       := xdp_tailcall
USER_SRCS      := xdp_tailcall.c
USER_OBJS      := $(USER_SRCS:.c=.o)

# --- compilation flags --------------------------------------
CFLAGS_BPF     ?= -g -O2 -target bpf
CFLAGS_USER    ?= -g -O2
LDFLAGS_USER   ?= -lelf -lbpf

# ---- build rules -------------------------------------------
all: vmlinux.h $(USER_BIN)

# 0. GENERATE vmlinux.h if it doesn't exist (Good practice!)
vmlinux.h:
	$(BPFTOOL) btf dump file /sys/kernel/btf/vmlinux format c > $@

# 1. COMPILE each C source file to an LLVM bitcode file (.bc)
#    Note the new flag: -emit-llvm
%.bpf.bc: %.bpf.c common.bpf.h Makefile
	$(CLANG) $(CFLAGS_BPF) -emit-llvm -c $< -o $@

# 2. LINK the individual bitcode files into a single bitcode file
$(BPF_BC_LINKED): $(BPF_BCS)
	$(LLVM_LINK) $^ -o $@

# 3. COMPILE the linked bitcode file into the final BPF ELF object
$(BPF_OBJ_FINAL): $(BPF_BC_LINKED)
	$(CLANG) $(CFLAGS_BPF) -c $< -o $@

# 4. GENERATE the skeleton from the final ELF object
$(SKEL_HEADER): $(BPF_OBJ_FINAL)
	$(BPFTOOL) gen skeleton $< > $@

# 5a. BUILD user-space object files
%.o: %.c $(SKEL_HEADER)
	$(CC) $(CFLAGS_USER) -c $< -o $@

# 5b. LINK the final user-space binary
$(USER_BIN): $(USER_OBJS)
	$(CC) $(CFLAGS_USER) $^ -o $@ $(LDFLAGS_USER)

# ---- housekeeping -----------------------------------------------
clean:
	rm $(USER_BIN) \
	   $(BPF_BCS) $(BPF_BC_LINKED) $(BPF_OBJ_FINAL) \
	   $(SKEL_HEADER) \
	   $(USER_OBJS)
