# BLE Sensor App

## WBZ451 Curiosity Board {#GUID-EAF6A09D-5BD9-487E-949D-582F453EDEB0 .section}

**Devices \(Device\):** PIC32CX1012BZ20548\(MCU\) on WBZ451 module

**Devices \(Used, On-Board\):** Analog Temperature Sensor, RGB LED, User Button, User LED, USB-UART converter

![](media/GUID-5AFED1F6-54D9-472A-9085-41449185CC5C-low.png "WBZ451 Curiosity Board")

Hardware Block Diagram - Curiosity Board

![](media/GUID-6E397659-B0E6-44AD-A3CD-E333A27ED960-low.png)

## Hardware Required {#GUID-93572D7A-7E33-4595-B2F4-BCBA6FE5CF08 .section}

|Tool|Qty|
|WBZ451 Curiosity Board|1|
|Micro usb cable|1|
|Andrioid/iOS Smartphone|1|

## Software {#GUID-B36205B2-65BE-4695-9AFB-0CCA7A6BC40A .section}

[Teraterm](https://ttssh2.osdn.jp/index.html.en)

## Smartphone App {#GUID-AECB4868-ED73-47F6-83C7-C7902FABB144 .section}

Microchip Bluetooth Data \(MBD\) iOS/Android app available in stores

## Demo Description {#GUID-0DFD12EB-FC6B-41F6-8F38-D584C5ABCA93 .section}

This application demonstrates the capability of WBZ451 module to connect to a mobile phone through Bluetooth Low Energy\(BLE\). The RGB LED on the Curiosity board can be controlled by mobile app. The WBZ451 device will also report the temperature data periodically to mobile phone through Bluetooth low energy \(BLE\).

1.  The WBZ451 module will be a BLE peripheral device and will advertise on startup. The user can initiate the connection through mobile application. The [advertisement payload](https://onlinedocs.microchip.com/pr/GUID-A5330D3A-9F51-4A26-B71D-8503A493DF9C-en-US-1/index.html?GUID-D320EA38-86CE-4C0B-8093-60C59E1F03AE) holds the temperature information and the RGB ON/OFF status.

2.  Uses "BLE Sensor" sub app from the Microchip Bluetooth Data \(MBD\) mobile app for BLE demonstration.

    -   Application is supported by iOS and Andrioid OS

    -   Available in respective app stores/play stores

3.  When Connected\(ING\) to the application the BLUE color "User LED" will turn on

    -   Blinks the "User LED" with 500 ms blink interval when the device sending advertisement packets

    -   Once connected, the "User LED" turns on solid

4.  From the Smartphone App the following actions can be performed

    -   The RGB LED can be switched On/Off from MBD mobile app.

    -   When LED is switched On, the RGB color can be changed from mobile app color wheel.

        -   The RGB color value is received as HSV \(Hue, Saturation, Value\) from mobile app through TRPS[transparent profile and service](https://onlinedocs.microchip.com/pr/GUID-A5330D3A-9F51-4A26-B71D-8503A493DF9C-en-US-1/index.html?GUID-D320EA38-86CE-4C0B-8093-60C59E1F03AE)
        -   The HSV value is converted to RGB equivalent value in the device. The corresponding PWM duty cycle for R,G,B will be calculated and the PWM pulse is provided on R,G,B LEDs.

5.  From the WBZ451 module the following actions can be performed

    -   The RGB LED can be switched on/off by pressing the On board "User Button"

        -   When the "User Button" is pressed and released the RGB LED is switched ON with default color GREEN or the last stored color

        -   When the "User Button" is pressed and released again, then the RGB LED will be toggled from the previous state

    -   Read the temperature sensor every 1 sec once and send the temperature value to mobile app when the temperature changes about 1 degree C


## Expected Out of Box Behavior {#GUID-55C9FA3E-CC06-46C9-8A62-327FFF7B44AB .section}

Follow the steps [Programming the precompiled hex file using MPLAB X IPE](#GUID-0373311D-973A-46EB-8BE0-C27F3A6F3057) to program the hex image

If want to do changes in the demo code and would like to program/debug the customized code follow the instruction in [Build and Program the Application](#GUID-409E287E-EF30-4179-9DA1-7B5375215230)

1.  Power Up: Connect the curiosity board using Micro USB to PC

    ![](media/GUID-3E27AD02-FE3B-49B1-A23C-8799600335EA-low.jpg)

2.  Power On Behavior: Power Supply Green LED will turn on when connected to PC

    ![](media/GUID-82216D6D-AD47-4BAF-A941-0437090A2761-low.jpg)

3.  UART Console Behavior

    1.  The application activity is shown as "Console Log" through on board UART-USB converter
    2.  Open Terminal \(eg: Tera Term\) to look for these logs - UART baud settings: 115200 8-N-1 without flow control
    ![](media/GUID-442FA5C7-3806-4A32-A1D9-4E120BAE9C03-low.jpg)

4.  BLE Not Connected Behavior

    1.  The Blue "User LED" will be blinking
    2.  Scanned from mobile app as "BLE\_Sensor\_XXXX", where 'XXXX' is a uniqu number associated with this board. Launch MBD mobile app and follow the below screenshots
    Note: "User LED" is multiplexed with SWO pin functionality. Hence when device is in debug mode "User LED" will be solid ON always.

    ![](media/GUID-F35CA847-D772-45D6-BEBF-77F57E15AEDB-low.jpg)

    ![](media/GUID-DF72B5C2-0EDF-497F-B2EA-841651E11A21-low.png)

    ![](media/GUID-B81B9BEA-BB91-4C78-A4E0-657AF94E251C-low.png)

    ![](media/GUID-82E50B04-1B11-44A0-8A72-588065B50B9F-low.png)

    ![](media/GUID-373C241F-2FAB-43C8-A5B7-5A9D60E78727-low.png)

5.  BLE Connected Behavior \(Connected to "BLE Sensor" MBD Mobile Application\)

    1.  The Blue "User LED" will be solid On when connected
    ![](media/GUID-F35CA847-D772-45D6-BEBF-77F57E15AEDB-low.jpg)

    ![](media/GUID-F07DED69-1400-43C1-B63D-E6D9AC712827-low.png)

    ![](media/GUID-1E17D2E9-0550-48D4-9C89-E1C4B3939071-low.png)

6.  Firmware Version

    1.  Bluetooth SIG defined "Device Info Service" is implemented in the device to share the device information like "firmware version", "manufacture name", etc.,
    2.  Verify the firmware version shown on console with MBD app
    ![](media/GUID-611E2FB0-F042-4E1E-A9C2-4C22D5561F4F-low.jpg)

    ![](media/GUID-B0DE44A2-4E55-40A5-81BA-E0CDFB26D56B-low.png)

    ![](media/GUID-42F7900E-8043-415C-95A5-1E408512BFBC-low.png)

7.  The BLE Sensor mobile app will show the temperature \(deg C\) and LED status received from device, as well as allow the user to vary the RGB color and brightness

    ![](media/GUID-75D94050-BC5A-4AF9-BFD1-155CBB26AD7A-low.jpg)

    ![](media/GUID-1E17D2E9-0550-48D4-9C89-E1C4B3939071-low.png)

8.  The RGB LED can also be switched On/Off by pressing the On board "User Button". The On/Off LED status will also be reported to mobile app if connected

    1.  When the "User Button" is pressed and released, RGB LED is switched ON. When pressed and released again, the state will be toggled from the previous state
    ![](media/GUID-CE48E102-8CC4-47AB-9BFF-EF1FC62575B7-low.jpg)


## Programming the precompiled hex file using MPLABX IPE {#GUID-8BB18DC1-DCF5-418F-AEFE-E3B01FD47F1A .section}

1.  Precompiled Hex file is located here - ble\_sensor/precompiled\_hex

2.  Follow the steps mentioned [here](https://microchipdeveloper.com/ipe:programming-device)

    1.  Users should choose the correct Device and Tool information

3.  Follow the steps for running the [demo](#GUID-EC3CD8D8-3AFA-4DDA-9A6C-308F8AAC2074)


## Build and Program the Application {#GUID-3D55FB8A-5995-439D-BCD6-DEAE7E8E78AD .section}

The source code of BLE Sensor demo application is available in "apps/ble/advanced\_applications/ble\_sensor/" folder. If want to do changes in the demo code and would like to program/debug the customized code follow the below instruction.

1.  Complete the steps mentioned in [SDK Setup](https://onlinedocs.microchip.com/pr/GUID-A5330D3A-9F51-4A26-B71D-8503A493DF9C-en-US-1/index.html?GUID-5DEB6FE0-1234-4A15-A805-E451B3E73825)

2.  Open the "/firmware/ble\_sensor.X" MPLAB X project using the IDE

    ![](media/GUID-DE91FE89-273C-4378-9D41-507FA1D19E2F-low.png)

    ![](media/GUID-A3A3A6A1-8EAF-4BA8-BA04-F7140B386BB9-low.png)

3.  Make your changes in the code if needed \([Example to add new sensor command](#GUID-08A54102-FB77-41F7-A1B7-BF14EF322FB5)\). Clean and build your application by clicking on the Clean and Build button as shown below

    ![](media/GUID-A6F32937-5C58-47B3-B870-6945A30123AD-low.png)

4.  Program your application to the device, by clicking on the Make and Program button as shown below

    ![](media/GUID-0991A609-834A-44FA-892F-65A0395B811C-low.png)


## Extended Functionalities {#GUID-1D59565E-144B-424D-A9CB-C174E7EAC3BA .section}

This application also implements Low power mode \(Standby sleep mode\), BLE DFU \(Over-The-Air upgrade\) and Serial device firmware upgrade through serial UART interface features.

Low Power mode: Refer[low power configurations](https://onlinedocs.microchip.com/pr/GUID-A5330D3A-9F51-4A26-B71D-8503A493DF9C-en-US-1/index.html?GUID-994B6462-D0F1-4B8C-A97B-A9CBF20426C2.md) for details on the configurations.

BLE DFU: Refer to [Device firmware update over BLE](https://onlinedocs.microchip.com/pr/GUID-A5330D3A-9F51-4A26-B71D-8503A493DF9C-en-US-1/index.html?GUID-908446A4-F490-4063-9096-66C4831F9BE4) for details on code, configuration and running OTA demo procedure.

Serial DFU: Refer to[Device firmware update over serial](https://onlinedocs.microchip.com/pr/GUID-A5330D3A-9F51-4A26-B71D-8503A493DF9C-en-US-1/index.html?GUID-003E64BA-98A3-40EA-8417-ED7F09C14761) for details on upgrading the device over serial interface.

## Protocol Exchange {#GUID-15A824DB-F219-4270-BC99-E52E02DC50E7 .section}

The communication protocol exchange between BLE sensor mobile app \(BLE central\) and WBZ451 module \(BLE peripheral\) is explained [here](https://onlinedocs.microchip.com/pr/GUID-A5330D3A-9F51-4A26-B71D-8503A493DF9C-en-US-1/index.html?GUID-D320EA38-86CE-4C0B-8093-60C59E1F03AE)

## Application Flow Diagram {#SECTION_WFD_CK2_25B .section}

![](media/GUID-F36DA5AF-5ACC-4FCA-9EE5-FD16D20A590D-low.png)

## Expanding on the Out of Box Application {#GUID-A499798B-6951-4DF3-8782-57F883E6C581 .section}

The BLE Sensor demo protocol based on TRPS profile is a transparent request, response based and not tied only to this BLE sensor application. It can be expanded and customized as per the users need.

![](media/GUID-63F57E94-F8AE-45EA-93F6-D6C51B63BAF7-low.png)

-   The protocol parser is implemented in app\_trps.c and app\_trps.h
    -   **Request and Response:** when user wants to get and modify data on the device
    -   **Notify:** when there is a new data to be sent to mobile app.

-   The protocol commands are defined in app\_ble\_sensor.h and related sensor functionalities are implemented in app\_ble\_sensor.c
    -   **Request and Response:** When a command request is received from mobile app, app\_trps.c parse the data and forward to specific command callback handler in app\_ble\_sensor.c
    -   **Notify:** When specific Sensor data to be sent to mobile app, app\_ble\_sensor.c places the data in data base and calls for notify handler in app\_trps.c \(/ref APP\_TRPS\_SendNotification\(\)\)

**Example: To add a command to get temperature in Fahrenheit**

Complete the SDK Setup and open the ble\_sensor.X MPLABX project from MPLABX as in [Build and Program The Application](#GUID-409E287E-EF30-4179-9DA1-7B5375215230)

-   **Changes to - app\_ble\_sensor.h**

    1.  Define request, response, size of response payload \(not including size of length byte and response ID\)
    2.  Define a new control command ID
    3.  Define a new control command response
    4.  Define the length for the new response command
    5.  Inside the existing APP\_TRPS\_SensorData\_T structure, add the new data structure which holds the response data
    6.  Increase the list size BLE\_SENSOR\_CMD\_RESP\_LST\_SIZE by one
    7.  Add new command request/response set in BLE\_SENSOR\_DEFINE\_CTRL\_CMD\_RESP\(\) list

-   **Changes to - app\_ble\_sensor.c**

    1.  Read the temperature periodically and store in above defined data structure. Add the below code in App\_TRPS\_Sensor\_TimerHandler\(\)
    2.  After doing the above changes follow the instruction in [Build and Program the Application](#GUID-409E287E-EF30-4179-9DA1-7B5375215230)

-   **Testing with MBD mobile app**

    The "BLE Sensor" sub app in MBD mobile app is designed only to work with specific functionality. So, if new command is added in BLE sensor application, "BLE Sensor" mobile app will not know about it. To test the above newly added code, "BLE Smart" sub app in MBD can be used. Follow the below screenshots after launching MBD mobile app.

    ![](media/GUID-F5D19AC5-05AD-4698-979C-7701AE1930B3-low.png)

    ![](media/GUID-34FCEBC4-689B-4801-B064-AA116825BC23-low.png)

    ![](media/GUID-4859ADE2-8794-4192-AE2E-FA9CCD5C95D9-low.png)

    ![](media/GUID-3B4DF2F4-DE4B-4AF6-A62F-16F2EE803CD8-low.png)

    ![](media/GUID-2E4B9219-9302-446A-BFBD-A18EE51475C5-low.png)

    ![](media/GUID-B865B7BC-D4A2-4F93-984F-014E7A098190-low.png)

    ![](media/GUID-224B2444-4165-4041-B9C4-2A08B0DF49B0-low.png)


## Using MPLAB Code Configurator {#GUID-92AE4CBA-30C5-4596-9359-1AC3C5065686 .section}

MPLABX Harmony provides the MPLAB® Code Configurator \(MCC\) tool, a set of modular device and middleware libraries, and numerous example applications, all of which are designed to help developers to quickly and easily develop powerful and efficient embedded software.

MPLAB® Code Configurator \(MCC\) is a free graphical programming environment that generates seamless, easy-to-understand C code to insert into your project. Using an intuitive interface, it enables and configures a rich set of peripherals and functions specific to your application. The following steps can be followed to open MCC tool and generate the code.

1.  Open MCC tool from MPLABX

    ![](media/GUID-46F8AE07-05CC-4A94-BA08-1473021E2671-low.png)

2.  Once the MCC tool is launched, the "Project Graph" shows the list of MCC components used in the specific project

    ![](media/GUID-8AEA4672-5BD0-4296-8237-77F82C6DB95E-low.png)

3.  Clicking on the specific component block will allow to change the configuration of that component. eg:SERCOM0 is shown in the screenshot.

    ![](media/GUID-626F3B1B-D93C-421F-8CC2-55908E5F84FC-low.png)

4.  New component can also be added by drag and drop from "Device Resources" to "Project Graph". Then configure the component as in step \#3. eg: SERCOM1 can be added into "Project Graph"

    ![](media/GUID-A5F58C96-02AE-4B86-B9D2-9A2DD4D79F31-low.png)

5.  Once all the required components and configurations are done, [Generate Code](https://onlinedocs.microchip.com/pr/GUID-A5330D3A-9F51-4A26-B71D-8503A493DF9C-en-US-1/index.html?GUID-9C28F407-4879-4174-9963-2CF34161398E)

    ![](media/GUID-6AA89D86-BE14-4053-927A-100C7066F337-low.png)


-   **[Protocol Exchange](https://onlinedocs.microchip.com/pr/GUID-A5330D3A-9F51-4A26-B71D-8503A493DF9C-en-US-1/index.html?GUID-D320EA38-86CE-4C0B-8093-60C59E1F03AE)**  


**Parent topic:**[Advanced Applications](https://onlinedocs.microchip.com/pr/GUID-A5330D3A-9F51-4A26-B71D-8503A493DF9C-en-US-1/index.html?GUID-92137358-930E-4654-9431-E8BA0E260468)

