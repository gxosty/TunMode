# TunMode Android

TunMode is an Android project that routes traffic through a VPN to intercept packets. It supports TCP and UDP protocols and works on a semi-TCP stack, making it suitable for intercepting packets. You can also optionally intercept DNS queries or let the system resolve them. Essentially, TunMode acts as a proxy without editing or blocking any packets, allowing traffic to flow in and out seamlessly.

## Features

- Route traffic through a VPN to intercept packets
- Support for TCP and UDP protocols
- Optional DNS query interception
- Proxy-like functionality with transparent traffic flow

## Notes
* Might not work properly on Android 14 because of some API changes on that version. I am unable to test it.
* Doesn't support IPv6.

## Usage

1. [Download the TunMode APK](https://github.com/gxosty/TunMode/releases/latest)
2. Install the APK on your Android device
3. Open and click button in the center
4. Start intercepting packets and DNS queries as needed

### Prerequisites

- Android Studio
- Android SDK API Level 21 or higher
- Android NDK 25+ (maybe 24 is also ok, didn't test)

### Building the Project

1. Clone the repository
2. Open the project in Android Studio
3. Build and run the project on a connected Android device or emulator

## Contributing

We welcome contributions to TunMode! If you have any ideas, bug fixes, or feature enhancements, feel free to contribute by submitting a pull request. Please refer to the [contribution guidelines](CONTRIBUTING.md) for more information.
