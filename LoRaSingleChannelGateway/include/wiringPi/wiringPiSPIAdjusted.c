/*
 * wiringPiSPI.c:
 *	Simplified SPI access routines
 *	Copyright (c) 2012-2015 Gordon Henderson
 ***********************************************************************
 * This file is part of wiringPi:
 *	https://github.com/WiringPi/WiringPi/
 *
 *    wiringPi is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU Lesser General Public License as
 *    published by the Free Software Foundation, either version 3 of the
 *    License, or (at your option) any later version.
 *
 *    wiringPi is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with wiringPi.
 *    If not, see <http://www.gnu.org/licenses/>.
 ***********************************************************************
 */

#include <asm/ioctl.h>
#include <fcntl.h>
#include <linux/spi/spidev.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <string.h>

#include "wiringPiSPIAdjusted.h"

// The SPI bus parameters
//	Variables as they need to be passed as pointers later on

// static const char       *spiDev0Ch0  = "/dev/spidev0.0" ;
// static const char       *spiDev0Ch1  = "/dev/spidev0.1" ;
// static const char       *spiDev1Ch0  = "/dev/spidev1.0" ;
// static const char       *spiDev1Ch1  = "/dev/spidev1.1" ;
// static const char       *spiDev1Ch2  = "/dev/spidev1.2" ;
static const uint8_t spiBPW = 8;
static const uint16_t spiDelay = 0;

static uint32_t spi0Speeds[2];
static int spi0Fds[2];
static uint32_t spi1Speeds[3];
static int spi1Fds[3];

/*
 * wiringPiSPIGetFd:
 *	Return the file-descriptor for the given channel
 *********************************************************************************
 */
/*int wiringPiSPIGetFdAdjusted(int channel) {
    return wiringPiSPIGetFdAdjusted(0, channel);
}*/

int wiringPiSPIGetFdAdjusted(int spi_nr, int channel) {
    switch (spi_nr) {
        case 0:
            if (channel <= 1) {
                return spi0Fds[channel];
            }
            break;
        case 1:
            if (channel <= 2) {
                return spi1Fds[channel];
            }
            break;
    }

    return -1;
}

/*
 * wiringPiSPIDataRW:
 *	Write and Read a block of data over the SPI bus.
 *	Note the data ia being read into the transmit buffer, so will
 *	overwrite it!
 *	This is also a full-duplex operation.
 *********************************************************************************
 */
/*int wiringPiSPIDataRWAdjusted(int channel, unsigned char *data, int len) {
    return wiringPiSPIDataRWAdjusted(0, channel, data, len);
}*/

int wiringPiSPIDataRWAdjusted(int spi_nr, int channel, unsigned char *data, int len) {
    struct spi_ioc_transfer spi;

    // Mentioned in spidev.h but not used in the original kernel documentation
    //	test program )-:

    memset(&spi, 0, sizeof(spi));

    spi.tx_buf = (unsigned long)data;
    spi.rx_buf = (unsigned long)data;
    spi.len = len;
    spi.delay_usecs = spiDelay;
    spi.bits_per_word = spiBPW;

    switch (spi_nr) {
        case 0:
            if (channel <= 1) {
                spi.speed_hz = spi0Speeds[channel];
                return ioctl(spi0Fds[channel], SPI_IOC_MESSAGE(1), &spi);
            }
            break;
        case 1:
            if (channel <= 2) {
                spi.speed_hz = spi1Speeds[channel];
                return ioctl(spi1Fds[channel], SPI_IOC_MESSAGE(1), &spi);
            }
            break;
    }

    return -1;
}

/*
 * wiringPiSPISetupMode:
 *	Open the SPI device, and set it up, with the mode, etc.
 *********************************************************************************
 */
/*int wiringPiSPISetupModeAdjusted(int channel, int speed, int mode) {
    wiringPiSPISetupModeAdjusted(0, channel, speed, mode);
}*/

int wiringPiSPISetupModeAdjusted(int spi_nr, int channel, int speed, int mode) {
    int fd;
    char spiDev[32];

    mode &= 3;  // Mode is 0, 1, 2 or 3

    // Channel can be anything - lets hope for the best
    //  channel &= 1 ;	// Channel is 0 or 1

    snprintf(spiDev, 31, "/dev/spidev%d.%d", spi_nr, channel);
    printf(spiDev);

    if ((fd = open(spiDev, O_RDWR)) < 0) {
        printf("couldn't open spi");
        return -1;
    }

    printf("spi opened");

    switch (spi_nr) {
        case 0:
            if (channel <= 1) {
                spi0Speeds[channel] = speed;
                spi0Fds[channel] = fd;
            }
            break;
        case 1:
            if (channel <= 2) {
                spi1Speeds[channel] = speed;
                spi1Fds[channel] = fd;
            }
            break;
    }

    // Set SPI parameters.

    if (ioctl(fd, SPI_IOC_WR_MODE, &mode) < 0)
        return -2;

    if (ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &spiBPW) < 0)
        return -3;

    if (ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed) < 0)
        return -4;

    return fd;
}

/*
 * wiringPiSPISetup:
 *	Open the SPI device, and set it up, etc. in the default MODE 0
 *********************************************************************************
 */
/*int wiringPiSPISetupAdjusted(int channel, int speed) {
    return wiringPiSPISetupAdjusted(0, channel, speed)
}*/

int wiringPiSPISetupAdjusted(int spi_nr, int channel, int speed) {
    return wiringPiSPISetupModeAdjusted(spi_nr, channel, speed, 0);
}