# ebpf-tailcall-example
> This project contains C source code to execute multiple functions via eBPF tail calls when a packet arrives.

<img width="500" height="160" alt="image" src="https://github.com/user-attachments/assets/0b80e0d4-89e4-445e-be73-6b7983afb9c2" />

### Build
This project is built with [Meson (the build system)](https://mesonbuild.com/index.html).
It requires an eBPF-supported Linux kernel (modern Linux kernels almost support them! :D), `bpftool`, `clang`, `llvm` (I used LLVM 18), `libbpf`, and `libelf`.

1. Create the build directory with `meson setup builddir --native-file=clang.ini`
2. Inside the build directory(`/builddir`), build the project with `meson compile`.
3. After that, execute the program via `sudo ./xdp_tailcall <NIC interface name>`.
<img width="500" height="500" alt="image" src="https://github.com/user-attachments/assets/84476df5-d005-4c70-a6c3-4ccf29c43d1e" />

Print the trace pipe(`/sys/kernel/debug/tracing/trace_pipe`) and you'll be able to observe program outputs.
Those tail-called functions are generated upon every packet arrival at the designated NIC (Network Interface Card).
