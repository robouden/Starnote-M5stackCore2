// Copyright 2019 Blues Inc.  All rights reserved.
//
// Use of this source code is governed by licenses granted by the
// copyright holder including that found in the LICENSE file.
//
// This example shows the simplest possible method demonstrating how a device
// might poll a notefile used as an "inbound queue", using it to receive
// messages sent to the device from the service.  The message gets into the
// service by use of the Notehub's HTTP/HTTPS inbound request capability.
//
// In order to use this example,
// 1. Get the device up and running the code below, successfully connecting to
//    the service
// 2. Use the "Devices" view on notehub.io to determine the DeviceUID of the
//    device, which is a unique string that looks like "dev:000000000000000"
// 3. Use the "Settings / Project" view on notehub.io to determine the App UID
//    of your project, a unique string that looks like
//    "app:00000000-0000-0000-0000-000000000000"
// 4. At the command line of your PC, send an HTTP message to the service
//    such as:
//    curl -L
//    'http://api.notefile.net/req?project="app:00000000-0000-0000-0000-000000000000"&device="dev:000000000000000"'
//    -d
//    '{"req":"note.add","file":"my-inbound.qi","body":{"my-request-type":"my-request"}}'

#include <M5Core2.h>
#include <Notecard.h>

// Parameters for this example
#define INBOUND_QUEUE_POLL_SECS 10
#define INBOUND_QUEUE_NOTEFILE "my-inbound.qi"
#define INBOUND_QUEUE_COMMAND_FIELD "my-request-type"

//versioning setup
#define  MAJOR_VERSION 2
#define  MINOR_VERSION .2
#define  PATCH_VERSION .3

#define VERSION_NUMBER STR(MAJOR_VERSION) "." STR(MINOR_VERSION) "." STR(PATCH_VERSION)
#define VERSION_STRING "Starnote M5tack" VERSION_NUMBER

// Note that both of these definitions are optional; just prefix either line
// with `//` to remove it.
// - Remove txRxPinsSerial if you wired your Notecard using I2C SDA/SCL pins
//   instead of serial RX/TX
// - Remove usbSerial if you don't want the Notecard library to output debug
//   information

// #define txRxPinsSerial Serial
#define usbSerial Serial

// This is the unique Product Identifier for your device
#define PRODUCT_UID                                                            \
  "biz.yr-design.rob:kittywood" // "com.my-company.my-name:my-project"

#define myProductID PRODUCT_UID

Notecard notecard;
#define myLiveDemo true

// One-time Arduino initialization
void setup() {

  M5.begin(true, true, true,
           true); // Init M5Core2(Initialization of external I2C is also
                  // included).  初始化M5Core2(初始化外部I2C也包含在内)
  // Wire.begin(21, 22); //Detect internal I2C, if this sentence is not added,
  // it will detect external I2C.  检测内部I2C,若不加此句为检测外部I2C

  Serial.println("Booting");
  M5.Lcd.setTextColor(
      WHITE); // Set the font color to yellow.  设置字体颜色为黄色
  M5.Lcd.setTextSize(2); // Set the font size to 2.  设置字体大小为2
  M5.Lcd.print("M5Core2 StarNote Tester"); 
  M5.Lcd.println(MAJOR_VERSION+MINOR_VERSION+PATCH_VERSION); // Print a string on the screen.

  delay(1000);
  // M5.Lcd.fillScreen(BLACK); // Make the screen full of black (equivalent to

#ifdef usbSerial
  // If you open Arduino's serial terminal window, you'll be able to watch
  // JSON objects being transferred to and from the Notecard for each request.
  usbSerial.begin(115200);
  const size_t usb_timeout_ms = 3000;
  for (const size_t start_ms = millis();
       !usbSerial && (millis() - start_ms) < usb_timeout_ms;)
    ;

    // For low-memory platforms, don't turn on internal Notecard logs.
#ifndef NOTE_C_LOW_MEM
  notecard.setDebugOutputStream(usbSerial);
#else
#pragma message("INFO: Notecard debug logs disabled. (non-fatal)")
#endif // !NOTE_C_LOW_MEM
#endif // usbSerial

  // Initialize the physical I/O channel to the Notecard
#ifdef txRxPinsSerial
  notecard.begin(txRxPinsSerial, 9600);
#else
  notecard.begin();
#endif

  // Configure the productUID, and instruct the Notecard to stay connected to
  // the service if `myLiveDemo` is `true`.
  J *req = notecard.newRequest("hub.set");
  if (myProductID[0]) {
    JAddStringToObject(req, "product", myProductID);
  }
#if myLiveDemo
  JAddStringToObject(req, "mode", "continuous");
  JAddBoolToObject(req, "sync", true); // Automatically sync when changes are
                                       // made on notehub
#else
  JAddStringToObject(req, "mode", "periodic");
  JAddNumberToObject(req, "inbound", 60);
#endif
  notecard.sendRequestWithRetry(req, 5);
}

// In the Arduino main loop which is called repeatedly, add outbound data every
// 15 seconds
void loop() {
  // On a periodic basis, check the inbound queue for messages.  In a
  // real-world application, this would be checked using a frequency
  // commensurate with the required inbound responsiveness. For the most
  // common "periodic" mode applications, this might be daily or weekly.  In
  // this example, where we are using "continuous" mode, we check quite often
  // for demonstratio purposes.
  static unsigned long nextPollMs = 0;
  if (millis() > nextPollMs) {
    M5.Lcd.println("pulling in data");
    nextPollMs = millis() + (INBOUND_QUEUE_POLL_SECS * 1000);

    // Process all pending inbound requests
    while (true) {
      // Get the next available note from our inbound queue notefile,
      // deleting it
      J *req = notecard.newRequest("note.get");
      JAddStringToObject(req, "file", INBOUND_QUEUE_NOTEFILE);
      JAddBoolToObject(req, "delete", true);
      J *rsp = notecard.requestAndResponse(req);
      if (rsp != NULL) {
        // If an error is returned, this means that no response is
        // pending.  Note that it's expected that this might return
        // either a "note does not exist" error if there are no pending
        // inbound notes, or a "file does not exist" error if the
        // inbound queue hasn't yet been created on the service.
        if (notecard.responseError(rsp)) {
          notecard.deleteResponse(rsp);
          break;
        }

        // Get the note's body
        J *body = JGetObject(rsp, "body");
        if (body != NULL) {

          // Simulate Processing the response here
          // usbSerial.print("[APP] INBOUND REQUEST: ");
          // usbSerial.println(JGetString(body, INBOUND_QUEUE_COMMAND_FIELD));
          // usbSerial.println();
        }
      }
      notecard.deleteResponse(rsp);
    }
  }
}