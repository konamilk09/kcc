# How to Run and Results
Following the tutorial, linking `foo.o` with `-static -Wl,--omagic` fails with `undefined reference to __ehdr_start`. This is because newer glibc references `__ehdr_start`. With `--omagic`, text and data are merged into a single RWE LOAD segment, and the ELF header, which `__ehdr_start` points to, is not loaded into memory.
```
$ cc -static -Wl,--omagic -o foo foo.o
/usr/bin/x86_64-linux-gnu-ld.bfd: warning: foo has a LOAD segment with RWX permissions
/usr/bin/x86_64-linux-gnu-ld.bfd: /usr/lib/gcc/x86_64-linux-gnu/15/../../../x86_64-linux-gnu/libc.a(dl-support.o): in function `_dl_aux_init':
(.text+0x17c): undefined reference to `__ehdr_start'
/usr/bin/x86_64-linux-gnu-ld.bfd: (.text+0x186): undefined reference to `__ehdr_start'
/usr/bin/x86_64-linux-gnu-ld.bfd: (.text+0x18d): undefined reference to `__ehdr_start'
/usr/bin/x86_64-linux-gnu-ld.bfd: (.text+0x19b): undefined reference to `__ehdr_start'
collect2: error: ld returned 1 exit status
```

## Solution
Explicitly define global variable `main` as executable `.text`
```
$ cc -c foo3.c
/tmp/cc66ZB9y.s: Assembler messages:
/tmp/cc66ZB9y.s:4: Warning: ignoring changed section attributes for .text
$ cc -static -o foo3 foo3.o
$ ./foo3 
$ echo $?
42
```
