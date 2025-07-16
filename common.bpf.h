// common.bpf.h

#ifndef __COMMON_BPF_H
#define __COMMON_BPF_H

#include "vmlinux.h"
#include <bpf/bpf_helpers.h>

#define MAX_TAIL_CALLS 2

char LICENSE[] SEC("license") __attribute__((weak)) = "GPL";

/*
 * 1) Give the map‐type struct a name:
 *    struct prog_array_map { ... };
 * 2) Declare the instance as extern (no SEC here).
 */
struct prog_array_map {
  __uint(type, BPF_MAP_TYPE_PROG_ARRAY);
  __uint(max_entries, MAX_TAIL_CALLS);
  __type(key, __u32);
  __type(value, __u32);
};

// This will be instantiated in the xdp_entry.bpf.c file
// and other programs will reference it.
extern struct prog_array_map prog_array;

#endif // __COMMON_BPF_H
