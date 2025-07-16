# Makefile

# ---- basic paths ------------------------------------------------
BPF_SRC        := xdp_tailcall.bpf.c
BPF_OBJ        := $(BPF_SRC:.c=.o)
SKEL_HEADER    := xdp_tailcall.skel.h
USER_BIN       := xdp_tailcall

# user-space sources
USER_SRCS      := xdp_tailcall.c
USER_OBJS      := $(USER_SRCS:.c=.o)

CLANG          ?= clang
BPFTOOL        ?= bpftool
CC             ?= gcc

# Adjust flags as needed
CFLAGS_BPF     ?= -g -O2 -target bpf
CFLAGS_USER    ?= -g -O2
LDFLAGS_USER   ?= -lelf -lbpf

# ---- default target ---------------------------------------------
all: $(USER_BIN)

# 1. compile the kernel program
$(BPF_OBJ): $(BPF_SRC)
	$(CLANG) $(CFLAGS_BPF) -c $< -o $@

# 2. generate skeleton
$(SKEL_HEADER): $(BPF_OBJ)
	$(BPFTOOL) gen skeleton $< > $@

# 3a. build user-space object files
%.o: %.c $(SKEL_HEADER)
	$(CC) $(CFLAGS_USER) -c $< -o $@

# 3b. link the final loader binary
$(USER_BIN): $(USER_OBJS)
	$(CC) $(CFLAGS_USER) $^ -o $@ $(LDFLAGS_USER)

# ---- housekeeping -----------------------------------------------
clean:
	rm -f $(USER_BIN) \
	      $(BPF_OBJ) $(SKEL_HEADER) \
	      $(USER_OBJS)
