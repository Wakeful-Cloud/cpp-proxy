# ptrace
A static-library proxy based on `ptrace` (X86-only)

## Documentation

### Running
1. Run without the proxy:
```bash
task run:no-proxy
```
2. Run with the proxy:
```bash
task run:proxy
```

### Static Library Interception
Using [`/proc/[PID]/maps`](https://man7.org/linux/man-pages/man5/proc.5.html) and
[`libelf`](https://sourceware.org/elfutils/), it's possible to calculate a symbol's address in
memory (Note: this should even work with ASLR). Once a symbol's address is known,
[`ptrace`](https://man7.org/linux/man-pages/man2/ptrace.2.html) can be used to insert debugging
trap/breakpoint instruction inside of the target function. Then, whenever the target function is
called, the process emits a `SIGTRAP` which is easily detected by the parent. See
[Eli Bendersky's website](https://eli.thegreenplace.net/2011/01/27/how-debuggers-work-part-2-breakpoints)
for more information.