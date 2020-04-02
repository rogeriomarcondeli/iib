/*
 * spi.h
 *
 *  Created on: 16 de ago de 2017
 *      Author: allef.silva
 */

#ifndef DRIVERS_PERIPHERAL_DRIVERS_SPI_SPI_H_
#define DRIVERS_PERIPHERAL_DRIVERS_SPI_SPI_H_

extern void spi_init();
extern uint32_t read_spi_byte(uint8_t reg);
extern void read_spi_data(uint8_t reg, uint32_t *data, uint8_t size);
extern void write_spi_byte(uint8_t reg, uint32_t data);

#endif /* DRIVERS_PERIPHERAL_DRIVERS_SPI_SPI_H_ */
