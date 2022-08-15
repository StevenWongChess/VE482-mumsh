*Big Picture of mumsh:*

​    *There are several different standards and implementations for shell.*

​    *Q: What's the difference between sh, bash, and zsh?*

​    *A: Think about C, C++, and Modern C++*

​    *POSIX Standard: https://pubs.opengroup.org/onlinepubs/9699919799/utilities/contents.html*

​    *- 2.1 Shell Introduction*

​    *- 2.3 Token Recognition*

​    *A minimal POSIX shell: https://github.com/emersion/mrsh*

​    *Bash: https://www.gnu.org/software/bash/manual/bash.html#What-is-Bash_003f*

###### A complete tutorial on how to implement a Shell: 

-   [x] 1, read/parse/execute loop, `exit` command (5')

    -   Command line at most `1024` char

    -   **NO** lex or yacc allowed

    -   Escape char like `\033` **NOT** handled

    -   NOTE: good structure for future extensions/modifications

    -   **NO** need to handle `exit` with pipe, **BUT** how?

    -   How to read a line till `enter`?

        -   ```c
            char *fgets(char *str, int count, FILE *stream)
            ```

        -   Read at most `count - 1` char, append `\0` at end if no errors

            when meet `\n` stop and store `\n`, when meet `EOF`, 

            Return `str` on success, `nullptr `on failure

        -   ```c
            size_t strlen( const char *str )
            ```

    -   How to **parse**?

        My idea is to avoid using `strtok` because the complex logic design

        ```c++
        // Sample structure
        do{
        	read_line() // store into tokens
          parse() // parse tokens into command table
        	execute_command() // deal with redirection, pipe
          garbage_collection()
        }while(true)
        ```

-   [x] 2&3, single command without & with args `e.g. ls, ls -a` (5' + 5')

    -   Idea: `fork()` and then do `execvp()` in child

    -   *Functions in the exec() family have different behaviours:*

        ​    *l : arguments are passed as a list of strings to the main()*

        ​    *v : arguments are passed as an array of strings to the main()*

        ​    *p : path/s to search for the new running program*

        ​    *e : the environment can be specified by the caller*

    -   `fork()` 

        -   success, return pid for parent, 0 for child

        -   fail, return -1 for parent

-   [x] 4, IO redirection (support `>`, `>>`, `<`) (5' + 5' + 5' + 2')
    -   `dup2/dup()` 
    -   `open()` & `close()`
    -   !!! Remember to recover the stdout & stdin
    -   What will happen when we have both `>` and `>>` ?
    
-   [x] 5, bash style (8')
    
    -   No ` ` need after `>`, `>>`, `<`  
    -   multi space are ignored
    -   `>`, `>>`, `<` can happen anywhere 

------------------------- Milestone 1 => 40' so far----------------------

-   [x] 6, pipe (5 + 5 + 5 + 10')

    -   ```c++
        // to create pipe
        in fd[2];
        pipe();
        ```

    -   Run in parallel + cascade pipes => use for loop

    -   Work with redirection (**NOTE**: `>`, `>>`, and `<` can be anywhere)

        => here I reorg the code (i.e. add `Tokens` struct between buffer and `command_table`)

-   [x] 7, `Ctrl-D`

    -   unfinished command => nothing (but what about double `ctrl-d` ?)
    -   Empty command => exit
    -   When using `fgets()` and `feof()`, =>  double `ctrl-d` will cause bug
        -   If **EOF** is entered other than at the beginning of the line, the results are unspecified.

-   [x] 8, Built-in `pwd`, `cd` 

    -   **NO** need to launch new process
    -   Support `cd -` (use a global variable `OLDPWD`), `cd ~`
    -   Where to write:
        -   `pwd` => in `execute()`, before `fork()`
        -   `cd` => either near `pwd`, or at the beginning of `parse()`

-   [ ] 9, `Ctrl-C`

    -   

###### To Compile

```bash
mkdir cmake-build-debug && cd cmake-build-debug
cmake -DCMAKE_C_COMPILER=clang .
make
```

















