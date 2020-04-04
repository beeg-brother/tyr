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
