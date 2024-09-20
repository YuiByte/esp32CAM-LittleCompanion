# esp32-CAM
This ESP32 project allows you to create a little companion that takes pictures of what you're doing every 10 minutes and sends them to a local server at home.
That server will analyze the images, generate descriptions using AI, and add them to a local web server.

In the future, the idea might be to add cool features like sharing galleries using Immich (a self-hosted photo and video management solution),
to add more detailed pictures of life and analyze them in the same way as the ESP32-CAM pictures.

# Working Process (Now)
![](https://github.com/YuiByte/esp32-CAM/blob/main/img/Working%20Process%20(Now).png)

# Working Process (At least)
![](https://github.com/YuiByte/esp32-CAM/blob/main/img/Working%20Process%20(At%20least).png)


# Components (Now)
- ESP32-CAM
- 1000mAh 3,7v Battery

# Components (At least)
- Seeed Studio XIAO ESP32 S3
- ON/OFF switch button
- 300mAh 3,7v Battery

My idea is to have a smaller device at the end of the project.
It's simple to switch to a different device; only a few lines of code will need to be changed.

Later, I will add information about the battery duration for 100mAh, 300mAh, and 1000mAh to see the device's power consumption and how long it can stay powered on.

# Next Functionalities that will be added :
- Send message on telegram when battery’s low (20%) & When it's charged (around 80%)