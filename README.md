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
- [ ] Copy the `canvas_struct.h` header file to the project folder on your ESPHome Host.
- [ ] Load up the `epaper_guest_deshboard.yaml` file on your ESPHome driver board.
- [ ] Adjust the substitutions, add the corresponding passwords to your `secrets.yaml` file, and install.

#### Note that:
1. MDIs are imported as images.

    This means you must have `cairosvg` and `libcairo2-dev` packages installed on your ESPHome Host Platform.

    ```
    pip install cairosvg
    apt install libcairo2-dev
    ```

2. The Home Assistant QR integration requires `zbar-tools`.
   
    To add it, just execute the following command on HA's terminal:

    ```
    apk add zbar
    ```

4. All helper entities have been included in the `configuration.yaml` file.

   However, `binary_sensor.guest_display_deep_sleep_flag` does not support an `icon` option.

   To add one, you'll have to do so via the GUI (e.g., `mdi:sleep`).

And that's it.

---

### Hold on... Can I make this work without Unifi?

Absolutely! Though, the following code hasn't been tested. That's my next step.

To try it out yourself:

1. **Ignore** the `automations.yaml` file.
2. Use the files under the `no_unifi` directory.
3. In `secrets.yaml` replace `your_ssid` and `your_password` with your own.
    - Remember to use a 20-char long password.
    - Also remember this is the HA's `secrets.yaml` file.

---

With that out of the way, let's dive into the details...

## Project Details

### What is the purpose of this project?

My parents love to entertain people. They also love living in an area with limited cell coverage. And they also have no qualms about making their tech problems someone else's tech problems...

As such, this project has two goals
- Present a device-agnostic response to non-tech visitors needing WAN access on a semi-isolated location.
- Relieve any tech person in the vicinity from having to play the on-call tech support role in any way, shape, or form.

### Who is this project intended for? (and who should stay away!)

People who:
1. Understand the pros and cons involved in managing convenience vs network security.
2. Hate captive portals.
3. Typically entertain friends and family.
4. Would be comfortable managing a public WiFi network.

If you are ok with those, please skip to [implementation details](https://github.com/ozoidemi/ePaper_guest_dashboard/tree/main#detailed-implementation).

Otherwise, please read on.

If you have trouble figuring out how (or even *why*) should you isolate all your client devices on your guest network, or if setting up VLANs and creating Firewall rules sounds like a hassle to you... Well, I really hope you learn about them one day because you are missing out.

Until then, please know that the downside of implementing this project far outweighs any gains that you could ever get from it.

Seriously.

### Anything else you need to know?

#### Fact 1 out of 3

Yes. I mentioned it already, but let's hit it one more time for the people in the back:

**DISCLAIMER**
**This project stores and exposes WiFi credentials in plain text!**

Both on Home Assistant and on the ESPHome device.

Please thoroughly evaluate your own security or consult with a specialist if you are unsure. You know, the kind of people that actually know what they are talking about (unlike myself).

If you don't understand the risks involved, then do yourself a favor and **avoid using this project!**

#### Fact 2 out of 3

The Unifi Network Integration team decided against having the network password stored into a sensor.

There are very good reasons for this.

In fact, at one point they had enabled this functionality, only to roll it back soon after due to security concerns.

With great power comes great responsibility... thankfully, no power means no responsibility, right? RIGHT?

#### Fact 3 out of 3

You should **DEFINITELY** know that the guy who developed the base QR code implementation—which was later adopted by the official Unifi Network integration team in HA—had this to say about my idea:

> You do you, but I for one will never do any of the things you just suggested. In fact, given what you just wrote, I question the need for you to even have a QR code at all.

[See for yourself](https://community.home-assistant.io/t/updated-automating-unifi-wifi-ssid-password-changes-and-qr-code-generation/380616/75?u=ozoidemi).

*Harsh?* Perhaps.

*Deservingly so?* Still perhaps.

I just don't agree with shoving a 20-char long random password down my users' throat. Especially when it is possible that the QR won't work for them in the first place.

Besides, the password is already in plain text the moment its printed in a QR format, so... meh.

Moving on.

**FINAL WARNING - If you decide to proceed, know that you are doing so at your own risk!**

# Detailed Implementation

## ESPHome Device

### Device Hardware

This project makes use of:

1. [Waveshare 7.5inch e-Ink Raw Display](https://www.amazon.com/dp/B075R69T93).
2. [Waveshare Universal e-Paper Raw Panel Driver Board](https://www.amazon.com/dp/B07M5CNP3B).

It's worth mentioning that [Waveshare site](https://www.waveshare.com/product/) has updated their product catalog, and these devices are no longer available directly.

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

I hope it isn't for you.

ESP32 devices can be very temperamental when it comes to ESPHome flashing. One way around is to connect the soon-to-be-flashed device directly to the ESPHome Host.

Since my host is a Proxmox LXC, I needed a USB passthrough to the container.

If you find yourself in that situation, I hope you read my advice about [privileged containers](https://github.com/ozoidemi/ePaper_guest_dashboard/tree/main#privileged-container-type).

Check the [troubleshooting](https://github.com/ozoidemi/ePaper_guest_dashboard/tree/main#troubleshooting) section if you want to know more about my own installation, or if you have further issues.

### Programming the display

#### Home Assistant

##### Automations.yaml

On HA's file editor, search for the `/homeassistant/automations.yaml` file and add the two entries from the automations.yaml project file.

If you don't have the file editor, you can add it from the add-on store (*Settings -> Add-ons -> Add-on store*).

Also, if you don't know exactly *where* to add these entries to the file, know that the file you are seeing is my entire automations file at this point. So you can just copy and paste its contents, or completely replace it via HA's file editor.

The first entry takes a snapshot of the QR code (`image.guests_qr_code`) and stores it in `/config/www/wifi_qr.png`.

The second ensures that the guest network password is updated automatically at least weekly. If you need a different schedule, then adjust accordingly. It will not break anything, and you could easily remove it altogether.

> **I don't get it... why are we taking a snapshot of a QR, storing and decoding it, and then sending the decoded info to ESPHome so that it can be rebuilt from scratch? Can't we just pass over the existing QR and be done with it?**
>
> Not really, no.
> 
> Even when the Unifi Network Integration generates a QR code for us automatically, it:
> 1. Only exists in memory.
> 2. Is much harder to use than what you might think.
>
> If you know the right URL, you can get access to the QR. However, this access is very short lived due to the way the URL is created - it relies on the entity's attributes, which are continuously changing.
>
>  Also, if you know enough about HA templates, you can get a template expression that will allow you to stay one step ahead of the changes. Regretfully, the QR will continue to be useless.
>
> Unless you save the QR, you almost can't do anything with it. That's why I used the snapshot automation.
>
> In fact, even if you were to send the QR as an image to the ESPHome device, you'd still need to get the snapshot first.

#### Configuration.yaml

Back into the file editor, go to `/homeassistant/configuration.yaml`.

Here, load up all the entities into your HA's `configuration.yaml` file.

You should see the following entries, each with a differnet number of elements:

1. `image_processing`.
2. `input_select`.
3. `input_boolean`.
4. `binary_sensor`.
5. `template`.

If you already have any of these sections, just append the new entries at the end of each, and make sure to respect the spacing.

The file includes all the entities that you will need. Other than adding an icon to `guest_display_deep_sleep_flag`, you wont need to create nor modify any entities via the GUI.

#### CLI
The final step is to install `zbar-tools`, which are necessary for the [QR code integration](https://www.home-assistant.io/integrations/qrcode) to work.

To do so, just use the Terminal add-on.

If you don't have it already, then go to *Settings -> Add-ons -> Add-on Store" and look for the Advanced SSH & Web Terminal.

Once its up, run the following command.

```
apk add zbar
```

That should be all from the Home Assistant side.

### ESPHome Host

1. First open the CLI on your ESPHome host.

    - For me, that means getting to my PVE console, selecting the LXC container, and then opening the console for the container.

2. Go to the `config/custom_components` directory.

   ```
   cd config/custom_components
   ```

3. Copy the `canvas_struct.h` file into that directory.
    
    - If you don't know how, you can:
        
        - Create a new file called `canvas_struct.h'.
            
          ```
          nano canvas_struct.h
          ```

        - Paste the contents of the `canvas_struct.h' project file.
        - And save (ctrl + s, ctrl + x).

4. Verify that the content of the file is correct using the `cat` command.

    ```
    cat canvas_struct.h
    ```

Two down. One more to go.

### ESPHome Device

Load up the `epaper-guest-display.yaml` file onto your device's yaml. 

Make sure to update the following substitutions / relevant values at the beginning:

1. Update `guest_ssid_switch: "switch.guests"` to match the SSID for your guest wifi.

2. Make sure to define the following entries on your `secrets.yaml` file:

    1. `homeassistant_api_encryption_key` for your HA API.

    2. `ota_update_password` for your OTA functionality.

    3. `wifi_ssid` and `wifi_password` for your Wifi

        - These define the wifi network your ESPHome device will connect to.

        - This is NOT your guest wifi!

        - The credentials **MUST** be different from the credentials that will be displayed on your screen.

        - Also, they **MUST** be kept **completely confidential**.

    4. `wifi_ssid_fallback` and `wifi_password_fallback` for your Fallback Hotspot.

3. On the `esphome` entry, look at the `on_boot` lambda.

    - Make sure to adjust `canvas.width` and `canvas.height` if your screen isn't 800x480 px. 

Then just install, kick back, and wait for the screen to retrieve and load up all the info you have.

Congrats! you should now have a functional display to present to your guests!

## Troubleshooting

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

#### Privileged Container Type

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

### My Network infrastructure is based on Unifi, but I don't have the integration.

You can install it by following the instructions on the Home Assistant Unifi Network integration [page](https://www.home-assistant.io/integrations/unifi/).

Once the integration is up and running in HA, go to *Settings -> Devices & Services -> Entities*.

On the search box, search for "guests" (remember? the guest network SSID we'd assume moving forward?).

You will see a few entities here.
- sensor.guests
- switch.guests
- image.guests_qr_code
- button.guests_regenerate_password

The latter two entities should be disabled if you haven't touch them.

To enable them, click on the entity, then click on the cog in the upper right to go to settings, and then enable the entities. Repeat for the second one.

Upon enablement, Home Assistant can now automatically generate a QR code with the SSID and password needed to log into your guests network. At first, it will have whatever password you assigned to your network.

However, your button entity will now enable you to create and apply a 20-char random password to your guest network without any work on your part. Just remember this is a fixed string, and there is nothing you can (easily) do to adapt it to better suit your needs.

### I'm using a LXC for the ESPHome host, but I need more help my ESPHome device directly to it.

Let's try forcing the passthrough.

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

If at this point you still don't have USB passthrough to your ESPHome Host, there is something else going on. Maybe a bad device, or a bad port.

I won't cover further passthrough troubleshooting here.

### I'm getting an ESP_ERR_NVS_NOT_ENOUGH_SPACE error!

After multiple uploads, your device's Non-Volatile Storage might need some housekeeping.

If you ever get this NVS error during the compilation and programming of your ESPHome device, then use the following commands.

```
dd if=/dev/zero of=nvs_zero bs=1 count=20480
esptool.py --chip esp32 --port /dev/ttyACM0 write_flash 0x009000 nvs_zero
```

The first command writes a file of null bytes (the typical size of your NVS), naming it nvs_zero.

The second command writes the file on your ESPHome device at 0x009000, which is the typical address for the NVS file.

Note that `/dev/ttyACM0` depends on your installation, and you will need to verify the rigth entry for your project.

## Bonus

### Presentation

There are many ideas around how to present your screen. From handmade stands, to Ikea frames and everything in between.

In my mind, this project would benefit from a stand like this:
https://www.hackster.io/lmarzen/esp32-e-paper-weather-display-a2f444

What I'd do differently though, is the USB-C connector. Instead of having it connected straight in the back, I would either place the connector facing downwards - just like the power plug of a Sonos One speaker - with a 90-degree cable, or offset it into the base, at the bottom of it.

That would hide the connector nicely. Guests would only see a single straight cable coming from the bottom of the base.

If you haven't seen something like that, it looks super clean!

The challenge with the 90-degree connector approach might be the length of the connector. If you are ever in the need to replace it, you might find it hard to find the same original one, or one with the same dimensions. So it could become a pain.

From that perspective, an straight but offset connector at the bottom migth be the best bet. 

FYI: These pictures aren't mine. They are here just to illustrate the kind of look I'd go after.

![Image](https://github.com/user-attachments/assets/2d5c4c79-645e-4da3-a7ba-aa9d328ff006)

<img src="https://github.com/user-attachments/assets/7dabbb9d-426a-44e0-ab22-7767d7c1ba43" width=50% height=50%>

![Image](https://github.com/user-attachments/assets/917fc204-fbc2-4dfd-abe5-21a84617246e)

## TODO

As the first half-decent version, I still have a lot of work to do in this implementation.

Things I'm thinking of in no particular order:

- Clean up the code.

    - There is more flexibility than it is needed, so the code can be difficult to follow.

    - Lots of unused variables, or poorly defined ones (looking at you canvas_struct.h!).
      
- Rework the prevent_deep_sleep logic.

    - I've been testing it a lot and I don't think it is working like it should.

- Stream-line the ESP messages.

    - Talking about a mess... There is no consistency whatsoever, and many messages are repeated.

- Fully test the **no-unifi** version.

    - That needs much more attention than what I've given
 
- Change the whole approach to use the existing (but removed) Unifi integration component!


## References

There are lots of comments, articles, projects, and tools that allowed me to develop this little solution. Either through inspiration, or problem solving. They are all worth taking a look at.

**Inspiration**
- https://github.com/Madelena/esphome-weatherman-dashboard
- https://github.com/maxmacstn/HA-ePaper-Display
- https://github.com/DeastinY/esphome-waveshare-e-paper-dashboard
- https://github.com/lmarzen/esp32-weather-epd
- https://www.printables.com/model/994770-waveshare-75-e-ink-display-insert-for-ikea-rodalm/files

**Tools**
- https://moqups.com/

    - Excellent tool to design this type of mockups

- claude.ai

    - Seriously, incredibly easy to define templates with it.    

**Problem Solving**
- https://community.home-assistant.io/t/where-are-helpers-stored-when-created-in-the-gui/347556
- https://pocketables.com/2022/01/how-to-format-that-wifi-qr-code-in-plain-text.html
- https://tatham.blog/2021/02/06/esphome-batteries-deep-sleep-and-over-the-air-updates/
- https://community.home-assistant.io/t/astimezone-output-differs-from-server-local-time-zone/850071/2
- https://community.home-assistant.io/t/error-unable-to-import-component-image-on-e-paper-display/712496/5
- https://community.home-assistant.io/t/updated-automating-unifi-wifi-ssid-password-changes-and-qr-code-generation/380616/87
- https://community.home-assistant.io/t/unifi-network-integration-official-thread/486308/79
- https://community.home-assistant.io/t/definitive-guide-to-weather-integrations/736419

Among many others!
