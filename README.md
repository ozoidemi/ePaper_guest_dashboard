# Multi-language Guest Dashboard for ESPHome
<img src="/resources/overview.jpg" width=80% height=80%>

---

## WARNING

**This project stores and exposes your guest network's password in plain text.**

**DO NOT implement if you don't understand the serious risks involved!**

Ubiquiti published a set of best practices for guest networks. [Make sure to understand them.](https://help.ui.com/hc/en-us/articles/23948850278295-Best-Practices-Guest-WiFi).

---

## TL;DR.
### You'll need:
- UniFi Network
- Home Assistant running the Unifi Network Integration
- ESPHome host
- Waveshare 7.5 inch Display (2-tone grey, 800x480px)
- Waveshare driver board flashed with ESPHome

### Installation
- [ ] Isolate your guest network.
    - [ ] For minimal modifications use SSID "guests".
- [ ] Add automation rules to HA - `automations.yaml` file.
- [ ] Declare all the configuration entities on your HA - `configuration.yaml` file.
- [ ] Copy the `canvas_struct.h` header file to `~/custom_components/` on your ESPHome Host.
- [ ] Load up the `epaper_guest_dashboard.yaml` file on your ESPHome driver board.
    - [ ] Adjust the esphome name and friendly name
    - [ ] Ensure the `secrets.yaml` file includes entries for:
        - [ ] `wifi_ssid`
        - [ ] `wifi_password`
        - [ ] `wifi_ssid_fallback`
        - [ ] `wifi_password_fallback`
        - [ ] `homeassistant_api_encryption_key`
        - [ ] `ota_update_password`
- [ ] Create a HA dashboard card to control your display from HA (refresh password on demand or turn wifi off).

#### Note that:
1. MDIs are imported as images.

    You'll need `cairosvg` and `libcairo2-dev` on the ESPHome Host

    ```
    pip install cairosvg
    apt install libcairo2-dev
    ```

2. The Home Assistant QR integration requires `zbar-tools`.
   
    On HA's terminal run

    ```
    apk add zbar
    ```

3. The `configuration.yaml` file includes all helper entities.

   However, `binary_sensor.guest_display_deep_sleep_flag` does not support an `icon` option.

   This must be done via GUI (e.g., `mdi:sleep`).

And that's it.

---

### Hold on... Can I make this work without Unifi?

Yes, but you'd lose all the automation advantages.

Also, I wrote the code, but haven't tested it (yet).

Essentially, you'd need to manually define a `wifi_ssid` and a `wifi_password` for your network from Home Assistant.

From then on, the functionality already in place should take care of the rest.

To try it:

1. **Ignore** the `automations.yaml` file.
2. Use the files under the `no_unifi` directory.
3. In HA's `secrets.yaml`, replace `your_ssid` and `your_password` with your own.
    - Password must be 20 chars long.

---

With that out of the way, let's dive into the details...



## Project Details

### What is the purpose of this project?

My parents love to entertain people. They also love living in an area with limited cell coverage. And they also have no qualms about making their tech problems someone else's tech problems...

As such, this project has two goals:
- Present a device-agnostic response to non-tech visitors needing WAN access on a semi-isolated location.
- Relieve any tech person in the vicinity from having to play the on-call tech support role in any way, shape, or form.

### Who is this project intended for? (and who should stay away!)

People who:
1. Understand the pros and cons involved in managing convenience vs network security.
2. Hate captive portals.
3. Typically entertain friends and family.
4. Would be comfortable managing a public WiFi network.

If you are ok with those, please skip to [implementation details](https://github.com/ozoidemi/ePaper_guest_dashboard/tree/main#detailed-implementation).

Otherwise, please know that the downside of implementing this project far outweighs any gains that you could ever get from it.

Seriously.

### Anything else you need to know?

#### Fact 1 out of 3

Let's hit it one more time for the people in the back:

**DISCLAIMER**
**This project stores and exposes WiFi credentials in plain text!**

Both on Home Assistant and on the ESPHome device.

Please thoroughly evaluate your own security or consult with a specialist if you are unsure. You know, the kind of people that actually know what they are talking about (unlike myself).

If you don't understand the risks involved, then do yourself a favor and **avoid using this project!**

#### Fact 2 out of 3

The HA team in charge of the Unifi Network Integration decided against having the network password stored into a sensor.

There are very good reasons for this.

In fact, at one point they had enabled this functionality, only to roll it back soon after due to security concerns.

With great power comes great responsibility.

#### Fact 3 out of 3

You should **DEFINITELY** know that the guy who developed the base QR code implementation—which was later adopted by the official Unifi Network integration in HA—had this to say about my idea:

> You do you, but I for one will never do any of the things you just suggested. In fact, given what you just wrote, I question the need for you to even have a QR code at all.

[See for yourself](https://community.home-assistant.io/t/updated-automating-unifi-wifi-ssid-password-changes-and-qr-code-generation/380616/75?u=ozoidemi).

*Harsh?* Perhaps.

*Deservingly so?* Still perhaps.

I just don't agree with shoving a 20-char long random password down my users' throat.

Especially when it is possible that the QR won't work for all of them in the first place - what if users need to connect a regular laptop? Or their camera doesn't work? Or their device simply fails to read the QR?

Besides, the password already exists in plain text the moment it's printed in a QR format, and there is the pesky issue of needing to be in physical proximity to exploit it... so...

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

- [This display](https://www.waveshare.com/product/displays/e-paper/epaper-1/7.5inch-e-paper.htm), which is a new version with 4 grayscale levels. The one used in this project is B&W only.

- [This driver board](https://www.waveshare.com/product/displays/e-paper/driver-boards/e-paper-esp32-driver-board.htm). The pinout definition *could* be different, but I'm not sure. Different pages have different information.

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

This could be as fun as having a dentist appointment for a root canal while suffering from explosive diarrhea.

Hopefully your fun won't be of *that* kind.

ESP32 devices can be very temperamental when it comes to ESPHome flashing. One way around is to connect the soon-to-be-flashed device directly to the ESPHome Host.

Since my host is a Proxmox LXC, I needed a USB passthrough to the container.

If you find yourself in that situation, I hope you read my advice about [privileged containers](https://github.com/ozoidemi/ePaper_guest_dashboard/tree/main#privileged-container-type-and-disk-size).

Check the [troubleshooting](https://github.com/ozoidemi/ePaper_guest_dashboard/tree/main#troubleshooting) section if you want to know more about my own installation, or if you need further assistance.

### Programming the display

#### Home Assistant

##### Automations.yaml

On HA's file editor, search for the `/homeassistant/automations.yaml` file and add the two entries from the automations.yaml project file.

If you don't have the file editor, you can add it from the add-on store (*Settings -> Add-ons -> Add-on store*).

Also, if you don't know exactly *where* to add these entries to the file, know that the file you are seeing is my entire automations file at this point - you can just copy and paste as-is.

The first entry takes a snapshot of the QR code (`image.guests_qr_code`) and stores it in `/config/www/wifi_qr.png`.

The second ensures that the guest network password is updated automatically at least weekly. If you need a different schedule, then adjust accordingly. It will not break anything, and you could easily remove it altogether.

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

If you don't have it already, then go to *Settings -> Add-ons -> Add-on Store* and look for the Advanced SSH & Web Terminal.

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



# Troubleshooting

## I don't get why this is so convoluted. Can't we just pass over the existing QR and be done with it?

Maybe? Regretfully I didn't another way.
 
Even when the UI Network Integration generates a QR code for us automatically, it:

1. Only exists in memory.
2. Is much harder to use than what you might think.

I won't go into all the details, except for saying that I did NOT find a way to use the in-memory QR without storing it.

If you want to try a different approach, you can implement a custom component using the rolled back PR from the UI Network Integration - this project is publicly available.

## What is my installation, you ask?

### HA/ESPHome

A Lenovo Tiny running Proxmox 8. Everything else is built on top of it.

Instructions can be found [here](https://community.home-assistant.io/t/installing-home-assistant-os-using-proxmox-8/201835).

Also, check out the amazing community managing [tteck's scripts](https://github.com/community-scripts/ProxmoxVE). He regretfully passed away in late 2024, but his scripts definitely live on.

Here is the [direct link](https://community-scripts.github.io/ProxmoxVE/scripts) to the actual scripts.

At a minimum, make sure to run the following scripts post Proxmox installation.
1. [Post VE Install](https://community-scripts.github.io/ProxmoxVE/scripts?id=post-pve-install)
2. [Update Repo](https://community-scripts.github.io/ProxmoxVE/scripts?id=update-repo)
3. [HAOS Installer](https://community-scripts.github.io/ProxmoxVE/scripts?id=haos-vm)
4. [ESPHome Installer](https://community-scripts.github.io/ProxmoxVE/scripts?id=esphome)

Notice that default options are typically all you need when going through the helper scripts... ***except*** when installing ESPHome.

#### Privileged Container Type and Disk Size

There are cases in which you'll need USB passthrough access to the LXC. If you don't choose a _Privileged_ container type during set up, you won't get it.

To do so, just choose the advanced settings when the installer prompts you.

![Image](/resources/pve_advanced.png)

Then choose all the defaults, except for the disk size and the container type.

For the disk size, you should allocate a minimum of 8GB, instead of the default 4GB. This isn't related to the container type, but will solve other issues.

![Image](/resources/pve_disk_size.png)

For the container type, simply choose "privileged".

![Image](/resources/pve_privileged.png)

### Network

Running a UXG-Pro with Unifi Network 9.0.114 on a local CK-Gen2. Also multiple U6+/U6 lite APs, and multiple USW-24-POE switches.

My guest network isolates all devices from one another within the same VLAN, and prevents them from accessing any other VLANs.

Ubiquiti provides some advice around [best practices for guest networks](https://help.ui.com/hc/en-us/articles/23948850278295-Best-Practices-Guest-WiFi).

For the purposes of this project, I'll assume your guest network SSID is `guests`.

## My Network infrastructure is based on Unifi, but I don't have the integration.

You can install it by following the instructions on the Home Assistant Unifi Network integration [page](https://www.home-assistant.io/integrations/unifi/).

Once the integration is up and running in HA, go to *Settings -> Devices & Services -> Entities*.

On the search box, search for "guests" (remember? the guest network SSID we'd assume moving forward?).

You will see a few entities here.
- `sensor.guests`
- `switch.guests`
- `image.guests_qr_code`
- `button.guests_regenerate_password`

The latter two entities should be disabled if you haven't touch them.

To enable them, click on the entity, then click on the cog in the upper right to go to settings, and then enable the entities. Repeat for the second one.

Upon enablement, Home Assistant can now automatically generate a QR code with the SSID and password needed to log into your guests network. At first, it will have whatever password you assigned to your network.

However, your button entity will now enable you to create and apply a 20-char random password to your guest network without any work on your part. Just remember this is a fixed string, and there is nothing you can (easily) do to adapt it to better suit your needs.

## I'm using a LXC for the ESPHome host, but I need more help to connect my ESPHome device directly to it.

Let's try forcing the USB passthrough.

Go to your web browser and type your Proxmox VE IP address, with port 8006.

If you have a typical network address, this should look like `https://192.168.1.X:8006`.

Once in your PVE environment, go to the PVE shell and type `lsusb`. You should get something like this:

![Image](/resources/pve_lsusb.png)

Notice the QinHeng device. This is my ESP32 driver board.

```
Bus 001 Device 002: ID 1a86:55d3 QinHeng Electronics USB Single Serial
```

Now go to your ESPHome container and go to *Resources -> Add -> Device Passthrough*.

![Image](/resources/esphome_device_passthrough.png)

On the dialog, type the info from your device where "dev" is the number of the device Bus (001), and "xyz" is the number of the device itself (002).

![Image](/resources/esphome_device_path.png)

You should now see something like this:

![Image](/resources/esphome_device_summary.png)

If at this point you still don't have USB passthrough to your ESPHome Host, there is something else going on. Maybe a bad device, or a bad port.

I won't cover further passthrough troubleshooting here.

## For the love of all that's sacred, I cannot make this work using the `esp-idf` esp32 framework type.

This one is a pain.

First, make sure that your ESPHome hosts has at least 8GB of disk space. I found the hard way that 4GB is simply not enough to download and compile all the platformio packages needed for this framework type.

If your ESPHome doesn't have the minimum 8GB, and you are using PVE, you can add the extra disk as follows:

Follow the same instructions above to login into your PVE environment, and click on your ESPHome LXC.

Go to *Resources -> Root Disk*.

![Image](/resources/pve_root_disk.png)

thenk Click on *Volume Action -> Resize*.

![Image](/resources/pve_volume_resize.png)

Finally type in the amount of disk you want to **add**. So if you already had 4GB, you should type in "4" to get to the 8GB you want.

![Image](/resources/pve_resize_dialog.png)

Reboot your LXC for good hygiene and you'll be ready for the next step.

Now go to your ESPHome LXC console, go to your platformio directory, and ruthlessly erase all of its contents with `rm -rf *`. Just be careful - this is quite a dangerous command to execute on a wrong directory.

The platformio directory is hidden so, unless you are using `ls -la` on your root directory, you won't see it.

```
cd ~/.platformio
rm -rf *
```

This will force the ESPHome Host to start off with a clean slate - it will redownload and recompilate all of its platformio packages, this time with the right amount of space needed to succeed.

## I'm getting an ESP_ERR_NVS_NOT_ENOUGH_SPACE error!

After multiple uploads, your device's Non-Volatile Storage might need some housekeeping.

If you ever get this NVS error during the compilation and programming of your ESPHome device, then connect directly to your ESPHome Host and use the following commands.

```
dd if=/dev/zero of=nvs_zero bs=1 count=20480
esptool.py --chip esp32 --port /dev/ttyACM0 write_flash 0x009000 nvs_zero
```

The first command writes a file of null bytes (the typical size of your NVS) on the current directory, naming it nvs_zero.

The second command uses your USB passthrough connection to write the file on your ESPHome device at 0x009000, which is the typical address for the NVS file.

You'll need to replace `/dev/ttyACM0` with your own USB port.



# Bonus

## Presentation

There are many ideas around how to present your screen. From handmade stands, to Ikea frames and everything in between.

In my mind, this project would benefit from a stand like this:

<img src="/resources/stand.png" width=70% height=70%>

https://www.hackster.io/lmarzen/esp32-e-paper-weather-display-a2f444

What I'd do differently though, is the USB-C connector embedded in the back of base.

Instead, I would either place the connector facing downwards (like the power plug of a Sonos One speaker) or I'd offset it into the base at the bottom (which would end up looking almost the same as the first picture below).

If you haven't seen something like that, it looks super clean!

FYI: These pictures are here just to illustrate the kind of look I'd go after. They aren't mine.

![Image](/resources/sonos_example1.jpg)

<img src="/resources/sonos_example2.jpg" width=50% height=50%>

![Image](/resources/sonos_example3.jpg)

## TODO

As this is the first half-decent version of this project, I still have a lot of work to do.

Things I'm thinking of:

- Implement the rolled back custom component for the UI integration

    - That would get rid of almost all the To Do's below.

- Automate deleting the QR snapshot from HA.

- Clean up the code.

    - There is more flexibility than it is needed, so the code can be difficult to follow.

    - Lots of unused variables, or poorly defined ones (looking at you canvas_struct.h!).
      
- Implement the pool temperature logic

    - I haven't constructed it so I don't have the sensor!

- Expand this document ot include a Home Assistant Card to manage/monitor meaningful variables.

- Rework the prevent_deep_sleep logic.

    - I've been testing it a lot and I don't think it is working like it should.

- Stream-line the ESP messages.

    - Talking about a mess... There is no consistency whatsoever, and many messages are repeated.

- Fully test the **no-unifi** version.

    - That needs much more attention than what I've given
 
- Change the whole approach to use the existing (but removed) Unifi integration component!



# References

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
- https://www.reddit.com/r/Esphome/comments/1iwroxi/esphome_github_and_licensing/

Among many others!
