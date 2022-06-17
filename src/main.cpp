#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ESP_Mail_Client.h>
#include "time.h"
#include "wifi.h"
#include "secrets.h"

const char* ntpServer = "de.pool.ntp.org";
const long  gmtOffset_sec = 3600;
const int   daylightOffset_sec = 3600;

HTTPClient sender;
WiFiClient wifiClient;

String currentHour;
String currentMinute;

/* The SMTP Session object used for Email sending */
SMTPSession smtp;

/* Callback function to get the Email sending status */
void smtpCallback(SMTP_Status status);

tm* getTimeInfo()
{
    time_t now;
    struct tm* timeinfo;
    time(&now);
    timeinfo = localtime(&now);

    return timeinfo;
}

// https://randomnerdtutorials.com/esp8266-nodemcu-send-email-smtp-server-arduino/
void sendMail(String text) {
    /** Enable the debug via Serial port
   * none debug or 0
   * basic debug or 1
  */
  smtp.debug(1);

  /* Set the callback function to get the sending results */
  smtp.callback(smtpCallback);

  /* Declare the session config data */
  ESP_Mail_Session session;

  /* Set the session config */
  session.server.host_name = GetSmtpHost();
  session.server.port = GetSmtpPort();
  session.login.email = GetSmtpUser();
  session.login.password = GetSmtpPassword();
  session.login.user_domain = GetSmtpRecipient();

  /* Declare the message class */
  SMTP_Message message;

  /* Set the message headers */
  message.sender.name = "ESP";
  message.sender.email = AUTHOR_EMAIL;
  message.subject = "ESP Test Email";
  message.addRecipient("Janno", RECIPIENT_EMAIL);

  //Send raw text message
  String textMsg = text;
  message.text.content = textMsg.c_str();
  message.text.charSet = "us-ascii";
  message.text.transfer_encoding = Content_Transfer_Encoding::enc_7bit;
  
  message.priority = esp_mail_smtp_priority::esp_mail_smtp_priority_low;
  message.response.notify = esp_mail_smtp_notify_success | esp_mail_smtp_notify_failure | esp_mail_smtp_notify_delay;

  /* Connect to server with the session config */
  if (!smtp.connect(&session))
    return;

  /* Start sending Email and close the session */
  if (!MailClient.sendMail(&smtp, &message))
    Serial.println("Error sending Email, " + smtp.errorReason());
}

/* Callback function to get the Email sending status */
void smtpCallback(SMTP_Status status){
  /* Print the current status */
  Serial.println(status.info());

  /* Print the sending result */
  if (status.success()){
    Serial.println("----------------");
    ESP_MAIL_PRINTF("Message sent success: %d\n", status.completedCount());
    ESP_MAIL_PRINTF("Message sent failled: %d\n", status.failedCount());
    Serial.println("----------------\n");
    struct tm dt;

    for (size_t i = 0; i < smtp.sendingResult.size(); i++){
      /* Get the result item */
      SMTP_Result result = smtp.sendingResult.getItem(i);
      time_t ts = (time_t)result.timestamp;
      localtime_r(&ts, &dt);

      ESP_MAIL_PRINTF("Message No: %d\n", i + 1);
      ESP_MAIL_PRINTF("Status: %s\n", result.completed ? "success" : "failed");
      ESP_MAIL_PRINTF("Date/Time: %d/%d/%d %d:%d:%d\n", dt.tm_year + 1900, dt.tm_mon + 1, dt.tm_mday, dt.tm_hour, dt.tm_min, dt.tm_sec);
      ESP_MAIL_PRINTF("Recipient: %s\n", result.recipients);
      ESP_MAIL_PRINTF("Subject: %s\n", result.subject);
    }
    Serial.println("----------------\n");
  }
}

//https://makesmart.net/esp8266-http-get-request/
String getCurrentIP()
{
    String payload = "";
    if (sender.begin(wifiClient, "http://api.ipify.org/")) {
        int httpCode = sender.GET();
        if (httpCode > 0) {
            if (httpCode == HTTP_CODE_OK) {
                payload = sender.getString();
                Serial.println(payload);
            }
        
        } else {
            Serial.printf("HTTP-Error: %s", sender.errorToString(httpCode).c_str());
        }

        sender.end();
        
    } else {
        Serial.printf("HTTP-Verbindung konnte nicht hergestellt werden!");
    }

  return payload;
}

String getHour(tm* timeinfo)
{
    String hourAsString = String(timeinfo->tm_hour);

    return hourAsString;
}

void setup()
{
	Serial.begin(9600);
    delay(10);
    wifiClient = ConnectWifi();

    configTime("CET-1CEST,M3.5.0,M10.5.0/3", ntpServer);    
}

void loop()
{        
    struct tm* timeInfo = getTimeInfo();
    String hour = getHour(timeInfo);

    if(hour != currentHour) {
        if(hour == "17") {
            Serial.println(getCurrentIP());
            sendMail(getCurrentIP());
        }

        currentHour = hour;
    }        
    
    delay(2000);
}
