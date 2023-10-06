## Synopsis

Daemon running in a Raspberry Pi to control a boat.

This repository is part of the piboat project: piboat.blogspot.fr (French blog).

You can clone this project from github:
- PiBoat project: https://github.com/matthieutdo/PiBoat
- PiBoat Android remote controller project: https://github.com/matthieutdo/PiBoat-Android-RC

## Installation

1. Download this project in your Raspberry Pi.

2. Configure your Raspberry Pi (using raspi-config):
   * Set host name as "piboat" (Network Options -> N1 Host-name)
   * Configure your Wi-Fi (Network Options -> Wi-Fi)
   * Enable camera (Interfacing options -> P1 Camera)  # XXX deprecated
   * Enable I2C (Interfacing options -> P5 I2C)

3. Install dependencies:

```shell
$ sudo apt-get install gcc make i2c-tools git
```

4. install WiringPi library from the unofficial Mirror/Fork:

```shell
$ git clone https://github.com/WiringPi/WiringPi.git
$ cd WiringPi
$./build
```

4. Generate and install binaries (installation is optional):

```shell
$ make
$ make install
```

5. start piboat using systemd:

```shell
$ systemctl start piboat.service
```

Note: you also can automatically start it at system boot:

```shell
# systemctl enable piboat.service
```

## Tests

1. Start the piboat (server) in your Raspberry Pi (debug mode):

```shell
# piboat -d 8
```

2. Compile the remote controller for PC (Linux or Windows):

```shell
$ cd rctrl/
$ make
```

3. start remote controller and send command.

```shell
$ ./rctrl
```

## License

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
