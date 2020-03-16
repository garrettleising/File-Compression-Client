# Running the program:

    $make encode
    $./encode -v -i file_name -o file_name

- You can run -v to get the statistics of the program after it finishes running. This includes how large the file is before and after compression. It also includes the percent difference between the two.
- You can run -i to change the file to compress. By default it reads from STDIN.
- You can run -o to change the file to write to. By default it reads from STDOUT.
```
    $make decode
    $./decode -v -i file_name -o file_name
```
- You can run -v to get the statistics of the program after it finishes running. This includes how large the file is before and after decompression. It also includes the percent difference between the two.
- You can run -i to change the file to decompress. It must be a file that was compressed originally. By default it reads from STDIN.
- You can run -o to change the file to write to. By default it reads from STDOUT.

# Makefile usage:

- Type `make` or `make all` into the terminal to compile both encode and decode and create a binary for both

- Type `make encode` into the terminal to compile encode and create a binary for it

- Type `make decode` into the terminal to compile decode and create a binary for it

- Type `make clean` to delete all the directories and files created using `make` and `make infer`

- Type `make format` to format all *.c files

- Type `make infer` to use infer and check for any errors

- Type `make valgrind` to check for any memory leaks in the program

# Infer Errors:

- For my decode program, it says that I have a memoryleak from a fileheader. This happens even though I freed the memory and valgrind shows no memory leaks.
