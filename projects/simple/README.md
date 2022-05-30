# Simple
Simple C++ shared-library interception prototype

## Documentation

### Running
1. Build and run:
```bash
task run
```

### Mangled Names
Since the target function and proxy function names should always be identical (Since they have the
same signature), retrieving the most recent backtrace-entry inside the proxy function (for itself)
yields the mangled name of the target function.