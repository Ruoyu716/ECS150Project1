# ECS150 Project1 Report

## Overview
In project1, we are supposed to implement 3 Built-in commands, output 
redirection, piping, error management, standard error redirection and directory 
stack. We made 3 built-in commands and output redirection working perfectly, 
piping and error management working partially, but we missed two extra features 
in project1.

### Parsing Command Line
The program will read in user input command and based on what meta symbols(>,>&,
 l&, etc.) it contains, parse the command to its proper splitter that splits 
command into command keyword, command arguments or output file. Also, the 
program will use a struct to hold command property. (real_cmd, arguments, etc.)
* see line 15-87.

### Special Case Handling
For output redirection, there might be some white spaces surround the meta 
symbol “>” and in order to display and transfer correct information, our code 
will eliminate those redundant whitespaces by using function memmove().
* see line 105 - 122 

### Dealing WIth Directory
For built-in commands **pwd** and **cd**, the program should be able to find 
current directory and change it to specified directory as needed. To find 
current directory, the program simply uses function getcwd(), and to change the 
directory, the program will find the new directory name by parsing the command 
and path got from getcwd(), and use chdir() to make changes.
* see line 188-195 
* see line 236-249

### Output Redirection
Our program treats command to redirect output as 4 parts: command keywords, 
command text, meta symbol and destination file name. Then use fopen() and 
open() to create and open declared file. To write into file, the program will 
use dup2() to close STDIN_FILENO.
* see line 199-229

### Piping and Extra Features
Our piping implementation is followed what we learned from lecture and since it 
does not work as expected, I just want to point out one thing that we tried to 
use a char** to store piping command(char*|char*|char*) and then parse it to 
proper command splitter. Since there is not much time left, we do not finish 
this implementation and also, we do not finish extra features implementation.
* see line 70 - 87
* see line 261 - 323

### Source
[remove given index character from string](https://stackoverflow.com/questions/5457608/how-to-remove-the-character-at-a-given-index-from-a-string-in-c)
[structure pointer](https://www.programiz.com/c-programming/c-structures-pointers)
[convert integer to string](https://stackoverflow.com/questions/8257714/how-to-convert-an-int-to-string-in-c)
* Sources are also commented in code.

