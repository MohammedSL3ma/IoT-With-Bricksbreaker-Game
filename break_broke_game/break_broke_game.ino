#include <WiFi.h>
#include <ESPAsyncWebServer.h>      //It enables handling HTTP requests and responses 
#include <AsyncTCP.h>              //AsyncTCP provides asynchronous TCP socket functionality for ESP8266 and ESP32
#include <ESPmDNS.h>        //ESPmDNS is used for implementing Multicast DNS (mDNS) functionality on ESP8266 and ESP32 microcontrollers.
                           // mDNS enables devices on a local network to discover each other using domain names without requiring a central DNS server.

const char *ssid = "M_Salama";
const char *password = "55555555";

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

// Create an Event Source on /events .
//server uses the event source protocol on the /events URL to send updates to the client;
AsyncEventSource events("/events");  //essentially creates an SSE endpoint named events accessible at the "/events" path on your web server, allowing clients to subscribe to real-time events pushed by the server.

//IR sensor pins
const int rightIRpin = 13;
const int leftIRpin = 14;

String processor(const String &var) {

  return String();
}

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>

<head>
  <meta charset="utf-8" />
  <title>ESP32 Game - Web Server</title>
  <style>
    canvas {
      display: block;
      margin: 0 auto;
      background-color: rgb(65, 65, 65);
    }
  </style>
</head>

<body>

  <script>
    // Create canvas element and set its dimensions
    const canvas = document.createElement("canvas");
    canvas.width = 800;
    canvas.height = 600;
    document.body.appendChild(canvas);

    // Get canvas context for drawing
    const ctx = canvas.getContext("2d");

    let gameOver = false;
    let winLevel = false;
    let gameStarted = false;
    let score = 0;

    let lastTime;
    let dt = 0;

    // Create bird object
    const paddle = {
      x: canvas.width / 2 - 60,
      y: canvas.height - 20,
      width: 125,
      height: 15,
      speed: 8,
      motion: 0,
      color: "#16bef7"
    };

    let ball = {
      x: paddle.x + paddle.width / 2,
      y: paddle.y - 15,
      r: 15,
      speedX: 3,
      speedY: -4,
    }

    // Create pipes array
    let targets = [];

    const randColor = () => {
      return "#" + Math.floor(Math.random() * 16777215).toString(16).padStart(6, '0').toUpperCase();
    }

    // Function to create targets
    function createTargets() {
      for (let i = 0; i < 7; i++) {
        for (let j = 0; j < 3; j++) {
          let target = {
            x: 5 + i * 115,
            y: 75 + j * 40,
            width: 90,
            height: 22,
            color: randColor()
          }
          targets.push(target);
        }
      }
    }

    // Draw Paddle on canvas
    function drawPaddle() {
      ctx.fillStyle = paddle.color;
      ctx.fillRect(paddle.x, paddle.y, paddle.width, paddle.height);
      ctx.strokeStyle = "white"
      ctx.strokeRect(paddle.x, paddle.y, paddle.width, paddle.height);
    }

    // Update Paddle Position
    function updatePaddle() {
      paddle.x += paddle.speed * paddle.motion * (dt/10);
      //check for boundarie
      if (paddle.x < 0) {
        paddle.x = 0;
      } else if (paddle.x + paddle.width > canvas.width) {
        paddle.x = canvas.width - paddle.width;
      }
    }

    function drawBall() {
      ctx.beginPath();
      ctx.arc(ball.x, ball.y, ball.r, 0, Math.PI * 2);
      ctx.fillStyle = "white";
      ctx.fill();
      ctx.closePath();

    }
    function updateBall() {
      ball.x += ball.speedX * (dt/10);
      ball.y += ball.speedY * (dt/10);
      if (ball.x + ball.r > canvas.width || ball.x - ball.r < 0) {
        ball.speedX *= -1;
      }
      if (ball.y - ball.r < 0) {
        ball.speedY *= -1;
      }
      if (ball.y > canvas.height) {
        gameOver = true;
      }
    }

    // Draw Targets on canvas
    function drawTargets() {

      for (let i = 0; i < targets.length; i++) {
        ctx.fillStyle = targets[i].color;
        ctx.fillRect(targets[i].x, targets[i].y, targets[i].width, targets[i].height);
        ctx.strokeStyle = "white"
        ctx.lineWidth = 2;
        ctx.strokeRect(targets[i].x, targets[i].y, targets[i].width, targets[i].height);
      }
    }

    // Check for collision between (ball and targets), (ball and paddle)
    function checkCollision() {
      for (let i = 0; i < targets.length; i++) {

        if (ball.x + ball.r > targets[i].x && ball.x - ball.r < targets[i].x + targets[i].width && ball.y - ball.r < targets[i].y + targets[i].height && ball.y + ball.r > targets[i].y) {
          ball.speedY *= -1;
          targets.splice(i, 1);
          score++;
        }
      }

      if (ball.x + ball.r >= paddle.x && ball.x - ball.r <= paddle.x + paddle.width && ball.y + ball.r >= paddle.y) {
        if (ball.x + ball.r - paddle.x < 2 || (paddle.x + paddle.width) - (ball.x - ball.r) < 2) {
          ball.speedX *= -1;
        } else {
          ball.speedY *= -1;
        }
      }

    }
    function displayScore() {
      ctx.fillStyle = 'white';
      ctx.font = "28px tahoma";
      ctx.fillText("score: " + score, 15, 25);
    }

    function resetGame() {
      paddle.x = canvas.width / 2 - 60;
      paddle.y = canvas.height - 20;
      ball = {
        x: paddle.x + paddle.width / 2,
        y: paddle.y - 15,
        r: 15,
        speedX: 3,
        speedY: -4,
      }

      targets = [];
      createTargets();
      gameOver = false;
      winLevel = false;
      gameStarted = false;
      score = 0;
    }

    //Add Event Listener to Arrow Keys
    document.onkeydown = function (e) {
      if (e.key == 'ArrowLeft') {
        gameStarted = true;
        paddle.motion = -1;
      } else if (e.key == 'ArrowRight') {
        gameStarted = true;
        paddle.motion = 1;
      }
    };
    document.onkeyup = function (e) {
      if (e.key == 'ArrowLeft') {
        paddle.motion = 0;
      } else if (e.key == 'ArrowRight') {
        paddle.motion = 0;
      }
    };

    //Add event listener for resetting game
    document.addEventListener('keydown', function (e) {
      if (e.code =='KeyR') {
        resetGame();
      }
    });


    createTargets();

    // Game loop function
    function gameLoop(currentTime) {
      requestAnimationFrame(gameLoop);
      //Get the delay time between callbacks of gameLoop
      if(!lastTime){
        lastTime = currentTime;
      }
      dt = currentTime-lastTime;
      lastTime= currentTime;

      if (gameOver) {
        //Show a Game Over Text
        ctx.fillStyle = 'red';
        ctx.font = "35px tahoma";
        ctx.fillText("Game Over :(", canvas.width / 3, canvas.height * 0.4);
        ctx.fillText(" press R to replay", canvas.width / 3.5, canvas.height * 0.5);
        return;
      }
      if (winLevel) {
        //Show a Win Level Text
        ctx.fillStyle = 'green';
        ctx.font = "35px tahoma";
        ctx.fillText("You Win :)", canvas.width / 3, canvas.height * 0.4);
        ctx.fillText(" press R to replay", canvas.width / 3.5, canvas.height * 0.5);
        return;
      }

      ctx.clearRect(0, 0, canvas.width, canvas.height);
      if (targets.length == 0) {
        winLevel = true;
      }
      drawPaddle();
      drawTargets();
      drawBall();
      displayScore();
      
      if(!gameStarted){
        return;
      }
      updatePaddle();
      updateBall();
      checkCollision();

    }

    gameLoop();

    if (!!window.EventSource) {
        var source = new EventSource('/events');
        source.addEventListener('updateInputs', function(e) {
          console.log(e.data);
          let jsonData = JSON.parse(e.data);
          if (jsonData.right == 0) {
            gameStarted = true;
            paddle.motion = 1;
          } else if (jsonData.left == 0) {
            gameStarted = true;
            paddle.motion = -1;
          }else{
            paddle.motion = 0;
          }

          if(gameOver && jsonData.right == 0 && jsonData.left ==0){
            //Reset game
            resetGame();
          }
          
        }, false);
      }
    </script>
</body>

</html>
)rawliteral";

void setup() {

  
  Serial.begin(9600);
  Serial.println("hi");

  pinMode(rightIRpin, INPUT);
  pinMode(leftIRpin, INPUT);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());//return ip from network

  Serial.println("mDNS responder started");
  // Add service to MDNS-SD
  MDNS.addService("http", "tcp", 80);      //service name , transport protocol ,port

  // Handle Web Server Root
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "text/html", index_html, processor);
  });

  // Handle Web Server Events
  events.onConnect([](AsyncEventSourceClient *client) {      //callback to be executed when a client connects to the event source.
    if (client->lastId()) {
      Serial.printf("Client reconnected! Last message ID that it got is: %u\n", client->lastId());
    }
    // send event with message "hello!", id current millis
    // and set reconnect delay to 1 second
    client->send("hello!", NULL, millis(), 10000);  //This method sends an event to the connected client.

  });
  server.addHandler(&events);

  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  updateReadings();
  delay(10);
}

void updateReadings() {
  //Create JSON DATA as String
  char data[200];
  snprintf(data, 500, "{\"right\":%d, \"left\":%d}", digitalRead(rightIRpin), digitalRead(leftIRpin));  //is used to format the string with the sensor readings. 

  // Send Event to the Web Client to update button status
  events.send(data, "updateInputs", millis());
}




