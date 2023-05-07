# Hi there!
This is a simple scripting language ReDe

# Table of content
 - [Syntax](#syntax)
     - [Variables](#variables)
     - [Functions](#functions)
     - [While-loop](#while-loop)


# Syntax
Rede simply consists of 3 things:
 - variables
 - functions
 - while loops

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
