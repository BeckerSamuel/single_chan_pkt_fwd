/*
 * wiringPiSPI.h:
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
 * Slightly changed version!!!
 * Separated file used, to eb able to use the newest wiringPi version without problems.
 */

#ifdef __cplusplus
extern "C" {
#endif

int wiringPiSPIGetFdAdjusted(int spi_nr, int channel);
int wiringPiSPIDataRWAdjusted(int spi_nr, int channel, unsigned char *data, int len);
int wiringPiSPISetupModeAdjusted(int spi_nr, int channel, int speed, int mode);
int wiringPiSPISetupAdjusted(int spi_nr, int channel, int speed);

#ifdef __cplusplus
}
#endif
