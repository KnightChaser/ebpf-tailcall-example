// xdp_prog_2.bpf.c

#include "common.bpf.h"

SEC("xdp")
int xdp_prog_2(struct xdp_md *ctx) {
  bpf_printk("eBPF program #2: Final step, packet passed!\n");
  return XDP_PASS;
}
