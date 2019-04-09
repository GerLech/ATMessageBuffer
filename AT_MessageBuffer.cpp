//Version 0.1
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.

#include <Arduino.h>

#include "AT_MessageBuffer.h"

AT_MessageBuffer::AT_MessageBuffer() {
  _messageBuffer.packets = 0; //clear packetcount
}

void AT_MessageBuffer::setId(int id[6]) {
  for (uint8_t i = 0; i<6; i++) _messageBuffer.id[i] = id[i];
}

void AT_MessageBuffer::setId(String id){
  int imac[6];
  char x;
  sscanf(id.c_str(), "%x:%x:%x:%x:%x:%x%c",  &imac[0], &imac[1], &imac[2], &imac[3], &imac[4], &imac[5], &x );
  for (uint8_t i = 0; i<6; i++) _messageBuffer.id[i]=imac[i];
}

ATMESSAGEHEADER AT_MessageBuffer::getHeader() {
  ATMESSAGEHEADER result;
  for (uint8_t i = 0; i<6; i++) result.id[i] = _messageBuffer.id[i];
  result.devicebits = _messageBuffer.devicebits;
  return result;
}
void AT_MessageBuffer::clear(){
  _messageBuffer.packets = 0;
  _messageBuffer.devicebits = 0;
}

void AT_MessageBuffer::addLong(long value, uint8_t channel, uint8_t unit, uint8_t type) {
  if (_messageBuffer.packets < ATMAXDEVCHANNELS){
    uint8_t index = _messageBuffer.packets;
    _dataPackets[index].channel = channel;
    _dataPackets[index].type = type;
    _dataPackets[index].unit = unit;
    Serial.println(value);
    memcpy(&_dataPackets[index].value[0],&value,4);
    _messageBuffer.packets++;
  }
}


void AT_MessageBuffer::addLongIn(long value, uint8_t channel, uint8_t unit) {
  addLong(value,channel,unit,ATTYPE_DIGITALIN);
}

void AT_MessageBuffer::addLongOut(long value, uint8_t channel, uint8_t unit) {
  addLong(value,channel,unit,ATTYPE_DIGITALOUT);
}

void AT_MessageBuffer::addFloat(float value, uint8_t channel, uint8_t unit, uint8_t type) {
  if (_messageBuffer.packets < ATMAXDEVCHANNELS){
    uint8_t index = _messageBuffer.packets;
    _dataPackets[index].channel = channel;
    _dataPackets[index].type = type;
    _dataPackets[index].unit = unit;
    memcpy(&_dataPackets[index].value[0],&value,4);
    _messageBuffer.packets++;
  }
}

void AT_MessageBuffer::addFloatIn(float value, uint8_t channel, uint8_t unit) {
  addFloat(value, channel, unit, ATTYPE_ANALOGIN);
}

void AT_MessageBuffer::addFloatOut(float value, uint8_t channel, uint8_t unit) {
  addFloat(value, channel, unit, ATTYPE_ANALOGOUT);
}

void AT_MessageBuffer::addSwitch(boolean value, uint8_t channel, uint8_t type){
  if (_messageBuffer.packets < ATMAXDEVCHANNELS){
    uint8_t index = _messageBuffer.packets;
    _dataPackets[index].channel = channel;
    _dataPackets[index].type = type;
    _dataPackets[index].unit = ATUNIT_NONE;
    _dataPackets[index].value[0] = (value)?1:0;
    _messageBuffer.packets++;
  }
}
void AT_MessageBuffer::addSwitchIn(boolean value, uint8_t channel){
  addSwitch(value,channel,ATTYPE_SWITCHIN);
}
void AT_MessageBuffer::addSwitchOut(boolean value, uint8_t channel){
  addSwitch(value,channel,ATTYPE_SWITCHOUT);
}


void AT_MessageBuffer::addCelsius(float value, uint8_t channel) {
  addFloatIn(value, channel, ATUNIT_CELSIUS);
}

boolean AT_MessageBuffer::fillBuffer(uint8_t * buffer, uint8_t * size){
  uint8_t max_size = * size;
  uint8_t sz = sizeof(_messageBuffer.id) + sizeof(_messageBuffer.packets) + sizeof(_messageBuffer.devicebits);
  *size = sz;
  if (sz <= max_size) memcpy(buffer, & _messageBuffer, sz); else return false;
  uint8_t szp = sizeof(ATDATAPACKET);
  for (uint8_t i = 0; i<_messageBuffer.packets; i++) {
    if ((sz + szp) <= max_size) memcpy(buffer + sz, &_dataPackets[i], szp);
    sz += szp;
  }
  * size = sz;
  return (sz <= max_size);
}

void AT_MessageBuffer::readBuffer(const uint8_t * buffer){
  uint8_t sz = sizeof(_messageBuffer.id) + sizeof(_messageBuffer.packets) + sizeof(_messageBuffer.devicebits);
  memcpy(& _messageBuffer, buffer, sz);
  if (_messageBuffer.packets > ATMAXDEVCHANNELS) _messageBuffer.packets = ATMAXDEVCHANNELS;
  for (uint8_t i = 0; i<_messageBuffer.packets; i++) {
    memcpy(& _dataPackets[i], buffer+sz, sizeof(ATDATAPACKET));
    sz += sizeof(ATDATAPACKET);
  }
}

uint8_t AT_MessageBuffer::getPackets() {
  return _messageBuffer.packets;
}

ATDATAPACKET AT_MessageBuffer::getData(uint8_t index) {
  if (index < _messageBuffer.packets) {
    return _dataPackets[index];
  }
  return _dataPackets[0];
}

uint16_t AT_MessageBuffer::getDeviceBits(){
  return _messageBuffer.devicebits;
}

float AT_GetFloat(uint8_t * data) {
  float value;
  memcpy(&value,data,4);
  return value;
}

long AT_GetLong(uint8_t * data){
  long value;
  memcpy(&value,data,4);
  return value;
}

uint32_t AT_CalculateCRC32(const uint8_t *data, size_t length) {
  uint32_t crc = 0xffffffff;
  while (length--) {
    uint8_t c = *data++;
    for (uint32_t i = 0x80; i > 0; i >>= 1) {
      bool bit = crc & 0x80000000;
      if (c & i) {
        bit = !bit;
      }
      crc <<= 1;
      if (bit) {
        crc ^= 0x04c11db7;
      }
    }
  }
  return crc;
}

String AT_MessageBuffer::getId(){
  String stid;
  char tmp[4];
  sprintf(tmp,"%02x",_messageBuffer.id[0]);
  stid=tmp;
  for (uint8_t j = 1; j<6; j++) {
    sprintf(tmp,":%02x",_messageBuffer.id[j]);
    stid = stid += tmp ;
  }
  return stid;
}

String AT_getUnitString(uint8_t unit) {
  String result = "";
  switch (unit) {
    case ATUNIT_LUX: result = "lx"; break;
    case ATUNIT_CELSIUS: result = "°C"; break;
    case ATUNIT_METER: result = "m"; break;
    case ATUNIT_PASCAL: result = "Pa"; break;
    case ATUNIT_PERCENT: result = "%"; break;
    case ATUNIT_FAHRENHEIT: result = "°F"; break;
  }
  return result;
}
