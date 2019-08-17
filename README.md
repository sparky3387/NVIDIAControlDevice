# NVIDIAControlDevice

Remotely control an NVIDIA shield over HTTP/Websockets 

This software runs on an ESP32 making it into a virtual keyboard running a Kodi style websocket server, this allows clients to connect via any of the standard Kodi remote control tools, such as:
* Kassi (Chrome)
* Yatse/Kore (Android)

#### Supported Hardware
* ESP32

#### Limitations
* Unfortuntely, this does not support authentication for its websocket, as the remote control applications do not currently support this, instead this relies on security through obscurity only allowing connections to the correct path and blocks a client from connecting for five minutes, if it connects to the wrong path 5 times.
* I managed to get the time for an action down to 15ms, if you know of any ways to improve this, please submit a patch.
