# Hi there!
This is a simple scripting language ReDe

# Table of content
 - [Byte code instructions](#byte-code-instructions)
     - [Data types](#data-types)
         - [Number](#number)
         - [String](#string)
         - [Variable](#variable)
         - [Stack](#stack)
     - [Instructions](#instructions)
         - [General syntax](#general-syntax)
         - [Assign](#assign)
         - [Push on stack](#push-on-stack)
         - [Call function](#call-function)
         - [End](#end)
     - [Example programs](#example-programs)
         - [Sum of 2 numbers](#sum-of-2-numbers)

# Byte code instructions
## Data types
Syntax:

**REDE_TYPE ...ARGS**
 - **REDE_TYPE** specifies the format of the following data

### Number
Represents 4-byte float.

**REDE_TYPE_NUMBER VALUE**
 - **REDE_TYPE_NUMBER** - **1 byte** - **0x00**
 - **VALUE** - **4 bytes** - float number in standard C format

### String
Represents string with max length of 255.

**REDE_TYPE_STRING STRING_LENGTH STRING_VALUE**
 - **REDE_TYPE_STRING** - **1 byte** - **0x01**
 - **STRING_LENGTH** - **1 byte** - length of the string
 - **STRING_VALUE** - **STRING_LENGTH bytes** - **NULL TERMINATED** string chars in ASCII format

### Variable
Represents value of variable

**REDE_TYPE_VAR VARIABLE_NAME**
 - **REDE_TYPE_VAR** - **1 byte** - **0x02**
 - **VARIABLE_NAME** - **1 byte** - variable name

### Stack
Represents value of the last stack item

**REDE_TYPE_STACK**
 - **REDE_TYPE_STACK** - **1 byte** - **0x03**

## Instructions
### General syntax
**REDE_CODE ...ARGS**

### Assign
Assigns data to variable

**REDE_CODE_ASSIGN NAME REDE_TYPE**
 - **REDE_CODE_ASSIGN** - **1 byte** - **0x00**
 - **NAME** - **1 byte** - range from **0x00** to **0xFF**
 - **REDE_TYPE** - data in one of the formats described [here](#data-types)

### Push on stack
Pushes value on the stack

**REDE_CODE_STACK_PUSH REDE_TYPE**
 - **REDE_CODE_STACK_PUSH** - **1 byte** - **0x01**
 - **REDE_TYPE** - data in one of the formats described [here](#data-types)

### Call function
Calls function with specified name and number of arguments from the stack.

Arguments will be taken from the stack and the result will be pushed to the stack.

**REDE_CODE_CALL FUNCTION_NAME ARGUMENTS_NUMBER**
 - **REDE_CODE_CALL** - **1 byte** - **0x02**
 - **FUNCTION_NAME** - **REDE_TYPE_STRING** - function name in format of [ReDe string](#string)
 - **ARGUMENTS_NUMBER** - **1 byte** - number of arguments

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
    REDE_CODE_STACK_PUSH, REDE_TYPE_VAR, 0,                 // push data from index "0" on the stack
    REDE_CODE_STACK_PUSH, REDE_TYPE_VAR, 1,                 // push data from index "1" on the stack
    REDE_CODE_CALL, 4, 's', 'u', 'm', '\0', 2,              // call function "sum" with 2 arguments from the stack and push the result back
    REDE_CODE_ASSIGN, 2, REDE_TYPE_STACK,                   // take last item from the stack and assign it to index 2
    REDE_CODE_STACK_PUSH, REDE_TYPE_VAR, 0,                 // push data from index "0" on the stack
    REDE_CODE_STACK_PUSH, REDE_TYPE_STRING, 2, '+', '\0',   // push string "+" on the stack
    REDE_CODE_STACK_PUSH, REDE_TYPE_VAR, 1,                 // push data from index "1" on the stack
    REDE_CODE_STACK_PUSH, REDE_TYPE_STRING, 2, '=', '\0',   // push string "=" on the stack
    REDE_CODE_STACK_PUSH, REDE_TYPE_VAR, 2,                 // push data from index "2" on the stack
    REDE_CODE_CALL, 4, 'l', 'o', 'g', '\0', 5,              // call function "log" with 5 arguments from the stack and push the result back
    REDE_CODE_END                                           // end the program
};
```