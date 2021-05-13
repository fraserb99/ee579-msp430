

#include <AzureIoTHub.h>
#include <stdio.h>
#include <stdlib.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>

#include "iot_configs.h"
#include "sample_init.h"
#include "cJSON.h"
#include "Esp.h"

#include "AzureIoTProtocol_MQTT.h"
#include "iothubtransportmqtt.h"

//Update these with your own wifi ssid and password
static const char ssid[] = IOT_CONFIG_WIFI_SSID;
static const char pass[] = IOT_CONFIG_WIFI_PASSWORD ;

/* Define several constants/global variables */
//static const char* connectionString = DEVICE_CONNECTION_STRING;

IOTHUB_MESSAGE_HANDLE message_handle;
const char* telemetry_msg = "test_message";
const char* quit_msg = "quit";
const char* exit_msg = "exit";
const char* serverName = "https://ee579-dev-api.azurewebsites.net/devices/register";
IOTHUB_DEVICE_CLIENT_LL_HANDLE device_ll_handle;

int receiveContext = 0;

/* -- receive_message_callback --
   Callback method which executes upon receipt of a message originating from the IoT Hub in the cloud.
   Note: Modifying the contents of this method allows one to command the device from the cloud.
*/
static IOTHUBMESSAGE_DISPOSITION_RESULT receive_message_callback(IOTHUB_MESSAGE_HANDLE message, void* userContextCallback)
{
  int* counter = (int*)userContextCallback;
  unsigned char* buffer;
  size_t size;
  const char* messageId;

  // Message properties
  if ((messageId = IoTHubMessage_GetMessageId(message)) == NULL)
  {
    messageId = "<null>";
  }

  // Message content
  if (IoTHubMessage_GetByteArray(message, (const unsigned char**)&buffer, &size) != IOTHUB_MESSAGE_OK)
  {
    LogInfo("unable to retrieve the message data\r\n");
  }
  else
  {
    LogInfo("Received Message [%d]\r\n Message ID: %s\r\n Data: <<<%.*s>>> & Size=%d\r\n", *counter, messageId, (int)size, buffer, (int)size);

    // This removes some characters from the end of the message body and sets payload to be the message body string
    char payload[size];
    memcpy( payload, buffer, size);
    payload[size] = '\0';

    // Check that message is in JSON format
    cJSON *parsed = cJSON_Parse(payload);
    if (parsed == NULL)
    {
      LogInfo("Message format is incorrect");
    } else {
      Serial.println(payload);
    }


  }
  (*counter)++;
  return IOTHUBMESSAGE_ACCEPTED;
}


/* -- send_confirm_callback --
   Callback method which executes upon confirmation that a message originating from this device has been received by the IoT Hub in the cloud.
*/
static void send_confirm_callback(IOTHUB_CLIENT_CONFIRMATION_RESULT result, void* userContextCallback)
{
  // When a message is sent this callback will get envoked
  Serial.println("Message sent confirmed");
}

// Callback method which executes on receipt of a connection status message from the IoT Hub in the cloud.
static void connection_status_callback(IOTHUB_CLIENT_CONNECTION_STATUS result, IOTHUB_CLIENT_CONNECTION_STATUS_REASON reason, void* user_context)
{
  (void)reason;
  (void)user_context;
  // This sample DOES NOT take into consideration network outages.
  if (result == IOTHUB_CLIENT_CONNECTION_AUTHENTICATED)
  {
    LogInfo("The device client is connected to iothub\r\n");
  }
  else
  {
    LogInfo("The device client has been disconnected\r\n");
  }
}

/* -- reset_esp_helper --
   waits for call of exit_msg over Serial line to reset device
   this will never happen with the current set-up
*/
static void reset_esp_helper()
{
  // Read from local serial
  if (Serial.available()) {
    String s1 = Serial.readStringUntil('\n');// s1 is String type variable.
    Serial.print("Received Data: ");
    Serial.println(s1);//display same received Data back in serial monitor.

    // Restart device upon receipt of 'exit' call.
    int e_start = s1.indexOf('e');
    String ebit = (String) s1.substring(e_start, e_start + 4);
    if (ebit == exit_msg)
    {
      ESP.restart();
    }
  }
}

static void run_demo()
{


  do
  {
    // This is where we should check for serial from uC
    // We can use the code below to send messages when there's serial input from the uC
    if (Serial.available() > 0) {
      char incomingBuffer[500];
      int rlen = Serial.readBytesUntil('\0', incomingBuffer, 500);
      byte msgText[rlen];
      // prints the received data
      Serial.print("I received: ");
      for (int i = 0; i < rlen; i++) {
        Serial.print(incomingBuffer[i]);
        msgText[i] = incomingBuffer[i];
      }

      cJSON *parsed = cJSON_Parse(incomingBuffer);
      if (parsed == NULL)
      {
        LogInfo("Message format is incorrect");
      }
      else {
        message_handle = IoTHubMessage_CreateFromByteArray(msgText, rlen);
        IOTHUB_CLIENT_RESULT result = IoTHubDeviceClient_LL_SendEventAsync(device_ll_handle, message_handle, send_confirm_callback, NULL);

        //The message is copied to the sdk so the we can destroy it
        IoTHubMessage_Destroy(message_handle);
      }
    }

    IoTHubDeviceClient_LL_DoWork(device_ll_handle);
    ThreadAPI_Sleep(3);

  } while (true);

  // Clean up the iothub sdk handle
  IoTHubDeviceClient_LL_Destroy(device_ll_handle);
  // Free all the sdk subsystem
  IoTHub_Deinit();

  return;
}
//code that runs once when the esp receives power
void setup() {
  //baudrate of 115200

  // Select the Protocol to use with the connection
  IOTHUB_CLIENT_TRANSPORT_PROVIDER protocol = MQTT_Protocol;

  //connect to wifi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected");
  //run with blank strings to initialise
  sample_init("", "");
  //flush the serial and reconfigure baud rate to 9600
  Serial.flush();
  Serial.begin(9600);
  //send a http post request with the device details to register the device with the system
  const char* connectionString;
  String post_data = "{\"deviceId\":\"" + WiFi.macAddress() + "\"}";
  HTTPClient http;
  http.begin(serverName, "8b00830bccf246f796d38eb5e12fcfbd1d3a6150");
  http.addHeader("Content-Type", "application/json");
  int httpResponseCode = -1;
  //if the http response isn't good keep retyring until it is
  while (httpResponseCode != 200) {
    httpResponseCode =  http.POST(post_data);
    if (httpResponseCode > 0) {
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
      String payload = http.getString();
      //returns JSON
      // parse the string as JSON and extract the connection string, which is the only thing we're interested in
      DynamicJsonDocument doc(1024);
      deserializeJson(doc, payload);
      JsonObject obj = doc.as<JsonObject>();
      connectionString = obj[String("connectionString")];
      Serial.println(connectionString);
    }
    else {
      Serial.print("Error code: ");
      Serial.println(httpResponseCode);
    }
    http.end();
    //free up resources
  }

  // Used to initialize IoTHub SDK subsystem
  (void)IoTHub_Init();
  // Create the iothub handle here using the connectionString extracted from the http post request
  device_ll_handle = IoTHubDeviceClient_LL_CreateFromConnectionString(connectionString, protocol);
  LogInfo("Creating IoTHub Device handle\r\n");

  if (device_ll_handle == NULL)
  {
    LogInfo("Error AZ002: Failure creating Iothub device. Hint: Check you connection string.\r\n");
  }
  else
  {
    // Set any option that are neccessary.
    // For available options please see the iothub_sdk_options.md documentation in the main C SDK
    // turn off diagnostic sampling
    int diag_off = 0;
    IoTHubDeviceClient_LL_SetOption(device_ll_handle, OPTION_DIAGNOSTIC_SAMPLING_PERCENTAGE, &diag_off);

    // Setting the Trusted Certificate.
    IoTHubDeviceClient_LL_SetOption(device_ll_handle, OPTION_TRUSTED_CERT, certificates);

    //Setting the auto URL Encoder (recommended for MQTT). Please use this option unless
    //you are URL Encoding inputs yourself.
    //ONLY valid for use with MQTT
    bool urlEncodeOn = true;
    IoTHubDeviceClient_LL_SetOption(device_ll_handle, OPTION_AUTO_URL_ENCODE_DECODE, &urlEncodeOn);
    /* Setting Message call back, so we can receive Commands. */
    if (IoTHubClient_LL_SetMessageCallback(device_ll_handle, receive_message_callback, &receiveContext) != IOTHUB_CLIENT_OK)
    {
      LogInfo("ERROR: IoTHubClient_LL_SetMessageCallback..........FAILED!\r\n");
    }

    // Setting connection status callback to get indication of connection to iothub
    (void)IoTHubDeviceClient_LL_SetConnectionStatusCallback(device_ll_handle, connection_status_callback, NULL);
    Serial.println("READY TO PARSE");
  }
}
//loop whilst there is stuff to do
void loop(void)
{
  run_demo();
  reset_esp_helper();
}
