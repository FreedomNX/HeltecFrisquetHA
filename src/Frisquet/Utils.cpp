#include "Utils.h"

String byteArrayToHexString(uint8_t *byteArray, int length) {
  String result = "";
  for (int i = 0; i < length; i++)
  {
    char hex[3];
    sprintf(hex, "%02X", byteArray[i]);
    if(i > 0) {
        result += " ";
    }
    result += hex;
  }
  return result;
}