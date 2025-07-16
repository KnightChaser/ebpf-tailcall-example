// xdp_tailcall.bpf.c

#include "vmlinux.h"
#include <bpf/bpf_core_read.h>
#include <bpf/bpf_helpers.h>
#include <bpf/bpf_tracing.h>

char LICENSE[] SEC("license") = "GPL";

/*
 * This is the program array map, the core of the tail call mechanism.
 * The user-space loader will populate this map with file descriptors
 * of the other eBPF programs.
 *
 * libbpf will find this map by its name "prog_array".
 */
struct {
  __uint(type, BPF_MAP_TYPE_PROG_ARRAY);
  __uint(max_entries, 2);
  __uint(key_size, sizeof(__u32));
  __uint(value_size, sizeof(__u32));
} prog_array SEC(".maps");

/*
 * Tail-called program at index 1.
 * This is the final program in our chain.
 */
SEC("xdp")
int xdp_prog_2(struct xdp_md *ctx) {
  bpf_printk("eBPF program #2: Final step, packet passed!\n");
  return XDP_PASS;
}

/*
 * Tail-called program at index 0.
 * This program will perform another tail call to the proram at index 1
 */
SEC("xdp")
int xdp_prog_1(struct xdp_md *ctx) {
  bpf_printk("eBPF program #1: I'm the first tail call, calling next...\n");

  // tail call to the program at index 1 in the prog_array map
  bpf_tail_call(ctx, &prog_array, 1);

  // Since tail call never returns on success,
  // The code below bpf_tail_call() will only execute if the tail call fails.
  bpf_printk("eBPF program #1: Tail call failed, continuing...\n");
  return XDP_DROP;
}

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
