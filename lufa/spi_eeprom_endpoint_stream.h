#ifndef _SPI_EEPROM_ENDPOINT_STREAM_H_
#define _SPI_EEPROM_ENDPOINT_STREAM_H_

uint8_t Endpoint_Read_Control_SpiMemStream_LE(void* const Buffer, uint16_t Length) ATTR_NON_NULL_PTR_ARG(1);
uint8_t Endpoint_Write_Control_SpiMemStream_LE(const void* const Buffer, uint16_t Length) ATTR_NON_NULL_PTR_ARG(1);

#endif //_USB_VENDOR_INTERFACE_H_
