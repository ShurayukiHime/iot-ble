# Bluetooth Low Energy, *A study of...*
We will consider the 5.0 specification, as, according to the official website, version 4.0 is Legacy.

### BLE in short
| | Bluetooth Low Energy (LE) | Bluetooth Basic Rate/ Enhanced Data Rate (BR/EDR)
--- | --- | ---
Optimized For...| Short burst data transmission | Continuous data streaming
Frequency Band  | 2.4GHz ISM Band (2.402 – 2.480 GHz Utilized) | 2.4GHz ISM Band (2.402 – 2.480 GHz Utilized)
Channels        | 40 channels with 2 MHz spacing (3 advertising channels/37 data channels) | 79 channels with 1 MHz spacing
Channel Usage |	Frequency-Hopping Spread Spectrum (FHSS) |	Frequency-Hopping Spread Spectrum (FHSS)
Modulation | 	GFSK | 	GFSK, π/4 DQPSK, 8DPSK
Power Consumption | 	~0.01x to 0.5x of reference (depending on use case) | 1 (reference value)
Data Rate |	LE 2M PHY: 2 Mb/s <br> LE 1M PHY: 1 Mb/s <br> LE Coded PHY (S=2): 500 Kb/s <br> LE Coded PHY (S=8): 125 Kb/s |	EDR PHY (8DPSK): 3 Mb/s <br> EDR PHY (π/4 DQPSK): 2 Mb/s <br> BR PHY (GFSK): 1 Mb/s 
Max Tx Power* |	Class 1: 100 mW (+20 dBm) <br> Class 1.5: 10 mW (+10 dbm) <br> Class 2: 2.5 mW (+4 dBm) <br> Class 3: 1 mW (0 dBm) |	Class 1: 100 mW (+20 dBm) <br> Class 2: 2.5 mW (+4 dBm) <br> Class 3: 1 mW (0 dBm)
Network Topologies |	Point-to-Point (including piconet) <br> Broadcast <br> Mesh |	Point-to-Point (including piconet)

Access scheme:
- Frequency Division Multiple Access: use 40 physical channels separated by 2MHz. 3 are used as primary advertising, 37 as secondary advertising and data channels. 
- Time Division Multiple Access: polling scheme in which one device transmits a packet at a predetermined time, the corresponding device responds with a packet after a predetermined interval.

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
| | Layers |
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

No protection against passive eavesdropping in Just Works and Passive Eavesdropping association modes, because of EC Diffie Hellman (not supported by LE Legacy). If the LE device supports Secure Connections, this protection is present.

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