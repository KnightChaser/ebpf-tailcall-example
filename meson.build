# meson.build

project(
  'xdp-tailcall',
  'c',
  default_options: ['c_std=c11', 'warning_level=2', 'buildtype=release'],
)

# Dependencies
bpftool = find_program('bpftool', required: true)
clang = find_program('clang', required: true)
llvm_link = find_program('llvm-link', required: true)
libbpf_dep = dependency('libbpf')
libelf_dep = dependency('libelf')

# 1. vmlinux.h header generation
vmlinux_h = custom_target(
  'vmlinux.h',
  output: 'vmlinux.h',
  command: [bpftool, 'btf', 'dump', 'file', '/sys/kernel/btf/vmlinux', 'format', 'c'],
  capture: true, # write stdout to file
)

# 2. BPF C code => LLVM bitcode
bpf_sources = [
  'xdp_entry.bpf.c',
  'xdp_prog_1.bpf.c',
  'xdp_prog_2.bpf.c',
]
bpf_bcs = []
foreach src : bpf_sources
  # Define a target for one C -> .bc compilation.
  bc_target = custom_target(
    input: [src, 'common.bpf.h', vmlinux_h],
    output: src.replace('.c', '.bc'),
    command: [
      clang,
      '-g',
      '-O2',
      '-target', 'bpf',
      '-emit-llvm',
      '-c', '@INPUT0@',
      '-o', '@OUTPUT@',
    ],
  )
  bpf_bcs += bc_target
endforeach

# 4. Link multple bitcode files into a single one
linked_bc = custom_target(
  'bpf_linked_bc',
  input: bpf_bcs,
  output: 'xdp_tailcall.linked.bc',
  # '@INPUT@' expands to the list of all input files.
  command: [llvm_link, '@INPUT@', '-o', '@OUTPUT@'],
)

# 5. Transform linked LLVM bitcode to BPF ELF object
bpf_obj = custom_target(
  'bpf_object',
  input: linked_bc,
  output: 'xdp_tailcall.bpf.o',
  command: [clang, '-g', '-O2', '-target', 'bpf', '-c', '@INPUT@', '-o', '@OUTPUT@'],
)

# 6. Generate BPF skeleton header
skel_h = custom_target(
  'bpf_skeleton',
  input: bpf_obj,
  output: 'xdp_tailcall.skel.h',
  command: [bpftool, 'gen', 'skeleton', '@INPUT@'],
  capture: true, # writes stdout to file
)

# 7. User-space Loader Executable
executable(
  'xdp_tailcall',
  sources: ['xdp_tailcall.c', skel_h],
  include_directories: include_directories('.'),
  dependencies: [
    libbpf_dep,
    libelf_dep,
  ],
  install: true,
)
