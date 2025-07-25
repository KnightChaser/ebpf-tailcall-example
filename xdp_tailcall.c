// xdp_tailcall.c

#include <bpf/bpf.h>
#include <bpf/libbpf.h>
#include <linux/if_link.h>
#include <net/if.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>

// This header is generated by bpftool
#include "xdp_tailcall.skel.h"

// Include the shared header for tail call targets
#include "shared.h"

static struct bpf_link *bpf_program_link = NULL;
static volatile bool exiting =
    false;               // Flag to indicate when to exit the program
static int ifindex = -1; // Interface index for the XDP program
static struct xdp_tailcall_bpf *skel; // BPF skeleton structure

static void sig_handler(int sig __attribute__((unused))) { exiting = true; }

static int libbpf_print_fn(enum libbpf_print_level level
                           __attribute__((unused)),
                           const char *format, va_list args) {
  return vfprintf(stderr, format, args);
}

int main(int argc, char **argv) {
  int err;

  if (argc < 2) {
    fprintf(stderr, "Usage: %s <interface_name>\n", argv[0]);
    return 1;
  }

  // Convert interface name to index
  ifindex = if_nametoindex(argv[1]);
  if (ifindex == 0) {
    perror("if_nametoindex");
    return 1;
  }

  // Set up libbpf errors and debug info callback
  libbpf_set_print(libbpf_print_fn);

  signal(SIGINT, sig_handler);
  signal(SIGTERM, sig_handler);

  // Open and load the BPF skeleton
  skel = xdp_tailcall_bpf__open();
  if (!skel) {
    fprintf(stderr, "Failed to open BPF skeleton\n");
    return 1;
  }

  err = xdp_tailcall_bpf__load(skel);
  if (err) {
    fprintf(stderr, "Failed to load and verify BPF skeleton\n");
    goto cleanup;
  }

  // NOTE: Tail call setup procedure
  // Create a table that maps the tail calls index enum to the
  // corresponding program in the BPF skeleton
  struct {
    enum tail_call_targets key;
    struct bpf_program *prog;
  } tail_call_entries[] = {
      {TAIL_CALL_PROG_1, skel->progs.xdp_prog_1},
      {TAIL_CALL_PROG_2, skel->progs.xdp_prog_2},
  };

  // Get the program array
  int prog_array_fd = bpf_map__fd(skel->maps.prog_array);
  if (prog_array_fd < 0) {
    fprintf(stderr, "Failed to get program array map FD\n");
    err = prog_array_fd;
    goto cleanup;
  }

  // Loop through the tail call entries and populate the map
  for (size_t i = 0; i < MAX_TAIL_CALLS; i++) {
    int key = tail_call_entries[i].key;
    int prog_fd = bpf_program__fd(tail_call_entries[i].prog);
    if (prog_fd < 0) {
      fprintf(stderr, "Failed to get program FD for tail call %d\n", key);
      err = prog_fd;
      goto cleanup;
    }

    // Update the program array map with the tail call program
    err = bpf_map_update_elem(prog_array_fd, &key, &prog_fd, BPF_ANY);
    if (err) {
      fprintf(stderr, "Failed to update tail call map for key %d: %s\n", key,
              strerror(-err));
      goto cleanup;
    }
  }

  printf("Successfully populated tail call map.\n");

  // Attach the main XDP program
  bpf_program_link =
      bpf_program__attach_xdp(skel->progs.xdp_prog_entry, ifindex);
  err = libbpf_get_error(bpf_program_link);
  if (err) {
    fprintf(stderr, "Failed to attach XDP program to interface %s: %s\n",
            argv[1], strerror(-err));
    bpf_program_link = NULL;
    goto cleanup;
  }

  printf("Successfully attached XDP program to iface %s (index %d)\n", argv[1],
         ifindex);
  printf("Press Ctrl-C to exit and detach.\n\n");
  printf("Run this command to see the output:\n");
  printf("  sudo cat /sys/kernel/debug/tracing/trace_pipe\n");

  while (!exiting) {
    sleep(1);
  }

cleanup:
  // Detach and clean up
  printf("\nDetaching and cleaning up...\n");
  if (bpf_program_link) {
    bpf_link__destroy(bpf_program_link);
  }
  xdp_tailcall_bpf__destroy(skel);
  return -err;
}
