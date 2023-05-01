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
             - [REDE_CODE_JUMP](#rede_code_jump)
             - [REDE_CODE_JUMP_IF](#rede_code_jump_if)
             - [REDE_CODE_JUMP_IF_NOT](#rede_code_jump_if_not)
         - [End](#end)
     - [Example programs](#example-programs)
         - [Sum of 2 numbers](#sum-of-2-numbers)
         - [Loop](#iterate-over-numbers-from-0-to-9-and-call-function-a-on-evens-and-b-on-odds)
 - [Refs]($refs)

# Byte code instructions
## Data types
Syntax:

**REDE_TYPE    ...ARGS**
 - **REDE_TYPE** specifies the format of the following data

### Number
Represents 4-byte float.

**REDE_TYPE_NUMBER    VALUE**
 - **REDE_TYPE_NUMBER** - **1 byte** - **0x00**
 - **VALUE** - **4 bytes** - float number in standard C format

### String
Represents string with max length of 255.

**REDE_TYPE_STRING    STRING_LENGTH    STRING_VALUE**
 - **REDE_TYPE_STRING** - **1 byte** - **0x01**
 - **STRING_LENGTH** - **1 byte** - length of the string
 - **STRING_VALUE** - **STRING_LENGTH bytes** - string bytes in ASCII format

### Variable
Represents value of variable

**REDE_TYPE_VAR    VARIABLE_NAME**
 - **REDE_TYPE_VAR** - **1 byte** - **0x02**
 - **VARIABLE_NAME** - **1 byte** - variable name

### Stack
Represents value of the last stack item

**REDE_TYPE_STACK**
 - **REDE_TYPE_STACK** - **1 byte** - **0x03**

### Boolean
Represents boolean value

**REDE_TYPE_BOOL    VALUE**
 - **REDE_TYPE_BOOL** - **1 byte** - **0x04**
 - **VALUE** - **1 byte** - **0x00** for false everything else for true

## Instructions
### General syntax
**REDE_CODE    ...ARGS**
 - **REDE_CODE** - **1 byte** - instruction code
 - **...ARGS** - various args for specific instruction

### Assign
Assigns data to variable

**REDE_CODE_ASSIGN NAME    REDE_TYPE**
 - **REDE_CODE_ASSIGN** - **0x00**
 - **NAME** - **1 byte** - range from **0x00** to **0xFF**
 - **REDE_TYPE** - data in one of the formats described [here](#data-types)

### Push on stack
Pushes value on the stack

**REDE_CODE_STACK_PUSH    REDE_TYPE**
 - **REDE_CODE_STACK_PUSH** - **0x01**
 - **REDE_TYPE** - data in one of the formats described [here](#data-types)

### Call function
Calls function with specified name and number of arguments from the stack.

Arguments will be taken from the stack and the result will be pushed to the stack.

**REDE_CODE_CALL    FUNCTION_NAME    ARGUMENTS_NUMBER**
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

**DIRECTION    CURSOR_MOVE**
 - **DIRECTION** - **1 byte** - **0x00** to move forward and anything else to move backwards
     - **REDE_DIRECTION_FORWARD** - **0x00**
     - **REDE_DIRECTION_BACKWARDS** - **0x01**
 - **CURSOR_MOVE** - **2 bytes** - number of bytes to move, in format of C integer. high byte goes first
     - **0x01 0x00** - 1
     - **0x00 0x01** - 256 

#### REDE_CODE_JUMP
Move program cursor

**REDE_CODE_JUMP    JUMP_DESTINATION**
 - **REDE_CODE_JUMP** - **0x04**
 - **JUMP_DESTINATION** - bytes in format of [Jump destination](#jump-destination)

#### REDE_CODE_JUMP_IF
Move program cursor if the provided value is reducible to true.

**REDE_CODE_JUMP    REDE_TYPE    JUMP_DESTINATION**
 - **REDE_CODE_JUMP** - **0x05**
 - **REDE_TYPE** - condition value in one of the formats described [here](#data-types)
 - **JUMP_DESTINATION** - bytes in format of [Jump destination](#jump-destination)

#### REDE_CODE_JUMP_IF_NOT
Move program cursor if the provided value is reducible to false.

**REDE_CODE_JUMP    REDE_TYPE    JUMP_DESTINATION**
 - **REDE_CODE_JUMP** - **0x06**
 - **REDE_TYPE** - condition value in one of the formats described [here](#data-types)
 - **JUMP_DESTINATION** - bytes in format of [Jump destination](#jump-destination)

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
### Iterate over numbers from 0 to 9 and call function a() on evens and b() on odds
```c
unsigned char program[] = {
    REDE_CODE_ASSIGN, 0, REDE_TYPE_NUMBER, 0, 0, 0, 0,                          // 1.  Initialize variable 0 as counter
    REDE_CODE_STACK_PUSH, REDE_TYPE_VAR, 0,                                     // 2.  Put it on the stack
    REDE_CODE_STACK_PUSH, REDE_TYPE_NUMBER, 0, 0, 32, 65,                       // 3.  Put 10 on the stack
    REDE_CODE_CALL, 7, 'i', 's', 'E', 'q', 'u', 'a', 'l', 2,                    // 4.  Call function "isEqual"
    REDE_CODE_JUMP_IF, REDE_TYPE_STACK, REDE_DIRECTION_FORWARD, 51, 0,          // 5.  If isEqual returned "true" on the stack, then skip 51 bytes after the instruction end (To point 18)
    REDE_CODE_STACK_PUSH, REDE_TYPE_VAR, 0,                                     // 6.  Put value from 0 on the stack
    REDE_CODE_CALL, 4, 'e', 'v', 'e', 'n', 1,                                   // 7.  Call function "even"
    REDE_CODE_JUMP_IF_NOT, REDE_TYPE_STACK, REDE_DIRECTION_FORWARD, 11, 0,      // 8.  If returned value is not true skip 11 next bytes after the instruction end (To point 12)
    REDE_CODE_CALL, 1, 'a', 0,                                                  // 9.  Call function a
    REDE_CODE_STACK_CLEAR,                                                      // 10. Clear the stack after the function call
    REDE_CODE_JUMP, REDE_DIRECTION_FORWARD, 5, 0,                               // 11. Skip 5 next bytes after the instruction end (To point 14)
    REDE_CODE_CALL, 1, 'b', 0,                                                  // 12. Call function b
    REDE_CODE_STACK_CLEAR,                                                      // 13. Clear the stack after the function call
    REDE_CODE_STACK_PUSH, REDE_TYPE_VAR, 0,                                     // 14. Put value from variable 0 on the stack 
    REDE_CODE_CALL, 9, 'i', 'n', 'c', 'r', 'e', 'm', 'e', 'n', 't', 1,          // 15. Call function increment
    REDE_CODE_ASSIGN, 0, REDE_TYPE_STACK,                                       // 16. Assign returned value to variable 0
    REDE_CODE_JUMP, REDE_DIRECTION_BACKWARDS, 74, 0,                            // 17. Jump 74 bytes back (To point 2)
    REDE_CODE_END                                                               // 18. End the program
};
```

# Refs
 - [ASCII Table](https://www.cs.cmu.edu/~pattis/15-1XX/common/handouts/ascii.html)