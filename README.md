#Description
If you have to use a shell that sucks like spim or jdb and doesn't come 
with the ability to clear the screen. Or is missing features likea history and
editing the first line.

# Installing
Make sure that in the make file, the location for install and uninstall are correct.
You just want to make sure that you install to a place on your path. To see path type 'echo $PATH'.
```
make
make install
```

# Usage
```
ishell <bad shell> <arguments>
```

# Example 1
```
./a.out mips
```

# Example 2
```
./a.out jdb ClassFile
```

# Uninstall
It just rm /usr/local/bin/ishell.
```
make uninstall
```

