#!/bin/bash
# Build si_ts_test (EABI, same compiler flags as Mvapp/build_eabi.sh): the SI port with a mock
# CSDEMUX that replays a TS file. Output: Mvapp/out-eabi/si_ts_test
#
# Usage: Mvapp/system/merih_eabi/test/build_si_test.sh [install_dir]
set -eu

here=$(cd "$(dirname "$0")" && pwd)
root=$(cd "$here/../../../.." && pwd)
img="${CHIPBOX_EABI_IMAGE:-chipbox-mainline-kernel-builder:bookworm}"
install_dir="${1:-}"

docker run --rm -u "$(id -u):$(id -g)" -v "$root:/w" -w /w "$img" bash -c '
set -e
inc="-I/w/csapi/include -I/w/open_sources/out-eabi/inc"
while IFS= read -r d; do inc="$inc -I$d"; done < <(find /w/Mvapp /w/mvapi -type d -name include -not -path "*/out-eabi/*")
cflags="-O2 -g -march=armv5te -mstructure-size-boundary=32 -w -fcommon -fgnu89-inline -D_LINUX_"
mkdir -p /w/Mvapp/out-eabi
arm-linux-gnueabi-gcc $cflags $inc -o /w/Mvapp/out-eabi/si_ts_test \
  /w/Mvapp/system/merih_eabi/test/si_ts_test.c /w/Mvapp/system/merih_eabi/test/mock_csdemux.c \
  /w/Mvapp/system/merih_eabi/si_table.c /w/Mvapp/system/merih_eabi/si_tabledrv.c \
  /w/Mvapp/mvmid/src/mvos.c /w/Mvapp/mvmid/src/mvutil.c /w/Mvapp/system/limit/crc/crc.c \
  /w/Mvapp/system/merih_eabi/test/si_test_glue.c \
  -lpthread -lrt -Wl,--dynamic-linker=/lib/ld-linux.so.3
ls -l /w/Mvapp/out-eabi/si_ts_test
'
if [ -n "$install_dir" ]; then
  docker run --rm -v "$root:/w" -v "$install_dir:/d" "$img" cp /w/Mvapp/out-eabi/si_ts_test /d/
fi
