# Server - Client File System

### Make commands
- `make` This command creates the compiled `client`, `server` and `parallel_client` executable files.

### How to Run
- Connect 2 USB drives named `FS1` and `FS2`
- `make` to get executable files.
- `./server` to run server
- `./client` to run client
    - Enter inputs to perform supported actions based on prompts
    - List of supported actions:
      - Read file
      - Get file info
      - Create folder
      - Create file
      - Delete file
- `./parallel_client` to run the test for a parallel client, this attempts to send 10 concurrent create file requests to the server.

### Example:
```
aiyappa@aiyappas-MacBook-Pro remote-file-system-tcp-c % ./client
Socket created successfully
Connected with server successfully
Enter choice
 1:Read file 
 2:Get file info
 3:Create folder
 4:Create file
 5:Delete file
4
Enter file path
bloor.txt
Enter file size
5
Enter data
sdfjs
Received data: PASS
aiyappa@aiyappas-MacBook-Pro remote-file-system-tcp-c % ./client
Socket created successfully
Connected with server successfully
Enter choice
 1:Read file 
 2:Get file info
 3:Create folder
 4:Create file
 5:Delete file
1
Enter size of file to read
4
Enter full path
bloor.txt
Received data: sdf
aiyappa@aiyappas-MacBook-Pro remote-file-system-tcp-c % ./client
Socket created successfully
Connected with server successfully
Enter choice
 1:Read file 
 2:Get file info
 3:Create folder
 4:Create file
 5:Delete file
4
Enter file path
somethingish.txt
Enter file size
6
Enter data
abcdef
Received data: PASS
aiyappa@aiyappas-MacBook-Pro remote-file-system-tcp-c % ./client
Socket created successfully
Connected with server successfully
Enter choice
 1:Read file 
 2:Get file info
 3:Create folder
 4:Create file
 5:Delete file
1
Enter size of file to read
somethingish.txt
Invalid choice, please try again: 5
Enter full path
somethingish.txt
Received data: abcd
```
