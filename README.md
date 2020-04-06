# tyr

## Build Instructions
Current run command:
`g++-8 tyr.cpp filemenu.cpp -lncurses -lpanel -lmenu -lzmq -pthread -std=c++17 -lstdc++fs`

## Plugins
The default ipc path that tyr communicates on is `/tmp/tyrplugins.ipc`, although this can be changed via the config file for tyr (eventually). Tyr uses zeromq to communicate over ipc, specifically a REP/REQ set of sockets. An example connection script for Python is as follows:
```python
import zmq
import time

# set up the zmq context
context = zmq.Context()
# create the socket
socket = context.socket(zmq.REQ)
# connect to the ipc socket that tyr has made on the other end
socket.connect("ipc:///tmp/tyrplugins.ipc")
while (True):
	socket.send_string("Whats poppin")
	response = socket.recv().decode('UTF-8')
	print(response)
	time.sleep(3)
# close the connection and terminate the zmq context
socket.close()
context.term()
```
The plugin-tyr interfacing works as follows, the plugin will send tyr a command string via ipc, and tyr will respond to the string with the requested information (in the case that the string was a request for information from the editor) or an acknowledgment (in the case that the plugin was changing something in the editor). The format for command strings is given below.
### Command String Formatting
All of the command strings are composed of a series of characters that identify what string represents, with comma delimiters.
#### First Character
The first character determines whether the string is an accessor or a mutator, with accessors obtaining information from the editor and mutators sending new information to the editor.

|Character|Meaning|
|---|---|
|`g` | Represents a "getter" (accessor)|
|`s`|Represents a "setter" (mutator)|
#### Second Item
The second item in the string represents what is being accessed or modified.

|Item| Meaning |
|---|---|
|`ed_curs`|Accessing or setting the cursor position in the editor|
|`file_curs`|Accessing or setting the cursor selection in the file viewer|
|`ed_char`|Accessing or setting the character at a location in the editor|
|`filename`|Accessing the name of the current file (This cannot be used via a setter)|
|`num_lines`|Accessing the number of lines in the current file (This cannot be used via a setter)|
|`file_down`|Moves the file viewer cursor down one item (Can only be used via setter)|
|`file_up`|Moves the file viewer cursor up one item (Can only be used via setter)|
|`file_expand`|Attempts to expand the current selection in the file viewer (Can only be used via setter)|
|`file_collapse`|Attempts to collapse the current selection in the file viewer (Can only be used via setter)|

#### Arguments
The rest of the items in the string are arguments that are necessary for setters. The necessary arguments are shown below for the different setters.
