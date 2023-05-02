# Byte code documentation

# Table of content
 - [Byte code instructions](#byte-code-instructions)
     - [Data types](#data-types)
         - [Number](#number)
         - [String](#string)
         - [Variable](#variable)
         - [Stack](#stack)
         - [Boolean](#boolean)
     - [Instructions](#instructions)
         - [General syntax](#general-syntax)
         - [Assign](#assign)
         - [Push on stack](#push-on-stack)
         - [Call function](#call-function)
         - [Clear stack](#clear-stack)
         - [Jump](#jump)
             - [Reducible to "true" or "false"](#reducible-to-true-or-false)
             - [Jump destination](#jump-destination)
             - [Cursor move behavior](#cursor-move-behavior)
             - [REDE_CODE_JUMP](#rede_code_jump)
             - [REDE_CODE_JUMP_IF](#rede_code_jump_if)
             - [REDE_CODE_JUMP_IF_NOT](#rede_code_jump_if_not)
         - [End](#end)
     - [Example programs](#example-programs)
         - [Sum of 2 numbers](#sum-of-2-numbers)
         - [Simple if-condition](#simple-if-condition)
         - [Loop](#loop)
 - [Refs]($refs)

# Byte code instructions
## Data types
Syntax:

**REDE_TYPE    ...ARGS**
 - **REDE_TYPE** specifies the format of the following data

### Number
Represents 4-byte float.

**REDE_TYPE_NUMBER....VALUE**
 - **REDE_TYPE_NUMBER** - **1 byte** - **0x00**
 - **VALUE** - **4 bytes** - float number in standard C format

### String
Represents string with max length of 255.

**REDE_TYPE_STRING....STRING_LENGTH....STRING_VALUE**
 - **REDE_TYPE_STRING** - **1 byte** - **0x01**
 - **STRING_LENGTH** - **1 byte** - length of the string
 - **STRING_VALUE** - **STRING_LENGTH bytes** - string bytes in ASCII format

### Variable
Represents value of variable

**REDE_TYPE_VAR....VARIABLE_NAME**
 - **REDE_TYPE_VAR** - **1 byte** - **0x02**
 - **VARIABLE_NAME** - **1 byte** - variable name

### Stack
Represents value of the last stack item

**REDE_TYPE_STACK**
 - **REDE_TYPE_STACK** - **1 byte** - **0x03**

### Boolean
Represents boolean value

**REDE_TYPE_BOOL....VALUE**
 - **REDE_TYPE_BOOL** - **1 byte** - **0x04**
 - **VALUE** - **1 byte** - **0x00** for false everything else for true

## Instructions
### General syntax
**REDE_CODE....ARGS**
 - **REDE_CODE** - **1 byte** - instruction code
 - **...ARGS** - various args for specific instruction

### Assign
Assigns data to variable

**REDE_CODE_ASSIGN NAME....REDE_TYPE**
 - **REDE_CODE_ASSIGN** - **0x00**
 - **NAME** - **1 byte** - range from **0x00** to **0xFF**
 - **REDE_TYPE** - data in one of the formats described [here](#data-types)

### Push on stack
Pushes value on the stack

**REDE_CODE_STACK_PUSH....REDE_TYPE**
 - **REDE_CODE_STACK_PUSH** - **0x01**
 - **REDE_TYPE** - data in one of the formats described [here](#data-types)

### Call function
Calls function with specified name and number of arguments from the stack.

Arguments will be taken from the stack and the result will be pushed to the stack.

**REDE_CODE_CALL....FUNCTION_NAME....ARGUMENTS_NUMBER**
 - **REDE_CODE_CALL** - **0x02**
 - **FUNCTION_NAME** - **REDE_TYPE_STRING** - function name in format of [ReDe string](#string)
 - **ARGUMENTS_NUMBER** - **1 byte** - number of arguments

### Clear stack
Clears the program stack.

**REDE_CODE_STACK_CLEAR**
 - **REDE_CODE_STACK_CLEAR** - **1 byte** - **0x03**

### Jump
This is a group of 3 instructions:
 - **REDE_CODE_JUMP** - move program cursor
 - **REDE_CODE_JUMP_IF** - move program cursor if the provided value is reducible to true
 - **REDE_CODE_JUMP_IF_NOT** - move program cursor if the provided value is reducible to false

#### Reducible to "true" or "false"
Rede types that are reducible to "true":
 - Numbers that not equal to zero
 - Strings with length greater than zero
 - Boolean true value

Rede types that are reducible to "false":
 - Numbers that equal to zero
 - Strings with length equal to zero
 - Boolean false value

If the provided type is a stack type, then the described rules are applied to the current value on the stack.

#### Jump destination
Represents a relative cursor move.

**DIRECTION....CURSOR_MOVE**
 - **DIRECTION** - **1 byte** - **0x00** to move forward and anything else to move backwards
     - **REDE_DIRECTION_FORWARD** - **0x00**
     - **REDE_DIRECTION_BACKWARDS** - **0x01**
 - **CURSOR_MOVE** - **2 bytes** - number of bytes to move, in format of C integer. high byte goes first
     - **0x01 0x00** - 1
     - **0x00 0x01** - 256 

#### REDE_CODE_JUMP
Move program cursor

**REDE_CODE_JUMP....JUMP_DESTINATION**
 - **REDE_CODE_JUMP** - **0x04**
 - **JUMP_DESTINATION** - bytes in format of [Jump destination](#jump-destination)

#### REDE_CODE_JUMP_IF
Move program cursor if the provided value is reducible to true.

**REDE_CODE_JUMP_IF....REDE_TYPE....JUMP_DESTINATION**
 - **REDE_CODE_JUMP_IF** - **0x05**
 - **REDE_TYPE** - condition value in one of the formats described [here](#data-types)
 - **JUMP_DESTINATION** - bytes in format of [Jump destination](#jump-destination)

#### REDE_CODE_JUMP_IF_NOT
Move program cursor if the provided value is reducible to false.

**REDE_CODE_JUMP_IF_NOT....REDE_TYPE....JUMP_DESTINATION**
 - **REDE_CODE_JUMP_IF_NOT** - **0x06**
 - **REDE_TYPE** - condition value in one of the formats described [here](#data-types)
 - **JUMP_DESTINATION** - bytes in format of [Jump destination](#jump-destination)

#### Cursor move behavior
Cursor shift counts from the last byte of the instruction and it doesn't include current byte.

##### Examples
Forward jump
```c
unsigned char bytes[] = {
    REDE_CODE_JUMP, REDE_DIRECTION_FORWARD, 0x1E, 0x00,
    1, 2, 3, 4, 5, 6,
    1, 2, 3, 4, 5, 6,
    1, 2, 3, 4, 5, 6,
    1, 2, 3, 4, 5, 6,
    1, 2, 3, 4, 5, 6,
    REDE_CODE_STACK_PUSH, REDE_TYPE_STRING, 3, 'h', 'i', '!',
    REDE_CODE_CALL, 3, 'l', 'o', 'g', 1,
    REDE_CODE_END
};
```
Program cursor will move for 30 bytes from the last byte of the jump instruction (**0x00**) and will point to REDE_CODE_STACK_PUSH instruction.

Backward jump:
```c
unsigned char bytes[] = {
    REDE_CODE_STACK_PUSH, REDE_TYPE_STRING, 3, 'h', 'i', '!',
    REDE_CODE_CALL, 3, 'l', 'o', 'g', 1,
    REDE_CODE_STACK_CLEAR,
    REDE_CODE_JUMP, REDE_DIRECTION_BACKWARD, 0x0F, 0x00,
    REDE_CODE_END
};
```
Here we are creating an infinite loop with backward jump. We jump 15 bytes back from the last byte of the instruction(**0x00**) to REDE_CODE_STACK_PUSH

In general you can thing about the jump length as the number of bytes between start and finish.

if we have a list of numbers:
> 1 2 3 4 5 6 7 8 9

and we want to jump from 1 to 9 we need to pass 7 numbers and it also work in the reversed direction: we need to pass 7 numbers to jump from 9 to 1.

### End
Should be at the end of the program.

**REDE_CODE_END**
 - **REDE_CODE_END** - **1 byte** - **0xFF**

## Example programs
### Sum of 2 numbers
```c
unsigned char program[] = {
    REDE_CODE_ASSIGN, 0, REDE_TYPE_NUMBER, 0, 0, 0, 64,     // assign 2.0f to index 0
    REDE_CODE_ASSIGN, 1, REDE_TYPE_NUMBER, 0, 0, 64, 64,    // assign 3.0f to index 1
    REDE_CODE_STACK_PUSH, REDE_TYPE_VAR, 0,                 // put data from index "0" on the stack
    REDE_CODE_STACK_PUSH, REDE_TYPE_VAR, 1,                 // put data from index "1" on the stack
    REDE_CODE_CALL, 3, 's', 'u', 'm', 2,                    // call function "sum" with 2 arguments from the stack and put the result back
    REDE_CODE_ASSIGN, 2, REDE_TYPE_STACK,                   // take last item from the stack and assign it to index 2
    REDE_CODE_STACK_PUSH, REDE_TYPE_VAR, 0,                 // put data from index "0" on the stack
    REDE_CODE_STACK_PUSH, REDE_TYPE_STRING, 1, '+',         // put string "+" on the stack
    REDE_CODE_STACK_PUSH, REDE_TYPE_VAR, 1,                 // put data from index "1" on the stack
    REDE_CODE_STACK_PUSH, REDE_TYPE_STRING, 1, '=',         // put string "=" on the stack
    REDE_CODE_STACK_PUSH, REDE_TYPE_VAR, 2,                 // put data from index "2" on the stack
    REDE_CODE_CALL, 3, 'l', 'o', 'g', 5,                    // call function "log" with 5 arguments from the stack and put the result back
    REDE_CODE_END                                           // end the program
};
```

### Simple if-condition
The program prints "true" if variable "0" is reducible to true, and prints "false" if it is not
```c
unsigned char program[] = {
    REDE_CODE_ASSIGN, 0, REDE_TYPE_BOOL, 1,                                         // 1.  Assign variable 0 to true (condition)
    REDE_CODE_JUMP_IF_NOT, REDE_TYPE_VAR, 0, REDE_DIRECTION_FORWARD, 0x12, 0x00,    // 2.  Jump 18 bytes from the end of the instruct if variable 0 is reducible to false (Line 7)
    REDE_CODE_STACK_PUSH, REDE_TYPE_STRING, 4, 't', 'r', 'u', 'e',                  // 3.  Put string "true" on the stack
    REDE_CODE_CALL, 3, 'l', 'o', 'g', 1,                                            // 4.  Call function log
    REDE_CODE_STACK_CLEAR,                                                          // 5.  Clear the stack
    REDE_CODE_JUMP, REDE_DIRECTION_FORWARD, 0x0F, 0x00,                             // 6.  Jump 15 bytes from the end of the instruction(Line 10)
    REDE_CODE_STACK_PUSH, REDE_TYPE_STRING, 5, 'f', 'a', 'l', 's', 'e',             // 7.  Put string "false" on the stack
    REDE_CODE_CALL, 3, 'l', 'o', 'g', 1,                                            // 8.  Call function log
    REDE_CODE_STACK_CLEAR,                                                          // 9.  Clear the stack
    REDE_CODE_END                                                                   // 10. End the program
};
```

### Loop
This program iterates over numbers from 0 to 10 and prints "even" for evens and "odd" for odds
```c
unsigned char program[] = {
    REDE_CODE_ASSIGN, 0, REDE_TYPE_NUMBER, 0, 0, 0, 0,                              // 1.  Assign 0 to variable 0. Initializing the counter
                                                                                    // 2.  
    REDE_CODE_STACK_PUSH, REDE_TYPE_VAR, 0,                                         // 3.  Put value from variable 0 on the stack
    REDE_CODE_STACK_PUSH, REDE_TYPE_NUMBER, 0, 0, 32, 65,                           // 4.  Put 10 on the stack
    REDE_CODE_CALL, 2, 'e', 'q', 2,                                                 // 5.  Call function eq to check if the provided values are equal
                                                                                    // 6. 
    REDE_CODE_JUMP_IF, REDE_TYPE_STACK, REDE_DIRECTION_FORWARD, 59, 0,              // 7.  Jump 59 bytes forward if values are equal (Line 26)
    REDE_CODE_STACK_PUSH, REDE_TYPE_VAR, 0,                                         // 8.  Put value from variable 0 on the stack 
                                                                                    // 9.
    REDE_CODE_STACK_PUSH, REDE_TYPE_VAR, 0,                                         // 10. Put value from variable 0 on the stack 
    REDE_CODE_CALL, 4, 'e', 'v', 'e', 'n', 1,                                       // 11. Call function "even" to check that the value is even
                                                                                    // 12.
    REDE_CODE_JUMP_IF_NOT, REDE_TYPE_STACK, REDE_DIRECTION_FORWARD, 11, 0,          // 13. If it is not even, then jump forward 11 instructions (Line 16)
    REDE_CODE_STACK_PUSH, REDE_TYPE_STRING, 4, 'e', 'v', 'e', 'n',                  // 14. Put string "even" on the stack
    REDE_CODE_JUMP, REDE_DIRECTION_FORWARD, 6, 0,                                   // 15. Jump forward 6 instructions (Line 18)
    REDE_CODE_STACK_PUSH, REDE_TYPE_STRING, 3, 'o', 'd', 'd',                       // 16. Put string "odd" on the stack
                                                                                    // 17.
    REDE_CODE_CALL, 3, 'l', 'o', 'g', 2,                                            // 18. Call function "log" with 2 arguments
    REDE_CODE_STACK_CLEAR,                                                          // 19. Clear the stack
                                                                                    // 20. 
    REDE_CODE_STACK_PUSH, REDE_TYPE_VAR, 0,                                         // 21. Put value from variable 0 on the stack
    REDE_CODE_CALL, 4, 'i', 'n', 'c', 'r', 1,                                       // 22. Call function "incr" to get provided value incremented by 1
    REDE_CODE_ASSIGN, 0, REDE_TYPE_STACK,                                           // 23. and assign it to variable 0
                                                                                    // 24.
    REDE_CODE_JUMP, REDE_DIRECTION_BACKWARD, 76, 0,                                 // 25. Jump backward 76 instructions to the beginning of the loop (Line 3)
    REDE_CODE_END                                                                   // 26. End the program
};
```

# Refs
 - [ASCII Table](https://www.cs.cmu.edu/~pattis/15-1XX/common/handouts/ascii.html)