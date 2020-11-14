# Description:
Synchronous - Blocking
Several clients are connecting to the server (number of clients - server parameter via command line argument 1-5)
Everything that clients send in turn (1 2 3 1 2 3) is printed on the screen by 1 byte (letters) in the format:
client: letter

## Example:
```
client 1 send message: "hello"
client 2 send message: "hi"
server:
    1: h
    2: h
    1: e
    2: i
    1: l
    -Freeze
```
