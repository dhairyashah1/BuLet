## Installing Debian on the BBB ##

Use this version of Debian, others might not work:

https://debian.beagleboard.org/images/bone-debian-7.5-2014-05-14-2gb.img.xz

There may be some additional configuration necessary (disabling eMMC &
HDMI), but I'm not sure whether it's strictly necessary.


## Compiling the Device Tree Overlay ##

The device tree overlay activates the PRU and configures P8.11 as output
and P8.16 as input pins. You first need to compile the device tree
source into a binary blob:

    $ sudo dtc -O dtb -I dts -o /lib/firmware/PRU-GPIO-EXAMPLE-00A0.dtbo -b 0 -@ PRU-GPIO-EXAMPLE-00A0.dts

The blob then must be loaded:

    $ sudo -s
    # echo PRU-GPIO-EXAMPLE-00A0 > /sys/devices/bone_capemgr.?/slots                  #(This is old)
    # echo PRU-GPIO-EXAMPLE > /sys/devices/platform/bone_capemgr/slots           #(This is new)

Make sure that loading succeeded by looking at the output of dmesg

    # dmesg | tail
    [  636.923746] bone-capemgr bone_capemgr.9: part_number 'PRU-GPIO-EXAMPLE', version 'N/A'
    [  636.923926] bone-capemgr bone_capemgr.9: slot #7: generic override
    [  636.923972] bone-capemgr bone_capemgr.9: bone: Using override eeprom data at slot 7
    [  636.924021] bone-capemgr bone_capemgr.9: slot #7: 'Override Board Name,00A0,Override Manuf,PRU-GPIO-EXAMPLE'
    [  636.924290] bone-capemgr bone_capemgr.9: slot #7: Requesting part number/version based 'PRU-GPIO-EXAMPLE-00A0.dtbo
    [  636.924337] bone-capemgr bone_capemgr.9: slot #7: Requesting firmware 'PRU-GPIO-EXAMPLE-00A0.dtbo' for board-name 'Override Board Name',  version '00A0'
    [  637.020116] bone-capemgr bone_capemgr.9: slot #7: dtbo 'PRU-GPIO-EXAMPLE-00A0.dtbo' loaded; converting to live tree
    [  637.020570] bone-capemgr bone_capemgr.9: slot #7: #2 overlays
    [  637.040291] omap_hwmod: pruss: failed to hardreset
    [  637.067891] bone-capemgr bone_capemgr.9: slot #7: Applied #2 overlays.


## fsk ##

The fsk.[cp] programs use the PRU to toggle the GPIO pin P8.11 at
compile-time defined frequencies.

Modify fsk.c to write tuples of (ntoggles, delay) into the PRU's RAM.
Example:

    pruMem8[0] = 5;
    pruMem8[1] = DELAY(80);
    pruMem8[2] = 10;
    pruMem8[3] = DELAY(160);
    pruMem8[4] = 0; // Last byte must always be 0.

This will first toggle the GPIO five times with a delay of 80 ns in
between, and then toggle the GPIO ten times with a delay of 160 ns in
between.

The delay must be a multiple of 10 an must be at least 80 ns and at
most 2580 ns. `ntoggle` must be between 1 and 255.

To run:

    $ make fsk
    $ sudo ./fsk


## Useful resources ##

* http://credentiality2.blogspot.se/2015/09/beaglebone-pru-gpio-example.html
* http://mythopoeic.org/bbb-pru-minimal/
* https://github.com/beagleboard/am335x_pru_package
