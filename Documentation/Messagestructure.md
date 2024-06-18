# 1. Message structure
```
0x01 0x23 0x45 0x67 0x89 0xAB 0x33 0xF3 -> Device ID (8 Byte)
          	                       0x01 -> Device Type (1 Byte)
                                   0x00 -> Message Counter (1 Byte)
          	                       0xD8 -> Content (1 - 243 Bytes)
       		                  0x12 0x34 -> Checksum (2 Bytes)
```
- **Device ID**: Unique ID of the device. For ESP32 Devices this will be the WiFi Mac, because it is unique over all WiFi devices. Other device might need to be given a number to make sure it's unique over all posible devices.
- **Device Type**: The type of the device. This is used on the server side to process the message correctly.
- **Message Counter**: Will increase for each message (except NACK/ACK). Is used either to make sure no double messages are received, or to assign a NACK/ACK to it's corresponding message.
- **Content**: The data that will be transmitted. It can be sensor data of an device, or the config for a device from the server.
- **Checksum**: Addition of all previous bytes of the message. Supports up to 257 characters, when every character is 255. Messages can be a maximum of 255 bytes long, and 2 of them are the checksum. This results in a maximum of 253 bytes that will be added.

Maximum message size is 255 Byte.<br>
Every byte should be processed as unsigned char by default, signed chars will be only used in spezial cases depending on the Device Type Message processing.<br>
Hexadezimal characters are only shown at the top, to be able to use 2 digits per byte.<br>

# 2. Security
## 2.1 Encoding
Every byte will be encrypted with the **Encrypt Key**.
- ((byte * **Encrypt Key**) % 256) = encrypted byte

### 2.1.1 **Encrypt Key**
A key that is the same for all devices and also the server. It is used to encrypt the messages.

## 2.2 Decoding
Every byte needs to be decrypted with the **Decrypt Key**, so it can be processed.
- ((encrypted byte * **Decrypt Key**) % 256) = byte

### 2.2.1 **Decrypt Key**
A key that is the same for all devices and also the server. It is used to decrypt the messages.

# 3. Message Header
The message header contains **Device ID**, **Device Type** and **Message Counter**.

## 3.1 Server
Can only answer to messages of the device. So it will always contain the data from the message it answers to.
- **Device ID**: the one of the corresponding device
- **Device Type**: the one of the corresponding device
- **Message Counter**: the one of the corresponding message

## 3.2 Device
Can send without being an answer. It can also be an answer. In case of an answer the **Message Counter** will not be increased, to be able to know it was an answer to an previous message.
- **Device ID**: the one of the device
- **Device Type**: the one of the device
- **Message Counter**: the incremental one of the device (should be at least 1 bigger than the last one, except for an answer then it stays the same)

# 4. Message Types
## 4.1 Config Message
### 4.1.1 Server
A message containing a config for the corresponding device. It can only be send as an answer to a message from the corresponding device.
- **Content**: an acceptable Config for the given **Device Type**

### 4.1.2 Device
Can only receive this type of message. Will process it and answer with an **ACK** if it was processed correctly, or an **NACK** if the processing failed.

## 4.2 Data Message
### 4.2.1 Server
Can only receive this type of message. Will check the **Message Counter** and send an **ACK** if it is higher than the last one, or 0.<br>
In the other cases it will send an **NACK** and the device should try later to send again, after fixing the issue.

### 4.2.2 Device**
A message containing (in most times measured sensor) data of the device that should be saved on the server to be accessable for the user.<br>
It will be send by the device and can't be an answer to any message.
- **Content**: the data in the correct format/structure depending on the **Device Type**

# 5. Acknowledge
It can only be an answer to a non **NACK**/**ACK** message.
- So it will be either an answer to a **Data Message** from the device or to a **Config Message** from the server.
- Make sure no devices support such short messages to prevent normal messages mistaken for **NACK**/**ACK**.
  - The server will prevent adding 'N' (0x4E) or 'A' (0x41) as ID for a config or message parameter!

## 5.1 **NACK** Message
- **Content**: contains 2 Bytes, the first is 'N' (0x4E) and the second is additional information

### 5.1.1 Server
If the **Message Counter** is wrong the server will directly send a **NACK**, where the additional information is the last received **Message Counter** of the device that is saved on the server.<br>
So the next message of the device should either have a counter of 0 (will reset the counter) or one bigger than the given.<br>
<br>
If the server received a **NACK** it will remove the **Config Message** it tried to send from it's config queue and notify the user which config parameter in this message was wrong.

### 5.1.2 Device
If there was a problem with the received **Config Message** it will send a **NACK**. The additional Data in the **NACK** will be the ID of the config parameter that resulted in the problem.
- It will only tell the first Parameter that resulted in a problem. That means there could be more than one problematic parameter.

If the device received a **NACK** it will use the given **Message Counter** as its own and increase it by one before sending the same message with different **Message Counter** again.<br>
When a problem occoured while retrieving the new **Message Counter** or if it was not possible to increase it by one, the device will fall back to 0.

## 5.2 ACK Message
- **Content**: contains 1 Byte: 'A' (0x41)

### 5.2.1 Server
If the **Message Counter** is correct the server will directly send an **ACK**.

If the server received an **ACK** it will remove the **Config Message** it tried to send from it's config queue and also flag it as send in the database, so it wont be send again.

### 5.2.2 Device
If the **Config Message** was processed correctly it will send an **ACK**.

If the device received an **ACK** it will do nothing.
