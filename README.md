# Multi-language Guest Dashboard for ESPHome
<img src="https://github.com/user-attachments/assets/369b15c7-1a71-4c7f-b978-df97bad4e748" width=80% height=80%>

---

## WARNING

**This project stores and exposes your guest network's password in plain text.**

**DO NOT implement if you don't understand the serious risks involved!**

---

## TL;DR.
### You'll need:
- UniFi Network
- Home Assistant instance running the Unifi Network Integration
- ESPHome host
- Waveshare 7.5 inch Display (2-tone grey)
- Waveshare driver board flashed with ESPHome

### Installation
- [ ] Make sure to properly isolate your guest network through the Unifi Network app.
- [ ] Add the automation rules from the *automations.yaml* project file to your HA's `automations.yaml` file.
- [ ] Declare all the entities from the *configuration.yaml* project file to your HA's `configuration.yaml` file.
- [ ] Copy the header file to the project folder on your ESPHome Host.
- [ ] Load up the `epaper_guest_deshboard.yaml` file on your ESPHome driver board and install.

#### Note that:
1. MDIs are imported as images.

    This means you must have `cairosvg` and `libcairo2-dev` packages installed on your ESPHome Host Platform.

    ```
    pip install cairosvg
    apt install libcairo2-dev
    ```

2. The QR integration on requires `zbar-tools` on HAOS.

    ```
    apk add zbar
    ```

3. All helper entities have been included in the `configuration.yaml` file.

   However, `binary_sensor.guest_display_deep_sleep_flag` does not support an `icon` option.

   To add one, you'll have to do so via the GUI. e.g., `mdi:sleep`.

And that's it.

---

### Hold on... Can I make this work without Unifi?

Absolutely!

1. **Ignore** the `automations.yaml` file.
2. Use the fles under the `no_unifi` directory, instead.
3. In `secrets.yaml` replace `your_ssid` and `your_password` with your own.
    Remember to use a 20-char long password. 

With that out of the way, let's dive into the details...

## Is this project right for you?

Skip to [implementation details](https://github.com/ozoidemi/ePaper_guest_dashboard/tree/main#Implementing-the-project) if you are comfortable securing a public wifi network.

Otherwise, please read on.

### What is the purpose of this project?
My parents love to entertain people. They also love living in an area with limited cell coverage. And they also have no qualms about making their tech problems someone else's tech problems...

So, when a guest comes around asking to "get wifi" (*\*\*shudders\*\**), guess who is now responsible to deal with the request...

As such, this project has two goals
- Present a device-agnostic response to non-tech visitors needing WAN access on a semi-isolated location.
- Relieve any tech person in the vicinity from having to play the on-call tech support role in any way.

### Who is this project intended for? (and who should stay away!)
People who:
1. Understand the pros and cons involved in managing convenience vs network security.
2. Hate captive portals.
3. Typically entertain friends and family.
4. Would be comfortable managing a public WiFi network.

So if you have trouble figuring out how (or even *why*) should you isolate all your client devices on your guest network, or if setting up VLANs and creating Firewall rules sounds like a hassle to you... Well, I really hope you learn about them one day because you are missing out.

Until then, please know that the downside of implementing this project far outweighs any gains that you could ever get from it.

Seriously.

### Anything else you need to know?

#### Fact 1 out of 3

Yes. I mentioned it already, but let's hit it one more time for the people in the back:

**DISCLAIMER**
**This project stores and exposes WiFi credentials in plain text!** Both on Home Assistant and on the ESPHome device.

Please thoroughly evaluate your own security or consult with a specialist if you are unsure. You know, the kind of people that actually know what they are talking about (unlike myself).

If you don't understand the risks involved, then do yourself a favor and **avoid using this project!**

#### Fact 2 out of 3

The Unifi Network Integration team decided against having the network password stored into a sensor. There are very good reasons for this.

In fact, at one point they had enabled this functionality, only to roll it back soon after due to security concerns. 

That should be understood in two ways:
1. Enabling a sensor to store a password will require some elbow grease.
2. Don't mess with this if you don't fully understand what you are doing.

#### Fact 3 out of 3

You should **DEFINITELY** know that the guy who developed the base QR code implementation—which was later adopted by the official Unifi Network integration team in HA—had this to say about my idea:

> You do you, but I for one will never do any of the things you just suggested. In fact, given what you just wrote, I question the need for you to even have a QR code at all.

[See for yourself](https://community.home-assistant.io/t/updated-automating-unifi-wifi-ssid-password-changes-and-qr-code-generation/380616/75?u=ozoidemi).

*Harsh?* Perhaps.

*Deservingly so?* Still perhaps.

But what kind of person sees no issue in forcing users into dealing with a 20-char long random password if their devices can't read the QR? The kind that is not trained to think about users. Because users are guests. And they probably don't understand the concept of "guests". Because they have never had any.

Moving on...

**FINAL WARNING - If you decide to proceed, know that you are doing so at your own risk!**

# Implementing the project

## What are the prerequisites?

- Home Assistant installation
  - Home assistant should be running the Unifi Network integration
- ESPHome host
- Network infrastructure based on Unifi Network

### What is my installation, you ask?

#### HA/ESPHome

A Lenovo Tiny running Proxmox 8. Everything else is built on top of it.

You can find very detailed installation instructions on how to set this up [here](https://community.home-assistant.io/t/installing-home-assistant-os-using-proxmox-8/201835).

If you haven't seen or used tteck's helper scripts before, please check out the amazing community managing [this project](https://github.com/community-scripts/ProxmoxVE) in his honor - he regretfully passed away in late 2024.

Here is the [direct link](https://community-scripts.github.io/ProxmoxVE/scripts) to the actual scripts.

At a minimum, make sure to run the following scripts post Proxmox installation.
1. [Post VE Install](https://community-scripts.github.io/ProxmoxVE/scripts?id=post-pve-install)
2. [Update Repo](https://community-scripts.github.io/ProxmoxVE/scripts?id=update-repo)
3. [HAOS Installer](https://community-scripts.github.io/ProxmoxVE/scripts?id=haos-vm)
4. [ESPHome Installer](https://community-scripts.github.io/ProxmoxVE/scripts?id=esphome)

Notice that default options are typically all you need when going through the helper scripts... ***except*** when installing ESPHome.

You see, there are cases in which you will need USB passthrough access to the LXC. And no matter what you do, you will not get it if you don't choose a _Privileged_ container type during set up.

To do so, just choose the advanced settings when the installer prompts you.
![Image](https://github.com/user-attachments/assets/5c41dfd7-b0d1-4d0c-a8c2-d71333cf7074)

Then choose all the defaults, except for the container type, which should be privileged.
![Image](https://github.com/user-attachments/assets/de530991-8717-4d6a-bf7d-6c2c9a157c31)

#### Network

Running a UXG-Pro with Unifi Network 9.0.114 on a local CK-Gen2. Also multiple U6+/U6 lite APs, and multiple USW-24-POE switches.

My guest network isolates all devices from one another in the same VLAN, and prevents them from accessing any other VLANs.

Ubiquiti provides some advice around [best practices for guest networks](https://help.ui.com/hc/en-us/articles/23948850278295-Best-Practices-Guest-WiFi).

For the purposes of this project, we'll assume your guest network SSID is `guests`.

### I have a Unifi Network infrastructure, but I don't have the integration.

[Skip ahead](https://github.com/ozoidemi/ePaper_guest_dashboard/tree/main#esphome-device) if this isn't your case and you have already activated the QR sensor.

If it is, get the integration installed following the instructions on the Home Assistant Unifi Network integration [page](https://www.home-assistant.io/integrations/unifi/).

Once the integration is up and running in HA, go to *Settings -> Devices & Services -> Entities*.

On the search box, search for "guests" (remember? the guest network SSID we'd assume moving forward?).

You will see a few entities here.
- sensor.guests
- switch.guests
- image.guests_qr_code
- button.guests_regenerate_password

The latter two entities should be disabled if you haven't touch them.

Click on them, then click on the cog in the upper right to go to settings, and then enable the entities.

Upon enablement, Home Assistant can now automatically generate a QR code with the SSID and password needed to log into your guests network. At first, it will have whatever password you assigned to your network.

However, your button entity will now enable you to create and apply a 20-char random password to your guest network without any work on your part. Just remember this is a fixed string, and there is nothing you can (easily) do to adapt it to better suit your needs.

We'll still take advantage of these two entities later on.

## ESPHome Device

### Device Hardware

This project makes use of:

1. [Waveshare 7.5inch e-Ink Raw Display](https://www.amazon.com/dp/B075R69T93).
2. [Waveshare Universal e-Paper Raw Panel Driver Board](https://www.amazon.com/dp/B07M5CNP3B).

It is worth mentioning that [Waveshare site](https://www.waveshare.com/product/) has updated their product catalog, and these are no longer available directly.

The closest ones are:
- [Display](https://www.waveshare.com/product/displays/e-paper/epaper-1/7.5inch-e-paper.htm)
  This new display has 4 gray scale levels, while the one used in this project only has two. 

- [Board](https://www.waveshare.com/product/displays/e-paper/driver-boards/e-paper-esp32-driver-board.htm)
  Here, the pinout definition *could* be different - different pages have different information.

#### Original Board - [Wiki](https://www.waveshare.com/wiki/E-Paper_ESP32_Driver_Board#Pins)
```
PIN  | ESP32 | Description
---  | ----- | -----------
VCC  | 3V3   | Power positive (3.3V power supply input)
GND  | GND   | Ground
DIN  | P14   | SPI's MOSI, data input
SCLK | P13   | SPI's CLK, clock signal input
CS   | P15   | Chip selection, low active
DC   | P27   | Data/Command, low for command, high for data
RST  | P26   | Reset, low active
BUSY | P25   | Busy status output pin (indicating busy)
```
#### New Board (Rev. 3) - [Product Page](https://www.waveshare.com/product/displays/e-paper/driver-boards/e-paper-esp32-driver-board.htm)
```
PIN  | ESP32 | Description
---  | ----- | -----------
VCC  | 3V3   | Power positive (3.3V power supply input)
GND  | GND   | Ground
DIN  | P12   | SPI's MOSI, data input
SCK  | P15   | SPI's CLK, clock signal input
CS   | P16   | Chip selection, low active
DC   | P11   | Data/Command, low for command, high for data
RST  | P10   | Reset, low active
BUSY | P9    | Busy status output pin (indicating busy)
```
At least for my HW revision, the wiki documentation works perfectly well.

### Flashing the device

This could be as much fun as having a dentist appointment for a root canal while suffering from explosive diarrhea.

I hope it isn't.

ESP32 devices can be very temperamental when it comes to ESPHome flashing. One way around is to connect the soon-to-be-flashed device directly to the ESPHome Host.

Since my host is a Proxmox LXC, I needed a USB passthrough to the container. If you find yourself in that situation, I hope you read my advice about [privileged containers](https://github.com/ozoidemi/ePaper_guest_dashboard/tree/main)

If you did, just plug in the device and go. And if that worked, just [skip ahead](https://github.com/ozoidemi/ePaper_guest_dashboard/tree/main#programming-the-display)

If for some reason it didn't, you can try the following:

Go to your web browser and type your Proxmox VE IP address, with port 8006.

If you have a typical network address, this should look like `https://192.168.1.X:8006`.

Once in your PVE environment, go to the PVE shell and type `lsusb`. You should get something like this:

![Image](https://github.com/user-attachments/assets/51111408-88cb-448f-9f42-8b8645c030e0)

Notice the QinHeng device. This is my ESP32 driver board.

```
Bus 001 Device 002: ID 1a86:55d3 QinHeng Electronics USB Single Serial
```

Now go to your ESPHome container and go to *Resources -> Add -> Device Passthrough*.

![Image](https://github.com/user-attachments/assets/069c07c7-b03e-47eb-97e0-2ca736c1e58a)

On the dialog, type the info from your device where "dev" is the number of the device Bus (001), and "xyz" is the number of the device itself (002).

![Image](https://github.com/user-attachments/assets/139d92ab-eb02-4888-b3e8-6598fb1be75a)

You should now see something like this:

![Image](https://github.com/user-attachments/assets/9d20faba-b4cf-4fa5-9153-1aaf0cc42276)

If at this point you still don't have USB passthrough to your ESPHome Host, there is something else going on. Maybe a bad device, or a bad port, or something else. 

I won't cover further troubleshooting here.

### Programming the display

This involves adding yaml entries to HA (*configuration.yaml*, and *automations.yaml*), creating a header file within the ESPHome Host, and adding the ESPHome device yaml. 

#### Home Assistant

##### Automations.yaml

On the file editor, search for the `/homeassistant/automations.yaml` file and add the two entries from the automations.yaml project file.

If you don't have the file editor, you can add it from the add-on store (*Settings -> Add-ons -> Add-on store*).

Also, if you don't know exactly *where* to add these entries to the file, know that the file you are seeing is my entire automations file at this point. So you can just copy and paste its contents, or completely replace it via HA's file editor.

The first part takes a snapshot of the QR code (`image.guests_qr_code`) and stores it in `/config/www/wifi_qr.png`.

The second ensures that the guest network password is updated automatically at least weekly. If you need a different schedule, then adjust accordingly. It will not break anything, and you could easily remove it altogether.


> **I don't get it... why are we taking a snapshot of a QR code that already exists?**
>
> Because even when the Unifi Network Integration generates a QR code for us automatically, it:
> 1. Only exists in memory.
> 2. Is much harder to use than what you might think.
>
> If you know the right URL, you can get access to the QR. However, this access is very short lived due to the way the URL is created - it relies on the entity's attributes, which are continuously changing.
>
>  Also, if you know enough about HA templates, you can get a template expression that will allow you to stay one step ahead of the changes. Regretfully, the QR will continue to be useless.
>
> Unless you save the QR, you almost can't do anything with it. That's why I used the snapshot automation.


**Configuration.yaml**

Back into the file editor, go to `/homeassistant/configuration.yaml`.

Here, add the contents of the *configuration.yaml* project file into your HA's `configuration.yaml` file.

These are:

1. The `image_processing` entry.
2. The `input_select` entry.
3. All the `template` elements.

Make sure the spacing is correct to avoid weird errors. If you already have any of these sections, just append the new entries at the end of each section.

This will create all the sensor entities that you'll need for ESPHome, and refresh all the data we'll be presenting on the Display.

The final step is to install `zbar-tools`, which are necessary for the [QR code integration](https://www.home-assistant.io/integrations/qrcode) to work.

To do so, you need to get the Terminal add-on. Just go to *Settings -> Add-ons -> Add-on Store" and look for the Advanced SSH & Web Terminal.

Once its up, run the following command.

```
apk add zbar
```

### ESPHome Host

1. First go to the ESPHome host on PVE.
2. Open the shell.
3. Go to the `config/custom_components` directory. `cd config/custom_components`
4. Create a new file called `canvas_struct.h'. You can use `nano canvas_struct.h`
5. Paste the contents of the `canvas_struct.h' project file and save.

You can quickly verify that the content of the file is correct using the `cat` command.
```
cat canvas_struct.h
```

### ESPHome Device

Just load up the `epaper-guest-display.yaml` file onto your device's yaml. 

Make sure to update the following substitutions / relevant values at the beginning:

1. Update `guest_ssid_switch: "switch.guests"` to match the SSID for your guest wifi.
2. Adjust ln. 63 and 64 in case your screen wasn't 800x480 px. 

Then make sure to define these entries on your `secrets.yaml` file:
   
1. Define `homeassistant_api_encryption_key`.
2. Define `ota_update_password`.
3. Define `wifi_ssid` and `wifi_password`.
    - These define the wifi network your ESPHome device will connect to. As such, these not only **SHOULD** be different from the credentials that will be displayed on your screen, but also should be kept **completely confidential**.
4. Define `wifi_ssid_fallback` and `wifi_password_fallback` in case the wifi connection fails.

Then just Install, kick back and wait for the screen to retrieve and load up all the info you have.

#### Note on ESP_ERR_NVS_NOT_ENOUGH_SPACE

If during the compilation and programming of your ESPHome device, you ever get this NVS error, use the following commands.

```
dd if=/dev/zero of=nvs_zero bs=1 count=20480
esptool.py --chip esp32 --port /dev/ttyACM0 write_flash 0x009000 nvs_zero
```

The first command writes a file of null bytes the typical size of your NVS, naming it nvs_zero.

The second command writes the file on your ESPHome device at 0x009000, which is the typical location for the NVS file. Note that `/dev/ttyACM0` depends on your installation, and you will need to verify the rigth entry for your project.

## Bonus
### Presentation
There are many ideas around how to present your screen. From handmade stands, to Ikea frames and everything in between.

In my mind, this project would benefit from a stand like this:
https://www.hackster.io/lmarzen/esp32-e-paper-weather-display-a2f444

What I'd do differently though, is the USB-C connector. Instead of having it connected straight in the back, I would place the connector facing downwards - just like the power plug of a Sonos One speaker - with a 90-degree cable.

That would hide the connector nicely, and would allow you to see a single straight cable coming from the bottom of the base. It you haven't seen it, I can tell you it looks super clean!

The challenge with the approach is the length of the connector. If you are ever in the need to replace it, you might find it hard to find the same original one, or one with the same dimensions.

With that in mind, a potential solution is to keep the connector straight, but move further into the base

FYI: These pictures aren't mine. They are here just to illustrate the type of connector I'd use and why.

![Image](https://github.com/user-attachments/assets/2d5c4c79-645e-4da3-a7ba-aa9d328ff006)

![Image](https://github.com/user-attachments/assets/7dabbb9d-426a-44e0-ab22-7767d7c1ba43)

![Image](https://github.com/user-attachments/assets/917fc204-fbc2-4dfd-abe5-21a84617246e)
