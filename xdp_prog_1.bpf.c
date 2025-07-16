// xdp_prog_1.bpf.c

#include "common.bpf.h"

/*
 * Tail-called program at index 0.
 * This program will perform another tail call to the proram at index 1
 */
SEC("xdp")
int xdp_prog_1(struct xdp_md *ctx) {
  bpf_printk("eBPF program #1: I'm the first tail call, calling next...\n");

  // tail call to the program at index 1 in the prog_array map
  bpf_tail_call(ctx, &prog_array, 1);

  bpf_printk("eBPF program #1: Tail call failed, drop the packet\n");
  return XDP_DROP;
}
