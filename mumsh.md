###### Hint:

-   [ ] 10 Support quotes: [5+2+3+5]
    - [ ] 10.1. Handle single and double quotes (e.g `echo "de'f' ghi" '123"a"bc' a b c`);
    - [ ] 10.2. Extend 10.1 to support requirement 4. and subtasks 6.1 to 6.3;
    - [ ] 10.3. Extend 10.2 in the case of incomplete quotes (e.g. Input `echo "de`, hit enter and input `cd"`); 
    - [ ] 10.4. Extend 10.3 to support requirements 4. and 6., together with subtask 9.3;
-   [ ] 11 Wait for the command to be completed when encountering `>, <, or |`: [3+2] 
    -   [ ] 11.1. Support requirements 3. and 4. together with subtasks 6.1 to 6.3;
    -   [ ] 11.2. Extend 11.1 to support requirement 10.;
-   [ ] 12 Handle errors for all supported features. [10]
    -   [ ] Note: a list of test cases will be published at a later stage. Marks will be awarded based on the number of cases that are correctly handled, i.e. if only if:
    -   [ ] A precise error message is displayed (e.g. simply saying “error happened!” is not enough);
    -   [ ]  The program continues executing normally after the error is identified and handled;
-   [ ] 13 A command ending with an `&` should be run in background (background job always needs to be launched in a new process). [10]
    -   [ ] 13.1. For any background job, the shell should print out the command line, prepended with the job ID and the process ID (e.g. if the two lines `/bin/ls &` and `/bin/ls | cat &` are input the output could be the two lines `[1] (32757) /bin/ls & and [2] (32758) (32759) /bin/ls | cat & )`;
    -   [ ] 13.2. Implement the command jobs which prints a list of background tasks together with their running states (e.g. in the previous case output the two lines `[1] done /bin/ls & and [2] running /bin/ls | cat &`);



##### Presentation

###### Lab 2 Topics - To get you started with `mumsh`

1.  Quotation marks in shell scripts, e.g.

    ```
    echo "de'f' ghi" '123"a"bc' a b c
    "echo" "<
    1.'txt'"
    ```

1.  Signals and signal handler (`signal()`, handling `CTRL`+`C`)

2.  What are background processes and their output format, such as

    ```
    ls &
    ```

3.  Identification of possible errors, such as failing to fork and

    ```
    echo abc > | grep abc # syntax errors
    echooo b # unable to find command
    ```

    and error handling (`perror()` or `printf()`).





