#include "WifiCam.hpp"
#include <StreamString.h>
#include <uri/UriBraces.h>

#include <HTTPClient.h>

static const char* flask_post = "http://172.20.10.2:5000/sendpicture";
static const char* spring_post = "http://172.20.10.6:8080/image/receive";

//static const char* flask_test = "http://172.20.10.4:8080/image/receive";

static const char FRONTPAGE[] = R"EOT(
<!doctype html>
<title>esp32cam WifiCam example</title>
<style>
table,th,td { border: solid 1px #000000; border-collapse: collapse; }
th,td { padding: 0.4rem; }
a { text-decoration: none; }
footer { margin-top: 1rem; }
</style>
<body>
<h1>esp32cam WifiCam example</h1>
<table>
<thead>
<tr><th>BMP<th>JPG<th>MJPEG
<tbody id="resolutions">
<tr><td colspan="3">loading
</table>
<footer>Powered by <a href="https://esp32cam.yoursunny.dev/">esp32cam</a></footer>
<script type="module">
async function fetchText(uri, init) {
  const response = await fetch(uri, init);
  if (!response.ok) {
    throw new Error(await response.text());
  }
  return (await response.text()).trim().replaceAll("\r\n", "\n");
}

try {
  const list = (await fetchText("/resolutions.csv")).split("\n");
  document.querySelector("#resolutions").innerHTML = list.map((r) => `<tr>${
    ["bmp", "jpg", "mjpeg"].map((fmt) => `<td><a href="/${r}.${fmt}">${r}</a>`).join("")
  }`).join("");
} catch (err) {
  document.querySelector("#resolutions td").textContent = err.toString();
}
</script>
)EOT";

static void serveStill(bool wantBmp) {
  auto frame = esp32cam::capture();
  if (frame == nullptr) {
    Serial.println("capture() failure");
    server.send(500, "text/plain", "still capture error\n");
    return;
  }
  //Serial.printf("capture() success: %dx%d %zub\n", frame->getWidth(), frame->getHeight(),
  //frame->size());
  Serial.println("Capture() success");

  if (wantBmp) {
    if (!frame->toBmp()) {
      Serial.println("toBmp() failure");
      server.send(500, "text/plain", "convert to BMP error\n");
      return;
    }
    //Serial.printf("toBmp() success: %dx%d %zub\n", frame->getWidth(), frame->getHeight(),
    //frame->size());
  }

  server.setContentLength(frame->size());
  server.send(200, wantBmp ? "image/bmp" : "image/jpeg");
  WiFiClient client = server.client();
  frame->writeTo(client);
}

static void serveMjpeg() {
  Serial.println("MJPEG streaming begin");
  WiFiClient client = server.client();
  auto startTime = millis();
  int nFrames = esp32cam::Camera.streamMjpeg(client);
  auto duration = millis() - startTime;
  //Serial.printf("MJPEG streaming end: %dfrm %0.2ffps\n", nFrames, 1000.0 * nFrames / duration);
}

String currentState = "";
#define LED_BUILTIN 4
static int isWorking = 0;
void takePictureCode() {
  if (isWorking) {
    return;
  }
  isWorking = 1;
  Serial.println("TAKINGPICTURE");
  digitalWrite(LED_BUILTIN, HIGH);
  delay(700);
  long width = 1280;
  long height = 720;  
  String format = "jpg";

  auto resolution = esp32cam::Camera.listResolutions().find(width, height);
  if (!resolution.isValid()) {
    Serial.println("INTERNALERROR");
    digitalWrite(LED_BUILTIN, LOW);
    isWorking = 0;
    server.send(500, "text/plain", "Internal Error - Resolution\n");
    return;
  }

  if (!esp32cam::Camera.changeResolution(resolution)) {
    //Serial.printf("changeResolution(%ld,%ld) failure\n", width, height);
    Serial.println("INTERNALERROR");
    digitalWrite(LED_BUILTIN, LOW);
    isWorking = 0;
    server.send(500, "text/plain", "Internal Error - changeResolution error\n");
    return;
  }

  // Capture the image
  auto frame = esp32cam::capture();
  if (frame == nullptr) {
    Serial.println("capture() failure");
    Serial.println("INTERNALERROR");
    digitalWrite(LED_BUILTIN, LOW);
    isWorking = 0;
    server.send(500, "text/plain", "Internal Error - Capture error\n");
    return;
  }
  delay(200);
  digitalWrite(LED_BUILTIN, LOW);
  Serial.println("PROCESSIMAGE");
  // Flask AI =================================================
  //===========================================================
  
  String resultString = "999-999";
  int boxDetected = 0;
  int confidence = 0;

  HTTPClient http;
  http.setTimeout(60 * 1000); 

  http.begin(String(flask_post));
  http.addHeader("Content-Type", "image/jpeg");

  // Send the image in the body of the POST request
  int httpResponseCode = http.POST(frame->data(), frame->size());
  
  if (httpResponseCode == 200) {
    Serial.println("Post to Flask Success!!!");

    resultString = http.getString();

    int dashIndex = resultString.indexOf('-');
    String firstNumber = resultString.substring(0, dashIndex);
    String secondNumber = resultString.substring(dashIndex + 1);
    boxDetected = firstNumber.toInt();
    confidence = secondNumber.toInt();

    // Serial.println();
    // Serial.print("boxDetected: ");
    // Serial.print(boxDetected);
    // Serial.println();
    // Serial.print("confidence: ");
    // Serial.print(confidence);
    // Serial.println();
    // Serial.println();
  } else {
    Serial.printf("Post to Flask Failed. Error code: %d\n", httpResponseCode);
    //server.send(400, "text/plain", "Send image to AI failed, discard process\n");
    //Serial.println("FLASKERROR");
    //isWorking = 0;
    //return;
  }

  //Springboot =================================================
  //===========================================================
  if (boxDetected >= 0) {

    http.begin(String(spring_post));
    http.setTimeout(15 * 1000); 
    http.addHeader("Content-Type", "image/jpeg");

    // Attach extra data to the POST request
    http.addHeader("X-Extra-Data", String(boxDetected));

    int springResponseCode = http.POST(frame->data(), frame->size());

    if (springResponseCode == 200) {
      Serial.printf("Post to Spring Boot Success!!!");
      Serial.println(http.getString());
    } else {
      Serial.printf("Post to Spring Boot Failed. Error code: %d\n", springResponseCode);
      //Serial.println("SPRINGBOOTERROR");
      //isWorking = 0;
    }
  }

  server.send(200);
  Serial.println("Result:" + String(boxDetected) + "-" + String(confidence));
  //Serial.println(boxDetected);
  delay(100);
  Serial.println("READYTOUSE");
  isWorking = 0;
}

void addRequestHandlers() {
  server.on("/takepicture", HTTP_GET, [] {
    takePictureCode();
    return;
    // Serial.println("TAKINGPICTURE");
    // digitalWrite(LED_BUILTIN, HIGH);
    // delay(1000);
    // long width = 1280;
    // long height = 720;
    // String format = "jpg";

    // auto resolution = esp32cam::Camera.listResolutions().find(width, height);
    // if (!resolution.isValid()) {
    //   server.send(500, "text/plain", "Internal Error - Resolution\n");
    //   Serial.println("INTERNALERROR");
    //   return;
    // }

    // if (!esp32cam::Camera.changeResolution(resolution)) {
    //   //Serial.printf("changeResolution(%ld,%ld) failure\n", width, height);
    //   server.send(500, "text/plain", "Internal Error - changeResolution error\n");
    //   Serial.println("INTERNALERROR");
    //   return;
    // }

    // // Capture the image
    // auto frame = esp32cam::capture();
    // if (frame == nullptr) {
    //   Serial.println("capture() failure");
    //   server.send(500, "text/plain", "Internal Error - Capture error\n");
    //   Serial.println("INTERNALERROR");
    //   return;
    // }

    // Serial.println("PROCESSIMAGE");
    // delay(1000);
    // digitalWrite(LED_BUILTIN, LOW);
    // // Flask AI =================================================
    // //===========================================================

    // HTTPClient http;
    // http.setTimeout(5 * 1000); // 5 second
    // http.begin(String(flask_post));
    // http.addHeader("Content-Type", "image/jpeg");

    // // Send the image in the body of the POST request
    // int httpResponseCode = http.POST(frame->data(), frame->size());
    // int boxDetected = -1;

    // if (httpResponseCode == 200) {
    //   Serial.println("Post to Flask Success!!!");

    //   String resultString = http.getString();
    //   boxDetected = resultString.toInt();

    //   //Serial.printf("Result value: %d\n", boxDetected);
    // } else {
    //   //Serial.printf("Post to Flask Failed. Error code: %d\n", httpResponseCode);
    //   server.send(400, "text/plain", "Send image to AI failed, discard process\n");
    //   Serial.println("FLASKERROR");
    //   return;
    // }

    // //Springboot =================================================
    // //===========================================================
    // if(boxDetected >= 0){
    //   http.setTimeout(3 * 1000);
    //   http.begin(String(spring_post));
    //   http.addHeader("Content-Type", "image/jpeg");

    //   // Attach extra data to the POST request
    //   http.addHeader("X-Extra-Data", "37");

    //   int springResponseCode = http.POST(frame->data(), frame->size());

    //   if (springResponseCode == 200) {
    //       //Serial.printf("Post to Spring Boot Success!!!. Response code: %d\n", springResponseCode);
    //       Serial.println(http.getString());
    //   } else {
    //       //Serial.printf("Post to Spring Boot Failed. Error code: %d\n", springResponseCode);
    //       //Serial.println("SPRINGBOOTERROR");
    //   }
    // }

    // server.send(200, "text/plain", "Take and Send Picture Success!\n");
    // Serial.println("692");
    // delay(100);
    // Serial.println("READYTOUSE");
  });

  // server.on("/callesp32", HTTP_GET, [] {
  //   Serial.println("Received GET request on /activeEsp32-2");

  //   HTTPClient http;
  //   http.begin(String(flask_test));
  //   int httpResponseCode = http.GET();

  //   if (httpResponseCode > 0) {
  //     //Serial.printf("GET request to /esp32-cam successful. Response code: %d\n", httpResponseCode);
  //     // Print the response from the server
  //     Serial.println(http.getString());
  //   } else {
  //     //Serial.printf("GET request to /esp32-cam failed. Error code: %d\n", httpResponseCode);
  //   }
  //   server.send(200, "text/plain", "Đã chạy xong, check serial đi");
  // });

  server.on("/READYTOUSE", HTTP_GET, [] {
    const char content[] = R"EOT(
      <!doctype html>
      <h1>READYTOUSE</h1>
      )EOT";

    server.setContentLength(sizeof(content));
    server.send(200, "text/html");
    server.sendContent(content, sizeof(content));
    Serial.println("READYTOUSE");
  });
  server.on("/PROCESSIMAGE", HTTP_GET, [] {
    const char content[] = R"EOT(
      <!doctype html>
      <h1>PROCESSIMAGE</h1>
      )EOT";

    server.setContentLength(sizeof(content));
    server.send(200, "text/html");
    server.sendContent(content, sizeof(content));
    Serial.println("PROCESSIMAGE");
  });

  server.on("/DONE", HTTP_GET, [] {
    const char content[] = R"EOT(
      <!doctype html>
      <h1>DONE, 192</h1>
      )EOT";

    server.setContentLength(sizeof(content));
    server.send(200, "text/html");
    server.sendContent(content, sizeof(content));
    Serial.println("192");
    delay(100);
    Serial.println("READYTOUSE");
  });

  server.on("/", HTTP_GET, [] {
    server.setContentLength(sizeof(FRONTPAGE));
    server.send(200, "text/html");
    server.sendContent(FRONTPAGE, sizeof(FRONTPAGE));
  });

  server.on("/robots.txt", HTTP_GET,
            [] {
              server.send(200, "text/html", "User-Agent: *\nDisallow: /\n");
            });

  server.on("/resolutions.csv", HTTP_GET, [] {
    StreamString b;
    for (const auto& r : esp32cam::Camera.listResolutions()) {
      b.println(r);
    }
    server.send(200, "text/csv", b);
  });

  server.on(UriBraces("/{}x{}.{}"), HTTP_GET, [] {
    long width = server.pathArg(0).toInt();
    long height = server.pathArg(1).toInt();
    String format = server.pathArg(2);
    if (width == 0 || height == 0 || !(format == "bmp" || format == "jpg" || format == "mjpeg")) {
      server.send(404);
      return;
    }

    auto r = esp32cam::Camera.listResolutions().find(width, height);
    if (!r.isValid()) {
      server.send(404, "text/plain", "non-existent resolution\n");
      return;
    }
    if (r.getWidth() != width || r.getHeight() != height) {
      server.sendHeader("Location",
                        String("/") + r.getWidth() + "x" + r.getHeight() + "." + format);
      server.send(302);
      return;
    }

    if (!esp32cam::Camera.changeResolution(r)) {
      //Serial.printf("changeResolution(%ld,%ld) failure\n", width, height);
      server.send(500, "text/plain", "changeResolution error\n");
    }
    //Serial.printf("changeResolution(%ld,%ld) success\n", width, height);

    if (format == "bmp") {
      serveStill(true);
    } else if (format == "jpg") {
      serveStill(false);
    } else if (format == "mjpeg") {
      serveMjpeg();
    }
  });
}
