# Project 1
**tasks**: 
* Write a simple shell
* Run the shell in Linux
* Run the shell in Minix 3
  
### Compiling instruction
```shell
make
```

### Current features
1. Write a working read/parse/execute loop and an exit command;
2. Handle single commands without arguments (e.g. ls);
3. Support commands with arguments (e.g. apt-get update or pkgin update);
4. File I/O redirection:
   
    4.1 Output redirection by overwriting a file (e.g. echo 123 > 1.txt);

    4.2. Output redirection by appending to a file (e.g. echo 465 >> 1.txt);

    4.3. Input redirection (e.g. cat < 1.txt);

    4.4. Combine 4.1 and 4.2 with 4.3;
5. Support for bash style redirection syntax (e.g. cat < 1.txt 2.txt > 3.txt 4.txt);
