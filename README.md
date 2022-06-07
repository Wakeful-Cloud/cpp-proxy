# C++ Proxy
C++ shared library proxy prototypes

## Projects
* `ld-preload`: a shared-library proxy based on `LD_PRELOAD`
* `ld-preload-template`: a shared-library proxy based on `LD_PRELOAD` with C++ templates
* `ptrace`: a static-library proxy based on `ptrace`

## Documentation

### Docker
1. Install [Docker](https://docs.docker.com/get-docker/)
2. Build:
```bash
docker build . -t cpp-proxy
```
3. Run:
```bash
docker run -it -v "$(pwd):/home/dev/cpp-proxy" --name cpp-proxy cpp-proxy
```
4. Notes:
  * The default user (`dev`) has password-less sudo setup and uses [Zsh](https://www.zsh.org)
  * The `/home/dev/cpp-proxy` directory in the container is mapped to the repository root directory on the host