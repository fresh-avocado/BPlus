# B+-Tree

## Run

```bash
# for macOS
g++ -std=c++2a main.cpp -DCACHELINESIZE=`sysctl -a | grep "hw.cachelinesize" | awk '{ print $2 }'`
# for Linux
g++ -std=c++2a main.cpp -DCACHELINESIZE=`getconf LEVEL1_DCACHE_LINESIZE`
```

## Notes

This is still a work in progress.
