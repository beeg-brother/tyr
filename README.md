# tyr

## Build Instructions
Current run command:
`g++-8 src/* -o tyr -lncurses -lpanel -lzmq -pthread -std=c++17`

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
### Command String Formatting (OLD)
All of the command strings are composed of a series of characters that identify what string represents, with comma delimiters.
|Item| Meaning |
|---|---|
|`get_ed_curs`|Accessing the cursor position in the editor|
|`set_ed_curs`|Setting the cursor position in the editor|
|`get_file_curs`|Accessing the cursor selection in the file viewer|
|`set_file_curs`|Setting the cursor selection in the file viewer|
|`get_ed_char`|Accessing the character at a location in the editor|
|`set_ed_char`|Setting the character at a location in the editor|
|`get_filename`|Accessing the name of the current file (This cannot be used via a setter)|
|`get_num_lines`|Accessing the number of lines in the current file (This cannot be used via a setter)|
|`file_down`|Moves the file viewer cursor down one item (Can only be used via setter)|
|`file_up`|Moves the file viewer cursor up one item (Can only be used via setter)|
|`file_expand`|Attempts to expand the current selection in the file viewer (Can only be used via setter)|
|`file_collapse`|Attempts to collapse the current selection in the file viewer (Can only be used via setter)|

#### Arguments
The rest of the items in the string are arguments that are necessary for setters. The necessary arguments are shown below for the different setters (eventually).
