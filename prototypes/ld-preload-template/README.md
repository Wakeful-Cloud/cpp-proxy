# LD_PRELOAD (Template)
A shared-library proxy based on `LD_PRELOAD` with C++ templates

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

### Mangled Names
Since the target function and proxy function names should always be identical (Since they have the
same signature), retrieving the most recent backtrace-entry inside the proxy function (for itself)
yields the mangled name of the target function.