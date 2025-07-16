// xdp_entry.bpf.c

#include "common.bpf.h"

struct prog_array_map prog_array SEC(".maps");

/*
 * This is the entry point for the XDP program.
 * It will be the first program executed when a packet arrives.
 */
SEC("xdp")
int xdp_prog_entry(struct xdp_md *ctx) {
  bpf_printk("eBPF program #0 (entry): I'm attached! Doing a tail call...\n");

  // tail call to the program at index 0 in the prog_array map.
  bpf_tail_call(ctx, &prog_array, 0);

  // If the tail call fails, we will drop the packet.
  // This code will only execute if the tail call fails.
  bpf_printk("eBPF program #0 (entry): Tail call failed, dropping packet.\n");
  return XDP_DROP;
}
