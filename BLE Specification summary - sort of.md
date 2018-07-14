# Bluetooth Low Energy, *A study of...*
We will consider the 5.0 specification, as, according to the official website, version 4.0 is Legacy.

### BLE in short
| | Bluetooth Low Energy (LE) | Bluetooth Basic Rate/ Enhanced Data Rate (BR/EDR)
--- | --- | ---
Optimized For...| Short burst data transmission | Continuous data streaming
Frequency Band  | 2.4GHz ISM Band (2.402 – 2.480 GHz Utilized) | 2.4GHz ISM Band (2.402 – 2.480 GHz Utilized)
Channels        | 40 channels with 2 MHz spacing (3 advertising channels/37 data channels) | 79 channels with 1 MHz spacing
Channel Usage |	Frequency-Hopping Spread Spectrum (FHSS) |	Frequency-Hopping Spread Spectrum (FHSS)
Modulation | 	Gaussian Frequency Shift Keying (GFSK) | 	GFSK, π/4 DQPSK, 8DPSK
Power Consumption | 	~0.01x to 0.5x of reference (depending on use case) | 1 (reference value)
Data Rate |	LE 2M PHY: 2 Mb/s <br> LE 1M PHY: 1 Mb/s <br> LE Coded PHY (S=2): 500 Kb/s <br> LE Coded PHY (S=8): 125 Kb/s |	EDR PHY (8DPSK): 3 Mb/s <br> EDR PHY (π/4 DQPSK): 2 Mb/s <br> BR PHY (GFSK): 1 Mb/s 
Max Tx Power* |	Class 1: 100 mW (+20 dBm) <br> Class 1.5: 10 mW (+10 dbm) <br> Class 2: 2.5 mW (+4 dBm) <br> Class 3: 1 mW (0 dBm) |	Class 1: 100 mW (+20 dBm) <br> Class 2: 2.5 mW (+4 dBm) <br> Class 3: 1 mW (0 dBm)
Network Topologies |	Point-to-Point (including piconet) <br> Broadcast <br> Mesh |	Point-to-Point (including piconet)

Access scheme:
- Frequency Division Multiple Access: use 40 physical channels separated by 2MHz. 3 are used as primary advertising, 37 as secondary advertising and data channels. 
- Time Division Multiple Access: polling scheme in which one device transmits a packet at a predetermined time, the corresponding device responds with a packet after a predetermined interval.

GSFK (from WIkipedia):
>Rather than directly modulating the frequency with the digital data symbols, "instantaneously" changing the frequency at the beginning of each symbol period, Gaussian frequency-shift keying (GFSK) filters the data pulses with a Gaussian filter to make the transitions smoother. This filter has the advantage of reducing sideband power, reducing interference with neighboring channels, at the cost of increasing intersymbol interference. It is used by DECT, Bluetooth, Cypress WirelessUSB, Nordic Semiconductor, Texas Instruments LPRF, Z-Wave and Wavenis devices. For basic data rate Bluetooth the minimum deviation is 115 kHz.
>A GFSK modulator differs from a simple frequency-shift keying modulator in that before the baseband waveform (levels −1 and +1) goes into the FSK modulator, it is passed through a Gaussian filter to make the transitions smoother so to limit its spectral width. Gaussian filtering is a standard way for reducing spectral width; it is called "pulse shaping" in this application.
>In ordinary non-filtered FSK, at a jump from −1 to +1 or +1 to −1, the modulated waveform changes rapidly, which introduces large out-of-band spectrum. If we change the pulse going from −1 to +1 as −1, −0.98, −0.93, …, +0.93, +0.98, +1, and we use this smoother pulse to determine the carrier frequency, the out-of-band spectrum will be reduced.

## Some terminology
Term | Definition
--- | ---
Controller | A collective term referring to all of the layers below HCI
HCI | The Host Controller Interface (HCI) provides a command interface to the baseband Controller and link manager and access to hardware status and control registers. This interface provides a uniform method of accessing the Bluetooth baseband capabilities.
L2CAP | Logical Link Control and Adaptation Protocol
Piconet |  A collection of devices occupying a shared physical channel where one of the devices is the Piconet Master and the remaining devices are connected to it.

## Core System Architecture
- Host
- Primary controller
- 0+ Secondary controllers

A minimal implementation for BLE covers the first four lower layers (?) + related protocols; additionally, there have to be also two service-level protocols: the **Security Manager** and the *Attribute Protocol*. The overall profile requirements are specified in the Generic ATTribute profile and Generic Access Profile.

The controller operates the first four lower layers, while the host operates L2CAP and other higher layers. The standard interface is HCI.

### Core Architectural Blocks - Host
- Channel manager
- L2CAP resource manager
- Security manager protocol
	- It is the P2P protocol to generate encryption and identity keys. It operates over a fixed L2CAP channel.
	- It manages the storage of the keys and generates random addresses or resolves addresses to find device identities.
	- It interacts directly with the controller to provide keys during authentication, encryption or pairing procedures.
	- This module is only present in LE systems.
- Attribute protocol
	- Implements the P2P protocol between an attribute server and a client. The client sends commands, requests and confirmations; the server send responses, notifications and indications to the client. The client can read and write attributes on a peer devices with an ATT server.
- AMP manager protocol
- Generic attribute profile
	- Represents represents the functionality of the attribute server and, optionally, the attribute client.
	- Describes the hierarchy of services, characteristics and attributes used in the attribute server. 
	- Provides interfaces for discovering, reading, writing and indicating of 
service characteristics and attributes
	- Is used on LE devices for LE profile service discovery.
- Generic Access Profile 

## Advertisement

Physical channel divided into time units aka *events*. Data is transmitted between LE devices in packets that are positioned in these events. **Types of events**:
- Advertising
- Extended Advertising
- Periodic Advertising
- Connection

![Advertising Events](https://raw.githubusercontent.com/ShurayukiHime/iot-ble/documentation/images/adv-events.png)

- Advertisers: devices that transmit advertising packets on advertising PHY channels
- Scanners: devices that receive such advertising packets but do not have any intention to connect
	- Scanners may answer advertisement on the same channel, and the advertiser may answer back; as a consequence, the next advertisement packet is sent on a new channel
- Initiators: devices that need to connect and thus listen to advertising packets from the target device
	- The advertising packet must be a connection packet; answer may be given on the same advertising channel
	- At the same time, the connection starts and the advertising event ends

Advertising events may be used to implement a unidirectional or broadcast communication.
They may be also used to establish pair-wise bidirectional communications on data channels, or establish broadcast on secondary advertisement channels.

### Header
Thanks to [advlib](https://github.com/reelyactive/advlib).
For reference, the 16-bit header is as follows (reading the hexadecimal string from left to right):

| Bit(s) | Function                      |
|-------:|-------------------------------|
| 15     | RxAdd: 0 = public, 1 = random |
| 14     | TxAdd: 0 = public, 1 = random |
| 13-12  | Reserved for Future Use (RFU) |
| 11-8   | Type (see table below)        |
| 7-6    | Reserved for Future Use (RFU) |
| 5-0    | Payload length in bytes       |

And the advertising packet types are as follows:

| Type | Packet           | Purpose                                |
|-----:|------------------|----------------------------------------|
| 0    | ADV_IND          | Connectable undirected advertising     |
| 1    | ADV_DIRECT_IND   | Connectable directed advertising       |
| 2    | ADV_NONCONN_IND  | Non-connectable undirected advertising |
| 3    | SCAN_REQ         | Scan for more info from advertiser     |
| 4    | SCAN_RSP         | Response to scan request from scanner  |
| 5    | CONNECT_REQ      | Request to connect                     |
| 6    | ADV_DISCOVER_IND | Scannable undirected advertising       |

For example: 4216 (hex) ---> 100001000010110 (binary)
- 15° bit: 0 = public receiver address
- 14° bit: 1 = random sender address
- 13° and 12° bit: 10
- 11° to 8° bit: 0001 = 2 ---> ADV_NONCONN_IND
- 7° and 6°: 00
- 5° to 0°: 100001 = 22 ---> bytes length

### Address
For reference, the 48-bit header is as follows (reading the hexadecimal string from left to right):

| Bit(s) | Address component |
|-------:|-------------------|
| 47-40  | xx:xx:xx:xx:xx:## |
| 39-32  | xx:xx:xx:xx:##:xx |
| 31-24  | xx:xx:xx:##:xx:xx |
| 23-16  | xx:xx:##:xx:xx:xx |
| 15-8   | xx:##:xx:xx:xx:xx |
| 7-0    | ##:xx:xx:xx:xx:xx |

### Data (Generic Access Profile)

For reference, the structure of the data is as follows:

| Byte(s)     | Data component                                        |
|------------:|-------------------------------------------------------|
| 0           | Length of the data in bytes (including type and data) |
| 1           | GAP Data Type (see table below)                       |
| 2 to length | Type-specifc data                                     |

The Generic Access Profile Data Types are listed on the [Bluetooth GAP Assigned Numbers website](https://www.bluetooth.org/en-us/specification/assigned-numbers/generic-access-profile).  The following table lists the Data Types, their names and the section in this document in which they are described.

| Data Type | Data Type Name                       | See advlib section |
|----------:|--------------------------------------|--------------------|
| 0x01      | Flags                                | Flags              |
| 0x02      | Incomplete List of 16-bit UUIDs      | UUID               |
| 0x03      | Complete List of 16-bit UUIDs        | UUID               |
| 0x04      | Incomplete List of 32-bit UUIDs      | UUID               |
| 0x05      | Complete List of 32-bit UUIDs        | UUID               |
| 0x06      | Incomplete List of 128-bit UUIDs     | UUID               |
| 0x07      | Complete List of 128-bit UUIDs       | UUID               |
| 0x08      | Shortened Local Name                 | Local Name         |
| 0x09      | Complete Local Name                  | Local Name         |
| 0x0a      | Tx Power Level                       | Tx Power           |
| 0x0d      | Class of Device                      | Generic Data       |
| 0x0e      | Simple Pairing Hash C-192            | Generic Data       |
| 0x0f      | Simple Pairing Randomizer R-192      | Generic Data       |
| 0x10      | Security Manager TK Value            | Generic Data       |
| 0x11      | Security Manager OOB Flags           | Generic Data       |
| 0x12      | Slave Connection Interval Range      | SCIR               |
| 0x14      | 16-bit Solicitation UUIDs            | Solicitation       |
| 0x15      | 128-bit Solicitation UUIDs           | Solicitation       |
| 0x16      | Service Data 16-bit UUID             | Service Data       |
| 0x17      | Public Target Address                | Generic Data       |
| 0x18      | Random Target Address                | Generic Data       |
| 0x19      | Public Target Address                | Generic Data       |
| 0x1a      | Advertising Interval                 | Generic Data       |
| 0x1b      | LE Bluetooth Device Address          | Generic Data       |
| 0x1c      | LE Bluetooth Role                    | Generic Data       |
| 0x1d      | Simple Pairing Hash C-256            | Generic Data       |
| 0x1e      | Simple Pairing Hash Randomizer C-256 | Generic Data       |
| 0x1f      | 32-bit Solicitation UUIDs            | Solicitation       |
| 0x20      | Service Data 32-bit UUID             | Service Data       |
| 0x21      | Service Data 128-bit UUID            | Service Data       |
| 0x22      | LE Secure Con. Confirmation Value    | Generic Data       |
| 0x23      | LE Secure Connections Random Value   | Generic Data       |
| 0x24      | URI                                  | Generic Data       |
| 0x25      | Indoor Positioning                   | Generic Data       |
| 0x26      | Transport Discovery Data             | Generic Data       |
| 0x27      | LE Supported Features                | Generic Data       |
| 0x28      | Channel Map Update Indication        | Generic Data       |
| 0x29      | PB-ADV                               | Generic Data       |
| 0x2a      | Mesh Message                         | Generic Data       |
| 0x2b      | Mesh Beacon                          | Generic Data       |
| 0x3d      | 3-D Information Data                 | Generic Data       |
| 0xff      | Manufacturer Specific Data           | Mfr. Specific Data |

### Use-case example
We received a packet whose payload is this:
<center>
02 01 06 0b 09 47 41 50 42 75 74 74 6f 6e 00 03 03 00 aa 04 16 00 aa 09
</center>
Let's see how we understand its content following the above tables.

- 02 01 06 
	- first byte 02 --> decimal length 02
	- second byte 01 = type --> see table, match with 0x01 = "**Flags**"
	- 06 --> 00000110 binary. Meaning:
		- 000 reserved
		- 0 = Simultaneous LE and BR/EDR to Same Device Capable (Host): false (0x0)
		- 0 = Simultaneous LE and BR/EDR to Same Device Capable (Controller): false (0x0)
		- 1 = BR/EDR Not Supported: true (0x1)
		- 1 = LE General Discoverable Mode: true (0x1)
		- 0 = LE Limited Discoverable Mode: false (0x0)
- 0b 09 47 41 50 42 75 74 74 6f 6e 00
	- first byte 0b --> decimal length 11
	- second byte 09 = type --> see table, match with 0x09 = "**Complete Local Name**"
	- third to 12th byte --> translate to ASCII and obtain GAPButton
		- I reckon the name is only 9 bytes long and the last byte 00 is "line ending"
- 03 03 00 aa
	- first byte 03 --> decimal length 03
	- second byte 03 = type --> see table, match with 0x03 = "**Complete List of 16-bit UUIDs**"
	-  third and 4th byte 00 aa = two bytes as expected, although to know the meaning we should look somewhere in mbed specs
- 04 16 00 aa 09
	- first byte 04 --> decimal length 04
	- second byte 16 = type --> see table, match with 0x16 = "**Service Data 16-bit UUID**"
	- third and 4th 00 aa = two bytes, although to know the meaning we should look somewhere in mbed specs (it is the UUID as before)
	- 5th byte 09 --> decimal service data 09

#### Final considerations
You don't really know beforehand how to interpret those data. You just know it will be organized in triplets "*length-meaning-content*". The first field is fundamental because it tells you when to stop reading content, and thus when a new triplet starts.

## Connections
- The initiator becomes the *master* and the advertiser becomes the *slave* in the piconet
- Connection events are used to send data packets
	- At the beginning of each event, there is channel hopping
	- The master initiates the beginning of each event and can terminate it at any time
	- The master and the slave alternate sending packets on the same PHY channel.

![Connection Events](https://raw.githubusercontent.com/ShurayukiHime/iot-ble/documentation/images/conn-events.png)

Frequency hopping pattern is determined by a field present in the connection request packet, and it is a pseudo-random choice of the available 37 frequencies in the ISM band.
	- Frequency hopping can be adapted not to interfere with existing non-hopping connections.

## Hierarchy of layers
<center>
|       | Layers |
| ---| --- |
Physical Layer | Physical transport
Physical Layer | Physical channel |
Physical Layer | Physical link |
Logical Layer | Logical transport |
Logical Layer | Logical link |
L2CAP Layer| L2CAP channel |
</center>
- Connections are formed in physical links, which are contained into physical channels.
- Physical channels can contain multiple master and slave devices, and slaves are permitted to have more than one physical link at a time (but only with masters). Role changes are not permitted.
- Physical links are used as a transport for more than one logical links for asynchronous traffic, and there is a resource manager to manage the physical link through logical link multiplexing.
- L2CAP provides a channel-based abstraction to application and services
	- Carries out fragmentation and defragmnetation
	- Takes care of multiplexing and demultiplexing
- On top of L2CAP there are two additional protocols:
	- **Security Manager Protocol** implements security function between devices
	- **Attribute Protocol** provides a method to communicate small amounts of data over a fixed L2CAP channel 

## LE Topology
![Example of Bluetooth LE Topology](https://raw.githubusercontent.com/ShurayukiHime/iot-ble/documentation/images/example-topology.png)

All the details are on the specification, section 4.2. It's useless to copypasta.

## LE Procedures
### Device filtering p.
- Method used by Controllers to reduce the number of requests they receive or which they answer to
- This is not trivial considering that answer is not mandatory and this reduces power consumption.
- Both advertisers and scanners can ignore messages
- It is achieved through a white list of devices - which are actually the ones you're gonna listen / answer to.

### Advertising p.
- It can be used to make broadcast or periodic broadcast (unidirectional communication)
- It can be used as connection initiator (connectable advertising event)
- Advertising can move to secondary advertising channels or to data channels

### Scanning p.
- Used to listen to advertising messages. The s. device can ask for additional data on the (primary) advertising channel, and the advertising device answers with the data on the same channel
- If the advertisement packet is a connectable advertising event, connection can be started; both devices move to another channel and cease their advertising / scanning to enter the connected mode. After that, the ex-scanning device can still be a scanner on another advertisement channels (provided he can manage all of the connections).

### Discovering p.
- To be a discovering device, you have to listen to scannable advertising events.
- Both discovering and discoverable devices can be active in other piconets, as long as they can manage all the things at the same time.

### Connecting p.
The only way to initiate a connection is to have both devices in the required modes:
- Advertising connectable (and if with other active connections, can manage all of them)
- Scanning for connections It has to initiate the procedure with a connection request specifying also the connection channel.

### Other p.
- Periodic advertising procedure
- Periodic advertising synchronization procedure
- Periodic advertising synchronized mode

## Security
The main concepts for Bluetooth security are explained regarding BR / EDR, and refreshed in the LE section, saying "this is the same" or "this is different". We'll follow the same structure of the specification.
### Functionalities 
- Pairing: the process for creating one or more shared secret keys 
- Bonding: the act of storing the keys created during pairing for use in subsequent connections in order to form a trusted device pair
- Device authentication: verification that the two devices have the same keys
- Encryption: message confidentiality
- Message integrity: protects against message forgeries

### Secure Simple Pairing
Would like to simplify the pairing procedure (while maintaining good levels of security. Other goals:
- Protection against passive eavesdropping
- Protection against MITM attacks

#### Passive eavesdropping protection
Requires
- Strong link key - given by the randomness in the generation
	- Legacy pairings (most likely Bluetooth 4.0, TO BE VERIFIED) use 4-digit user-defined or fixed PINs, thus making brute force attacks on the key pretty easy.
	- Now we use EC Diffie Hellman, which is resistant to passive eavesdropping attacks (a bit less against MITM, they say).
- Data encryption

No protection against passive eavesdropping in Just Works and Passkey Entry association modes, because of EC Diffie Hellman (not supported by LE Legacy). If the LE device supports Secure Connections, this protection is present.

#### MITM Prevention 
The Secure simple pairing offers these two methods:
- Numerical comparison
- Passkey entry

If the user uses a 6-digit umber for numerical comparison / passkey entry, there is a 1 / 1000000 chance of a successful attack (they would like).

4-digit key is sufficient for authentication, 6-digit key is FIPS-compliant (?).

### Association Modes
1. Numeric comparison
2. Just works
3. Out of band
4. Passkey entry

The association model is chosen deterministically basing on the IO capabilities of the device.

If the LE device is not a LE Legacy one, association modes provide the same security as in BR / EBR cases.
#### Numeric Comparison
Used when the devices can show numbers (at least 6 digits) and can receive user input (e.g. yes or no), like a phone or a laptop. Devices wishing to pair should show the same number, and the owners have to confirm the match.
- Ensures that the devices wishing to pair are exactly those.
- Provides protection against MITM attacks

Note that the PIN is not entered by the user but calculated by a purposefully crafted security algorithm.

#### Just Works
Used when 1+ devices doesn't have a keyboard for user IO and / or cannot display 6 digits. Consequently, no PIN is shown and the user just has to accept the connection.
- Protection against passive eavesdropping, but no protection against MITM

#### Out Of Band
Used when there is a (secure) OOB channel for pairing and security keys exchange. Of course, if such channel is not secure the whole process may be compromised.
- Example: NFC communication

One-way or bidirectional authentication are implemented depending on the implementation of NFC communication on both sides. Devices IDs and cryptographic parameters are exchanged.

#### Passkey Entry
One device has input capabilities, but can't display 6 digits; the other device has output capabilities: the PIN is showed on the second device, and the connection is "confirmed" by entering the same PIN on the first device.
Note that in this case the PIN is created by a specific security algorithm, while in legacy versions was an input from the user.

### LE Security
Remember that the previously explained association modes are present in LE too. 
- Key generation
	- Is performed by the Host on each LE device
	- Possible to use multiple keys depending on the purpose
		- Data confidentiality and device authentication
		- Authentication of cleartext
		- Device identity
- Encryption: AES/CCM algorithm. Performed in the controller (?)
- Signed data: data is signed with a Connection Signature Resolving Key. In particular, a Message Authentication Code + a counter are added after the message PDU; the MAC is verified by the recipient.

#### About Privacy
- To avoid tracking (i.e. identifying) a BLE device for too long, address changing techniques may be employed with appropriate frequency.
- The other device needs to know the new address, which is generated from the device's resolving identity key IRK exchanged during pairing.
- Keys may be generated / handled either by the Host or by the Controller.
- In the "device privacy" mode the device will still accept advertising packets from all devices, while in "network privacy" the device will only accept advertising packets from private addresses (?) 

## Application Level Architecture
By defining the required functions and features of each layer, Bluetooth profiles accomplish application interoperability. Application behaviour and data formats are also defined. Service discovery requirements are stated to allow connection, discovery and information gathering at the application level.
### Generic Access Profile
Defines the basic requirements for a Bluetooth device. For LE, defines the Physical layer, Link layer, L2CAP, Security manager, Attribute protocol and Generic attribute profile.

Moreover, in LE four specific roles are defined:
- Broadcaster: optimized for transmission only (broadcast)
- Observer: optimized for reception only. It does not support connections
- Peripheral: optimized for single connections
- Central: supports multiple connections and complex functions / roles.

Each Bluetooth device must implement GAP. Any other profile becomes thus a superset of GAP (hierarchical structure)

### Generic ATTribute Profile
Built on top of ATT, establishes common operations and a framework for transported data. Defines the Server and Client roles.

Specifies the format of data contained in the server; Attributes are formatted as Services and Characteristics. Services may contain a collection of characteristics; characteristics may contain a single value and any number of descriptors, describing the characteristic value.

### GATT-Based Profile Hierarchy

![GATT-based Profile Hierarchy](https://raw.githubusercontent.com/ShurayukiHime/iot-ble/documentation/images/GATT-hierarchy.png)

The GATT Profile specifies the structure in which profile data is exchanged. This structure defines basic elements such as services and characteristics, used in a profile. 

The top level of the hierarchy is a profile. A profile is composed of one or more services necessary to fulfill a use case. A service is composed of characteristics or references to other services. Each characteristic contains a value and may contain optional information about the value. The service and characteristic and the components of the characteristic (i.e., value and descriptors) contain the profile data and are all stored in Attributes on the 
server.

#### Services
A service is a collection of data and associated behaviors to accomplish a particular function or feature of a device or portions of a device. A service may reference other primary or secondary services and/or a set of characteristics that make up the service. There are primary and secondary services.

#### Characteristics
A characteristic is a value used in a service along with properties and configuration information about how the value is accessed and information about how the value is displayed or represented. A characteristic definition contains a characteristic declaration, characteristic properties, and a value. It may also contain descriptors that describe the value or permit configuration of 
the server with respect to the characteristic value.

# Dealing with BLE
Have patience.

## Bash commands

### systemctl bluetooth.service
- systemctl start bluetooth.service
- systemctl restart bluetooth.service
- systemctl enable bluetooth.service

They control the low-level bluetoothd demon, to start, enable and reset it. The effects of these commands may be checked via GUI as well as with the command `service bluetooth status` possibly with option `-l`.

Its error codes are mostly confusing and unfathomable to understand, although if you put some effort in it you could obtain some good achievements.

- If talks about something being blocked, check with `rfkill -l` to see if Bluetooth appears to be soft or hard block. In case, you may as well unblock it with `rfkill`.
- It could say that bluetooth is up and running, while actually it isn't. In this case, check for `time out` error codes. This will be the confirmation that bluetooth is not working properly - the demon has been started but failed to activate.
	- Confirmation can be attained via `hcitool dev` and `hciconfig -a`, which may show either no MAC address, or a completely zeroed address, or an ordinary address - in this case `hciconfig` will say that the device is down.

### hcitool
Simple command to check bluetooth status, make LE scans and connect to bluetooth devices. So simple that most of the times it doesn't work.
- `hcitool dev` returns the device MAC address, if it is working properly.
- `hcitool lescan` performs a LE scan for nearby devices.
- `hcitool leinfo MAC:ADDRESS` returns general information about the selected device.
- `hcitool lecc MAC:ADDRESS` tries to connect to a device.

### hciconfig
Similar to `hcitool` but a bit more useful.

- `hciconfig -a` shows all active devices and their properties. You can obtain some more information with `hciconfig devInt0 commands` and `hciconfig devInt0 features`. It is important to pay attention to two characteristics:
	- If the device is UP RUNNING or DOWN, and in this latter case whether it has been assigned a MAC address (most fortunate case)
	- If you know that your device supports LE, but no information is provided about it, you should know that something is not working. In regular cases, many lines of description are returned when invoking this command, including some about LE features.
- Device can be restarted or enabled via `hciconfig`, but this rarely works if there is a low-level problem with the bluetooth demon. Commands `hciconfig devInt0 reset` and so on.

### hcidump
Even less simple command, still more useful. Keep it in mind if you manage to get your bluetooth working fine. `hcidump` prints dumps of data exchanged between the laptop (or whatever) receiver and the mobile device. They can be either raw hex or pretty printed. They mostly correspond with what you'd find with wireshark.

### gatttool ---incomplete

gatttool -b D5:7A:0D:25:DE:50 -I

### bluetoothctl
It opens an interactive session with dedicated commands to manage the bluetooth agent and interact with nearby devices. The list of commands is quite comprehensive and all in all it may result easier to use than `gatttool` thanks to the added layer of abstraction.

- `bluetoothctl` opens the interactive session. The agent is automatically initialized, but you can anyway ask for its initialization. Nearby devices scanning is not activated by default. 
- `default-agent` asks for the activation of the default agent.
- `devices` tells you the nearby devices that are advertising. A quite irritating feature is that every device is recorded in this list, also from ancient times and also if now it is not currently available, so that this list usually becomes quite long to search through.
- `scan on` activates continuous scanning for nearby devices. It could be advisable to turn it off once you found your target device as it will keep adding new lines to your screen every time any change is received. Devices may take a while to appear here due to random advertisement timings to avoid collisions. Or just if you are unlucky. You can cross-check with the Bluetooth manager GUI or `hcitool lescan`.
- `connect C6:F8:CE:66:73:7E` sends a request for connection. Also in this case, response may take a while to come back. Not all devices allow connection (e.g. if they do only advertisement), and some of them may ask to enter a 6-digit pin to authenticate. See above for details.

## Use case with STM IoT node
Wireshark address filter: `bthci_evt.bd_addr == c6:f8:ce:66:73:7e`

We used the example ble gap button. This programs records the number of hits on the user button and sends non connectable advertising events on the main channel. Thus, the device is visible but it is impossible to connect to, as it is programmed only to perform advertising.
It is possible to read the value written in the data field as it doesn't have any encoding and is publicly advertised. We tested and tried three different approaches:
1) packet sniffing via CMD using `bluetoothctl` (you have to retrieve the hexdump as it automatically tries to display data, making it meaningless)
2) packet sniffing via Wireshark
3) packet sniffing via Android app BLE scanner (the app returns only advertising data, but it matches with wireshark)