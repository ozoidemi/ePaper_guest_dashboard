# Multi-language Guest Dashboard for ESPHome

<img src="/resources/overview.jpg" width=80% height=80%>

---

## Before You Proceed

**This project displays your guest network's Wi-Fi password on an e-paper screen. That is its entire purpose.**

A few things to understand upfront:

1. **The Unifi WiFi password is never exposed through Home Assistant sensors**

Credentials live in the ESP device's RAM and in your UniFi controller only. HA is the scheduler and weather source — nothing more. In the no-UniFi variant, the guest password lives in HA's `secrets.yaml` and flows through HA template sensors. See [No UniFi?](#no-unifi) for details.

2. **Your guest network must be isolated.**

This is a baseline requirement, not a suggestion. Guests on a flat network can reach your other devices. Ubiquiti's [best practices guide](https://help.ui.com/hc/en-us/articles/23948850278295-Best-Practices-Guest-WiFi) is a good starting point.

3. **Physical access = credential access.**

Anyone who can see the screen can read the password.


If you're comfortable with those three points and have a properly isolated guest network, read on.
---

## TL;DR:

### You'll need

- UniFi controller (UDM / UDM Pro / Cloud Key Gen2+) on your LAN
- Home Assistant instance
- ESPHome host (add-on or standalone)
- Waveshare 7.5" e-paper display (800×480, model 7.50inv2)
- Waveshare ESP32 Driver Board

### Steps

**UniFi**
- [ ] Set up and isolate your guest network
- [ ] Create an API key: *UniFi OS portal → Settings → Admins & Users → API Keys*

**Terminal (Powershell in Windows)**
- [ ] Find your guest WLAN's `_id` (command found in [Step 1](#step-1--unifi-setup))

**Home Assistant**
- [ ] Merge `configuration.yaml` contents into your HA configuration
- [ ] Copy `template.yaml` to `/homeassistant/template.yaml`
- [ ] Add the `automations.yaml` entry to your HA automations

**ESPHome Host**
- [ ] Install dependencies:
  ```bash
  pip install cairosvg
  apt install libcairo2-dev
  ```
- [ ] Copy `canvas_struct.h` into `external_components/` inside your ESPHome config directory

**ESPHome Device**
- [ ] Populate `secrets.yaml` with the entries listed below
- [ ] Flash `epaper-guest-dashboard.yaml` to your device

**Required `secrets.yaml` entries you need to create/update:**
```yaml
wifi_ssid: "YourMainNetworkSSID"
wifi_password: "YourMainNetworkPassword"
wifi_ssid_fallback_01: "epaper-fallback"
wifi_password_fallback: "your-fallback-password"
homeassistant_api_encryption_key: "your-ha-api-key"
ota_update_password: "your-ota-password"
unifi_api_key: "your-unifi-api-key"
unifi_controller_url: "https://192.168.1.1"
unifi_wlan_id: "your-guest-wlan-id"
guest_ssid: "YourGuestNetworkName"
```

**Three things worth noting:**
1. `cairosvg` is required because MDI icons are compiled from SVG at build time — the device itself doesn't need it
2. `canvas_struct.h` goes in `external_components/`
3. `template: !include template.yaml` **must be the last key** in `configuration.yaml` — this is a Home Assistant YAML parser requirement; anything after it is silently ignored

---

## No UniFi?

Use the files under `no_unifi/` instead. The display shows the same screen, but the credential pipeline is different: **HA holds the guest password**, not the ESP device.

### How it works

The guest SSID and password live in **HA's** `secrets.yaml` (`/homeassistant/secrets.yaml`). HA exposes them as template sensors, builds the QR string from them, and the ESP reads everything from HA over WebSocket — the same way it reads weather and time.

### Setup

**Home Assistant (`/homeassistant/secrets.yaml`)** — add your guest network credentials here:
```yaml
wifi_ssid: "YourGuestNetworkSSID"
wifi_password: "YourGuestNetworkPassword"
```

**HA config** — use `no_unifi/configuration.yaml` in place of the main `configuration.yaml`. Skip `automations.yaml` entirely.

**ESPHome (`secrets.yaml`)** — your management network (what the ESP connects to for HA communication) plus HA API credentials:
```yaml
wifi_ssid: "YourIoTNetworkSSID"
wifi_password: "YourIoTNetworkPassword"
wifi_ssid_fallback: "ePaper-Display-Fallback"
wifi_password_fallback: "your-fallback-ap-password"
homeassistant_api_encryption_key: "your-ha-api-key"
ota_update_password: "your-ota-password"
```

Note that `wifi_ssid`/`wifi_password` appear in **both** secrets files but mean different things: HA's copy is the guest network shown on the display; ESPHome's copy is the management network the device connects to.

**ESPHome device** — flash `no_unifi/epaper_guest_dashboard`.

### Trade-offs vs. the UniFi variant

| | UniFi variant | No-UniFi variant |
|---|---|---|
| Guest password visible to HA | No | Yes — in `secrets.yaml` and template sensors |
| Password rotation | Automatic (scheduled + on-demand button) | Manual — edit HA `secrets.yaml` and restart |
| Requires UniFi controller | Yes | No |

To change the guest password: update `wifi_password` in HA's `secrets.yaml`, then do a full HA restart. The ESP will pick up the new value on its next sensor subscription update.

> This variant has not been fully tested. Treat it as a starting point.

---

# Detailed Setup

## What Does This Thing Actually Do?

An ESP32 e-paper display that shows your guest Wi-Fi credentials and live weather. It updates every 5 minutes and deep-sleeps outside of configured hours.

**The display shows:**
- Wi-Fi QR code (generated on-device — no cloud, no HA)
- Guest SSID and formatted 12-character password separated in 4 groups of 3 characters for easy read
- Current temperature, humidity, UV index
- 3-hour weather forecast with conditions
- Sunset time
- Current time and date (in English, Castellano, Français, Italiano, or Deutsch)

**Home Assistant handles:**
- Weather data via [met.no](https://api.met.no/) — free, no account needed, coordinates pulled automatically from your `zone.home`
- Time and date formatting
- Display language selection
- Deep sleep scheduling
- Password rotation schedule (triggers the ESP on a timer)

**The ESP handles:**
- Talking directly to UniFi at boot to read the current password and network state
- Generating new passwords and pushing them to UniFi on rotation
- QR code generation and display rendering without ever interacting with the UniFi integration

**HA never touches the guest password** (UniFi variant). It only presses a button to tell the ESP to rotate — the ESP does everything else. In the no-UniFi variant, HA holds the credentials and the ESP reads them from HA.

---

## Who Is This For?

People who:
- Entertain guests regularly and are tired of the "what's the WiFi password?" conversation
- Have a properly isolated guest network
- Are comfortable editing YAML files

If you need a one-click install, this will frustrate you. If you're willing to follow a step-by-step guide, you'll have it running in an afternoon.

### A Note on the Security Posture

The HA team specifically chose not to expose guest network passwords as sensors in the UniFi integration. There are good reasons for that decision.

This project takes a different view: the password is already public the moment it's printed on a QR code on your wall — the only meaningful control is physical proximity, which is already required to use your guest network anyway.

You do you. But know what you're signing up for.

---

## Hardware

### The Display and Driver Board

This project uses:

1. [Waveshare 7.5" e-Ink Raw Display](https://www.amazon.com/dp/B075R69T93)
2. [Waveshare Universal e-Paper Raw Panel Driver Board](https://www.amazon.com/dp/B07M5CNP3B)

Both are older models. The current Waveshare catalog has updated versions:
- [Current display](https://www.waveshare.com/product/displays/e-paper/epaper-1/7.5inch-e-paper.htm) — new revision supports 4 grayscale levels; this project uses black and white only
- [Current driver board](https://www.waveshare.com/product/displays/e-paper/driver-boards/e-paper-esp32-driver-board.htm) — the GPIO pinout differs from the original board

The ESPHome config is wired for the **original board**. If you're using the new Rev. 3 board, update the GPIO substitutions at the top of `epaper-guest-dashboard.yaml`.

#### Original Board — [Wiki](https://www.waveshare.com/wiki/E-Paper_ESP32_Driver_Board#Pins)

```
PIN  | ESP32 | Description
---  | ----- | -----------
VCC  | 3V3   | Power (3.3V)
GND  | GND   | Ground
DIN  | P14   | SPI MOSI
SCLK | P13   | SPI CLK
CS   | P15   | Chip select (active low)
DC   | P27   | Data/Command
RST  | P26   | Reset (active low)
BUSY | P25   | Busy output
```

#### New Board (Rev. 3) — [Product Page](https://www.waveshare.com/product/displays/e-paper/driver-boards/e-paper-esp32-driver-board.htm)

```
PIN  | ESP32 | Description
---  | ----- | -----------
VCC  | 3V3   | Power (3.3V)
GND  | GND   | Ground
DIN  | P12   | SPI MOSI
SCK  | P15   | SPI CLK
CS   | P16   | Chip select (active low)
DC   | P11   | Data/Command
RST  | P10   | Reset (active low)
BUSY | P9    | Busy output
```

---

## Step 1 — UniFi Setup

### Create an API Key

The ESP communicates with your UniFi controller directly using an API key — no UniFi HA integration needed.

1. Log into your **UniFi OS portal** (the top-level portal at your controller IP, not the Network app inside it)
2. Go to *Settings → Admins & Users → API Keys*
3. Click *Create API Key*, give it a name, and copy the key — you won't see it again after closing the dialog

### Find Your Guest WLAN ID

The WLAN ID (`_id`) is a unique internal identifier for your guest network. The ESP uses it to make sure it's only ever touching the right network — regardless of what the SSID is called.

From a terminal (Powershell in Windows), execute the following command:

```
curl.exe -sk -H "X-API-KEY: [your-ha-api-key]" ` https://[unifi_controller_url]/proxy/network/api/s/default/list/wlanconf |  ConvertFrom-Json | Select-Object -ExpandProperty data | Where-Object { $_.name -eq "[YourGuestNetworkSSID]" } | Select-Object _id, name, x_passphrase, enabled
```

---

## Step 2 — Home Assistant Setup

You'll need the **File Editor** add-on to edit config files from the browser. If you don't have it: *Settings → Add-ons → Add-on Store → File Editor → Install*.

### configuration.yaml

Open `/homeassistant/configuration.yaml` in the File Editor.

Add the contents of the project's `configuration.yaml` to your existing file. If you already have sections like `input_select:` or `input_boolean:`, append the new entries inside the existing sections — don't create duplicate top-level keys.

What this adds:
- **`rest:`** block — pulls weather data from met.no every 30 minutes. No API key or account needed. Coordinates are read automatically from your `zone.home` entity.
- **`input_select:`** — language selector (English, Castellano, Français, Italiano, Deutsch)
- **`input_boolean:`** — three display control flags (border debug, grid debug, prevent deep sleep)
- **`binary_sensor:`** — a time-of-day sensor that activates deep sleep between 9:58 PM and 7:58 AM
- **`template: !include template.yaml`** — must be the **last line** in the file

> **Why must `template:` be last?** Home Assistant's YAML loader treats `!include` as a stream continuation — the included file is parsed as part of the same document. Any keys after it end up outside the parse stream and are silently dropped. Put it last and this is never an issue.

### template.yaml

Copy the project's `template.yaml` to `/homeassistant/template.yaml`.

This file defines the formatted time, date, sunset time, sleep message, and multilingual day/month names that the display renders. It references `sun.sun` (universally available in HA) and the `guest_display_language` input select.

### automations.yaml

Open `/homeassistant/automations.yaml` and add the single entry from the project's `automations.yaml`.

This fires every Thursday at 8:55 PM and presses the ESP's rotate button — the ESP then generates a new password and pushes it to UniFi. Adjust the schedule to your liking, or remove it entirely if you prefer to rotate manually.

### Restart and Finish

Do a **full HA restart** after saving all files (*Settings → System → Restart*). A config reload is not enough for the `rest:` sensors to register.

After restarting, go to *Settings → Devices & Services → Entities*, search for `guest_display_deep_sleep_flag`, open it, click the gear icon, and set the icon to `mdi:sleep`. This is the one thing the YAML config can't do for you.

### Stale Entities

If you previously had a different weather setup (HA weather integration, template-based forecast sensors, etc.), you may have stale entities in your registry with names like `current_temperature` or `forecast_condition_1h`. These can silently conflict with the new REST sensors.

Check *Settings → Devices & Services → Entities* and search for each weather sensor name. If you see two entries with the same name but different integrations (e.g., one `template` and one `rest`), delete the stale one if you can. Restart once more and the REST sensors will claim the correct entity IDs.

---

## Step 3 — ESPHome Host Setup

### Install Dependencies

MDI icons are rendered from SVG files at compile time. You need two packages installed on your ESPHome host:

```bash
pip install cairosvg
apt install libcairo2-dev
```

If you're running ESPHome as a Home Assistant add-on, open a terminal (*Settings → Add-ons → Advanced SSH & Web Terminal*) and run these there.

You only need to do this once. Subsequent device updates don't require reinstalling.

### Add canvas_struct.h

`canvas_struct.h` defines the layout structure for the display grid. It needs to be accessible to the ESPHome compiler.

1. Navigate to your ESPHome config directory (typically `/config/esphome/` on the add-on)
2. Create the `external_components` subdirectory if it doesn't exist:
   ```bash
   mkdir -p external_components
   ```
3. Create the file and paste in the contents of `canvas_struct.h`:
   ```bash
   nano external_components/canvas_struct.h
   ```
   Save with `Ctrl+S`, exit with `Ctrl+X`.
4. Verify:
   ```bash
   cat external_components/canvas_struct.h
   ```

---

## Step 4 — ESPHome Device Setup

### Populate secrets.yaml

In your ESPHome config directory, open or create `secrets.yaml` and add:

```yaml
# Main network — this is what the ESP connects to for HA communication
# This is NOT your guest network
wifi_ssid: "YourIoTNetworkSSID"
wifi_password: "YourIoTNetworkPassword"

# Fallback hotspot — used if the ESP can't reach your main network
wifi_ssid_fallback: "epaper-fallback"
wifi_password_fallback: "your-fallback-password"

# Home Assistant API
homeassistant_api_encryption_key: "your-ha-api-key"

# OTA updates
ota_update_password: "your-ota-password"

# UniFi — from Step 1
unifi_api_key: "your-unifi-api-key"
unifi_controller_url: "https://192.168.1.1"
unifi_wlan_id: "your-24-char-wlan-id"
guest_ssid: "YourGuestNetworkName"
```

The `wifi_ssid` and `wifi_password` here are for your **IoT network** — the network the ESP uses to talk to HA and UniFi. These are completely separate from the guest credentials the display shows.

### Flash the Device

1. Open the ESPHome dashboard in your browser
2. Create a new device (or open an existing one)
3. Paste the contents of `epaper-guest-dashboard.yaml` into the YAML editor
4. Optionally update `name` and `friendly_name` at the top of the file
5. Click Install

Updates can be done over-the-air.

### What to Expect on First Boot

On first connection to Home Assistant, the ESP:
1. Calls the UniFi API to read the current password and network state
2. Renders a blank screen
3. Waits 60 seconds and refreshes the screen with all the correct data
4. Then the screen refreshes every 5 minutes

---

# Troubleshooting

## Sensors exist in HA but show unavailable

You likely have stale entities from a previous configuration conflicting with the new REST sensors. See [Stale Entities](#stale-entities) above.

## ESP shows blank forecast or zero temperatures on first boot

The weather values are pushed from HA to the ESP via WebSocket subscription. If the display renders before HA has pushed all sensor states, the canvas arrays hold their defaults (empty string, 0.0). Trigger a manual display update or wait for the next 5-minute refresh cycle.

## Flashing the ESP over USB (Proxmox LXC)

### Privileged Container and Disk Size

When installing the ESPHome LXC via the community helper scripts, choose Advanced settings and select:
- Container type: **Privileged** (required for USB passthrough)
- Disk size: **8 GB minimum** (4 GB is not enough for compile artifacts)

![Advanced settings](/resources/pve_advanced.png)
![Disk size](/resources/pve_disk_size.png)
![Privileged container](/resources/pve_privileged.png)

### Adding USB Passthrough

From the Proxmox shell, identify your device:
```bash
lsusb
```
```
Bus 001 Device 002: ID 1a86:55d3 QinHeng Electronics USB Single Serial
```

Go to your ESPHome LXC → *Resources → Add → Device Passthrough* and enter the device path from the bus and device numbers above.

![Device passthrough](/resources/esphome_device_passthrough.png)
![Device path](/resources/esphome_device_path.png)
![Device summary](/resources/esphome_device_summary.png)

## esp-idf framework won't compile

This project uses the **arduino** framework. If you've switched to esp-idf for any reason, switch back.

If compilation fails even on arduino, your ESPHome host may be out of disk space. 8 GB minimum is required. If you need to add space on Proxmox:

*ESPHome LXC → Resources → Root Disk → Volume Action → Resize*

![Root disk](/resources/pve_root_disk.png)
![Volume resize](/resources/pve_volume_resize.png)
![Resize dialog](/resources/pve_resize_dialog.png)

After resizing, clear the platformio cache to force a clean rebuild:
```bash
cd ~/.platformio
rm -rf *
```

## NVS storage error

After many flashes, the device's Non-Volatile Storage can fill up. If you see `ESP_ERR_NVS_NOT_ENOUGH_SPACE`:

```bash
dd if=/dev/zero of=nvs_zero bs=1 count=20480
esptool.py --chip esp32 --port /dev/ttyACM0 write_flash 0x009000 nvs_zero
```

Replace `/dev/ttyACM0` with your device's actual port. If `esptool` isn't installed:
```bash
apt upgrade && apt install pip
pip install --upgrade esptool
```

---

# Bonus

## Stand Ideas

There are many ways to present the display. The approach that appeals most to me is a minimal stand with a recessed USB-C connector facing downward — no visible cables, clean finish.

<img src="/resources/stand.png" width=70% height=70%>

[Original by lmarzen](https://www.hackster.io/lmarzen/esp32-e-paper-weather-display-a2f444)

The Sonos-style downward-facing connector does this well:

![](/resources/sonos_example1.jpg)
<img src="/resources/sonos_example2.jpg" width=50% height=50%>
![](/resources/sonos_example3.jpg)

*(Reference images, not mine.)*

---

# References

**Inspiration**
- https://github.com/Madelena/esphome-weatherman-dashboard
- https://github.com/maxmacstn/HA-ePaper-Display
- https://github.com/DeastinY/esphome-waveshare-e-paper-dashboard
- https://github.com/lmarzen/esp32-weather-epd
- https://www.printables.com/model/994770-waveshare-75-e-ink-display-insert-for-ikea-rodalm/files

**Tools**
- https://moqups.com/ — mockup and layout design
- claude.ai — useful for template development

**Problem Solving**
- https://community.home-assistant.io/t/where-are-helpers-stored-when-created-in-the-gui/347556
- https://pocketables.com/2022/01/how-to-format-that-wifi-qr-code-in-plain-text.html
- https://tatham.blog/2021/02/06/esphome-batteries-deep-sleep-and-over-the-air-updates/
- https://community.home-assistant.io/t/astimezone-output-differs-from-server-local-time-zone/850071/2
- https://community.home-assistant.io/t/updated-automating-unifi-wifi-ssid-password-changes-and-qr-code-generation/380616/87
- https://community.home-assistant.io/t/unifi-network-integration-official-thread/486308/79
- https://community.home-assistant.io/t/definitive-guide-to-weather-integrations/736419
- https://www.reddit.com/r/Esphome/comments/1iwroxi/esphome_github_and_licensing/
- https://api.met.no/doc/locationforecast/datamodel
