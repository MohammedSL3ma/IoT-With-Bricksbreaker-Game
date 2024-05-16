<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>ESP32 Web-Based Game Project</title>
</head>
<body>
    <h1>ESP32 Web-Based Game Project</h1>
   <h2>ESP32 Web-Based Game Break_broke </h2>
    <p>This project involves creating a web-based game hosted on an ESP32 microcontroller. The ESP32 connects to a WiFi network and serves an interactive game through a web server using HTML and JavaScript. The game features a paddle and ball mechanism similar to classic brick-breaking games, where the paddle's movement is controlled by two IR sensors. The game is displayed on a web browser, and real-time updates are sent from the ESP32 to the client using server-sent events (SSE). The project utilizes several libraries, including WiFi.h for network connectivity, ESPAsyncWebServer.h for handling HTTP requests, AsyncTCP.h for asynchronous TCP socket functionality, and ESPmDNS.h for enabling local network device discovery. The setup function initializes the server and connects the ESP32 to the network, while the loop function continuously reads the IR sensor values to update the game state.</p>
</body>
</html>
