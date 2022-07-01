# C++ Proxy
C++ proxy/intercept prototypes

## Prototypes
* [`ld-preload`](prototypes/ld-preload): a shared-library proxy based on `LD_PRELOAD`
* [`ld-preload-template`](prototypes/ld-preload-template): a shared-library proxy based on `LD_PRELOAD` with C++ templates
* [`ptrace`](prototypes/ptrace): a static-library proxy based on `ptrace` (X86-only)
* [`dynamorio`](prototypes/dynamorio): a static-library proxy based on [DynamoRIO](https://dynamorio.org)
* [`qbdi`](prototypes/qbdi): a static-library proxy based on [QuarkslaB Dynamic binary Instrumentation (QBDI)](https://qbdi.quarkslab.com/)
* [`eigen`](prototypes/eigen): a proxy for intercepting [Eigen](https://eigen.tuxfamily.org/) static library calls based
on [QuarkslaB Dynamic binary Instrumentation (QBDI)](https://qbdi.quarkslab.com/) (X86-only)

## Documentation

### Architecture
This repository contains Docker images for x86-64 (`amd64`) and ARM v8 64 bit (`arm64-v8`) in order
to simulate running the proxies in a variety of environments including servers, desktops, and
mobile/embedded systems. You will likely need to [setup QEMU](https://stereolabs.com/docs/docker/building-arm-container-on-x86/)
for development unless you're using Docker Desktop on non-Linux systems.

### Docker
1. Install [Docker](https://docs.docker.com/get-docker/)
2. Build:
```bash
docker build . -f amd64.dockerfile -t cpp-proxy:amd64
# Or
docker build . -f arm64-v8.dockerfile -t cpp-proxy:arm64-v8
```
3. Run:
```bash
# Note: CAP_SYS_PTRACE is required for process_vm_readv and QBDIPreload to work
docker run -it --cap-add CAP_SYS_PTRACE -v "$(pwd):/home/dev/cpp-proxy" --name cpp-proxy-amd64 cpp-proxy:amd64
# Or
docker run -it --cap-add CAP_SYS_PTRACE -v "$(pwd):/home/dev/cpp-proxy" --name cpp-proxy-arm64-v8 cpp-proxy:arm64-v8
```
4. Notes:
  * The default user (`dev`) has password-less sudo setup and uses [Zsh](https://www.zsh.org)
  * The `/home/dev/cpp-proxy` directory in the container is mapped to the repository root directory
  on the host, but you will probably need to chown it (eg: `sudo chown dev:dev /home/dev -R`)