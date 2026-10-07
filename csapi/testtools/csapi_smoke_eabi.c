/*
 * csapi_smoke.c - read-only smoke test for the EABI-built CSAPI (libcs*) libraries.
 *
 * Checks, without changing any hardware state:
 *   1. libcsi2c: CAT6611 HDMI TX (I2C 0x4c) registers 0x00..0x03 (vendor/device ID).
 *   2. libcsi2c: board EEPROM (I2C 0x50, 2-byte subaddress) MAC at 0x114.
 *   3. libcsgpio: direction and level of GPIO2 pin 49 (LNB H/V select).
 *
 * Build (EABI, separate runtime under /usr/lib/eabi):
 *   arm-linux-gnueabi-gcc -I<csapi>/include -I<csapi>/csi2c/include \
 *     -I<csapi>/csgpio/include csapi_smoke.c -L<csapi>/lib -lcsi2c -lcsgpio \
 *     -Wl,--dynamic-linker=/lib/ld-linux.so.3 -Wl,-rpath,/usr/lib/eabi -o csapi_smoke
 */
#include <stdio.h>
#include <string.h>

#include "csapi.h"
#include "csi2c.h"
#include "csgpio.h"

/* Print a byte buffer as hex with a label. */
static void dump(const char *label, const unsigned char *buf, unsigned int len)
{
  unsigned int i;

  printf("%s:", label);
  for (i = 0; i < len; i++)
    printf(" %02x", buf[i]);
  printf("\n");
}

/* Read the CAT6611 ID registers; returns 0 when the expected ID is seen. */
static int check_cat6611(void)
{
  CSI2C_HANDLE h = CSI2C_Open(0x4c);
  unsigned char id[4] = { 0 };
  int ok;

  if (h == NULL) {
    printf("CAT6611: CSI2C_Open failed\n");
    return 1;
  }
  if (CSI2C_Read(h, 0x00, (char *)id, sizeof(id)) != CSAPI_SUCCEED) {
    printf("CAT6611: read failed: %s\n", CSI2C_GetErrString(h));
    CSI2C_Close(h);
    return 1;
  }
  CSI2C_Close(h);
  dump("CAT6611 id", id, sizeof(id));
  ok = (id[1] == 0xca && id[2] == 0x11 && id[3] == 0x16);
  printf("CAT6611: %s\n", ok ? "PASS" : "FAIL");
  return ok ? 0 : 1;
}

/* Read the MAC address from the board EEPROM; returns 0 when it has the Chipbox OUI. */
static int check_eeprom_mac(void)
{
  CSI2C_HANDLE h = CSI2C_Open(0x50);
  CSI2C_Attr attr;
  unsigned char mac[6] = { 0 };
  int ok;

  if (h == NULL) {
    printf("EEPROM: CSI2C_Open failed\n");
    return 1;
  }
  if (CSI2C_GetAttr(h, &attr) == CSAPI_SUCCEED) {
    attr.subaddr_num = 2;
    CSI2C_SetAttr(h, &attr);
  }
  if (CSI2C_Read(h, 0x114, (char *)mac, sizeof(mac)) != CSAPI_SUCCEED) {
    printf("EEPROM: read failed: %s\n", CSI2C_GetErrString(h));
    CSI2C_Close(h);
    return 1;
  }
  CSI2C_Close(h);
  dump("EEPROM MAC", mac, sizeof(mac));
  ok = (mac[0] == 0x02 && mac[1] == 0x03 && mac[2] == 0x04);
  printf("EEPROM: %s\n", ok ? "PASS" : "FAIL");
  return ok ? 0 : 1;
}

/* Query GPIO2 pin 49 direction and level; returns 0 when both calls succeed. */
static int check_gpio(void)
{
  CSGPIO_HANDLE h = CSGPIO2_Open(49);
  CSGPIO_DIRECTION dir;
  unsigned char bit = 0xff;
  int ok;

  if (h == NULL) {
    printf("GPIO2[49]: open failed\n");
    return 1;
  }
  ok = (CSGPIO_GetDirection(h, &dir) == CSAPI_SUCCEED);
  if (ok)
    ok = (CSGPIO_Read(h, &bit) == CSAPI_SUCCEED);
  CSGPIO_Close(h);
  printf("GPIO2[49]: dir=%d level=%d -> %s\n", (int)dir, (int)bit, ok ? "PASS" : "FAIL");
  return ok ? 0 : 1;
}

int main(void)
{
  int fails = 0;

  printf("csapi_smoke (EABI) start\n");
  fails += check_cat6611();
  fails += check_eeprom_mac();
  fails += check_gpio();
  printf("csapi_smoke result: %s (%d failure(s))\n", fails ? "FAIL" : "PASS", fails);
  return fails ? 1 : 0;
}
