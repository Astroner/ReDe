# Hi there!
This is a simple scripting language ReDe

# Table of content
 - [Syntax](#syntax)
     - [Variables](#variables)
     - [Functions](#functions)
     - [While-loop](#while-loop)
     - [If statement](#if-statement)
     - [Comments](#comment)


# Syntax
Rede simply consists of 5 things:
 - variables
 - functions
 - while loops
 - if statements
 - comments

## Variables
To define a variable just write its name and assign a value to it:
```ts
numberVariable = 33
stringVariable = "Hi!"
anotherStringVariable = 'Hello!'
truthy = true
falsy = false
```

## Functions
In ReDe you cant define your own functions, you can only use provided functions from the environment, but function calls themselves are easy:
```ts
sum = sum(1 2)
log(sum)
```
As you see, the function arguments are separated by any whitespace in any numbers. For example, the code below is also valid code:
```ts
log(   2222    3     
4  5
)
```

## While loop
Simple conditional loop:
```ts
counter = 0
while not(eq(counter 10)) (
    log(counter)
    counter = incr(counter)
)
```
The condition goes right after while keyword and after it goes while body. 
It can be wrapped by brackets if you have multiple statements or can be just one statement

ReDe provides "break" and "continue" keywords to control loop flow:
 - break - breaks the loop
 - continue - proceeds to the next iteration


## If statement
If statements allows you to execute conditional code. It has 3 structures:
 - if - if condition is truthy then execute the code
 - else - if main condition was falsy then execute this code
 - else if - if main condition was falsy try this condition

You can chain these structures in **if** -> **else if** -> **else** order
```python
if conditionA() log("A")
else if conditionB() (
    log("A failed")
    log("b")
) else (
    loc("C")
)
```
If conditionA fails then the program checks conditionB and if even it fails else block will be executed.

As you can see the structure is really simple: first goes a keyword, then goes the condition and then goes the body, which can be a single statement or multiple ones wrapped by brackets.


## Comment
Code comment begins with **#** symbol and ends with new line or with **#**:
```python
# Log function
log(
    2 # First arg
    3 # Second arg
    # Args end
)

# Code end
```
