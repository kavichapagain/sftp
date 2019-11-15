# Simple File Transfer Protocol - sftp

## server - myftpd

Server is named as myftpd. The 'd' in myftp means it runs as a daemon process.

### Run server

* `cd myftpd`
* `make`
* `./myftpd`


## client - myftp

`myftp` serves as a client. It takes following commands:

* `pwd` - to display the current directory of the server that is serving the client
* `lpwd` - to display the current directory of the client
* `dir` - to display the file names under the current directory of the server that is serving the client
* `ldir` - to display the file names under the current directory of the client
* `cd <directory_pathname>` - to change the current directory of the server that is serving the client
* `lcd <directory_pathname>` - to change the current directory of the client
* `get <filename>` - to download the named file from the current directory of the remote server and save it in the current directory of the client
* `put <filename>` - to upload the named file from the current directory of the client to the current directory of the remove server
* `quit` - to terminate the myftp session


### Run client

* `cd myftp`
* `make`
* `./myftp`



- - - -

*ICT374 - Operating Sytems and System Programming **(Project)***

