/*
 * spi_speed.h
 *
 *  Created on: Sep 25, 2022
 *      Author: mateusz
 */

#ifndef INCLUDE_DRIVERS_SPI_SPEED_H_
#define INCLUDE_DRIVERS_SPI_SPEED_H_

#define SPI_CR1_BR_Pos           (3U)
#define SPI_CR1_BR_Msk           (0x7UL << SPI_CR1_BR_Pos)                     /*!< 0x00000038 */
#define SPI_CR1_BR               SPI_CR1_BR_Msk                                /*!<BR[2:0] bits (Baud Rate Control) */
#define SPI_CR1_BR_0             (0x1UL << SPI_CR1_BR_Pos)                     /*!< 0x00000008 */
#define SPI_CR1_BR_1             (0x2UL << SPI_CR1_BR_Pos)                     /*!< 0x00000010 */
#define SPI_CR1_BR_2             (0x4UL << SPI_CR1_BR_Pos)                     /*!< 0x00000020 */


#define SPI_SPEED_DIV2       0x00000000U                                    /*!< BaudRate control equal to fPCLK/2   */
#define SPI_SPEED_DIV4       (SPI_CR1_BR_0)                                 /*!< BaudRate control equal to fPCLK/4   */
#define SPI_SPEED_DIV8       (SPI_CR1_BR_1)                                 /*!< BaudRate control equal to fPCLK/8   */
#define SPI_SPEED_DIV16      (SPI_CR1_BR_1 | SPI_CR1_BR_0)                  /*!< BaudRate control equal to fPCLK/16  */
#define SPI_SPEED_DIV32      (SPI_CR1_BR_2)                                 /*!< BaudRate control equal to fPCLK/32  */
#define SPI_SPEED_DIV64      (SPI_CR1_BR_2 | SPI_CR1_BR_0)                  /*!< BaudRate control equal to fPCLK/64  */
#define SPI_SPEED_DIV128     (SPI_CR1_BR_2 | SPI_CR1_BR_1)                  /*!< BaudRate control equal to fPCLK/128 */
#define SPI_SPEED_DIV256     (SPI_CR1_BR_2 | SPI_CR1_BR_1 | SPI_CR1_BR_0)   /*!< BaudRate control equal to fPCLK/256 */




#endif /* INCLUDE_DRIVERS_SPI_SPEED_H_ */
