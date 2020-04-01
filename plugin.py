# A little mini ipc test via zmq to communicate with tyr, trying to get info out of the editor
# so that it can be used for plugins
# attempts to bind to the ipc socket that tyr should be creating, and then tries to send and receive some data
# it then closes the connection

import zmq
import time

# set up the zmq context
context = zmq.Context()
# create the socket
socket = context.socket(zmq.REQ)
# connect to the ipc socket that tyr has made on the other end
socket.connect("ipc:///tmp/tyrplugins.ipc")
# send a connection message that lets tyr know that the plugin is communicating
socket.send_string("READY")
print("Sent server READY message, waiting on response...")
handshake = socket.recv().decode('UTF-8')

if (handshake == 'READY'):
	print("response received, commencing communication")
	while (True):
		time.sleep(.5)
		print("blah")

# close the connection and terminate the zmq context
socket.close()
context.term()