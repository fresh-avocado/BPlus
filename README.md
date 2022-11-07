# B+-Tree

## Run

```bash
# for macOS
g++ -std=c++2a main.cpp -DALIGNED_MEM -DCACHELINESIZE=`sysctl -a | grep "hw.cachelinesize" | awk '{ print $2 }'`
# for Linux
g++ -std=c++2a main.cpp -DALIGNED_MEM -DCACHELINESIZE=`getconf LEVEL1_DCACHE_LINESIZE`
```

## Notes

This is still a work in progress.
