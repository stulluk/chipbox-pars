#!/bin/bash
# Build an EABI mvapp.elf (gcc 12 / glibc 2.36) with the closed OABI libraries replaced by
# system/eabi_stubs/closed_stubs.c (no CAS/CI/teletext/subtitles/SI scan); tuner: system/merih_eabi.
#
# Needs: csapi/build_eabi.sh and open_sources/config_eabi.sh (zlib jpeg png freetype
# minigui) run first. Output: Mvapp/out-eabi/mvapp_eabi.elf
#
# Usage: Mvapp/build_eabi.sh [install_root]
#   installs <root>/usr/work0/app/mvapp_eabi.elf and the EABI open-source libraries
#   into <root>/usr/lib/eabi.
set -euo pipefail

mv_dir="$(cd "$(dirname "$0")" && pwd)"
top="$(cd "${mv_dir}/.." && pwd)"
img="${CHIPBOX_EABI_IMAGE:-chipbox-mainline-kernel-builder:bookworm}"
oabi_inc="${CHIPBOX_OABI_INC:-/media/WORK/CHIPBOX-REBORN/source/chipbox-pars/open_sources/inc}"
jobs=$(( $(nproc) * 8 / 10 ))
install_root="${1:-}"

docker run --rm --cpus="${jobs}" -u "$(id -u):$(id -g)" -v "${top}:/w" -v "${oabi_inc}:/oinc:ro" \
  "${img}" bash -c "
set -euo pipefail
out=/w/Mvapp/out-eabi
rm -rf \$out; mkdir -p \$out/obj
inc='-I/w/csapi/include -I/w/open_sources/out-eabi/inc -I/oinc -I/oinc/directfb'
inc=\"\$inc -I/oinc/directfb/direct -I/oinc/directfb/fusion -I/w/mvapi/include\"
for d in \$(find /w/Mvapp /w/mvapi -type d -name include -not -path '*/out-eabi/*'); do inc=\"\$inc -I\$d\"; done
for d in \$(find /w/Mvapp -type d -name src -not -path '*/out-eabi/*'); do inc=\"\$inc -I\$d\"; done
# -mstructure-size-boundary=32: OABI rounded every struct to 4 bytes. The channel database
# and settings files are raw struct dumps (MV_stIndex 4 vs 2 bytes, MV_stTPInfo 32 vs 30 ...),
# so mvapp keeps the OABI struct layout to read existing data. Shared libc / MiniGUI / csapi
# structs only gain trailing padding (checked 2026-10-07, notes/eabi/abi_probe).
cflags='-O2 -g -funwind-tables -march=armv5te -mstructure-size-boundary=32 -w -fcommon -fgnu89-inline -D_FILE_OFFSET_BITS=64 -DSUPPORT_CI'
cflags=\"\$cflags -DCS_ARCH_CSM1201 -DARCH_CSM1201 -D_LINUX_\"
srcs=\$(find /w/Mvapp/system/limit /w/Mvapp/system/open /w/Mvapp/system/eabi_stubs /w/Mvapp/system/merih_eabi \
  /w/Mvapp/csmid /w/Mvapp/mvmid /w/Mvapp/middleware /w/Mvapp/app /w/mvapi -name '*.c' \
  | grep -v -E '/test/|/demo/|/dvbt_tuner/' | sort)
compile() {
  c=\$1; o=\$out/obj/\$(echo \${c#/w/} | tr '/' '_').o
  arm-linux-gnueabi-gcc -c \$cflags \$inc \$c -o \$o
}
export -f compile; export out cflags inc
echo \"\$srcs\" | xargs -P ${jobs} -I{} bash -c 'compile {}' 2> \$out/compile_errors.log
echo \"compiled \$(ls \$out/obj | wc -l) objects\"
arm-linux-gnueabi-gcc -o \$out/mvapp_eabi.elf \$out/obj/*.o \
  -L/w/csapi/out-eabi -L/w/open_sources/out-eabi/libs \
  -Wl,--start-group -lcsevt -lcsgpio -lcshdmi -lcsdemux -lcsi2c -lcsosd -lcsvid -lcsaud \
  -lcstvout -lcssqc -lminigui -lttf -ljpeg -lpng12 -lz -lpthread -lrt -lm -Wl,--end-group \
  -Wl,--wrap=pthread_cancel -Wl,--wrap=sem_wait -Wl,--wrap=sem_timedwait -Wl,--dynamic-linker=/lib/ld-linux.so.3 -Wl,-rpath,/usr/lib/eabi \
  2> \$out/link_errors.log
cp \$out/mvapp_eabi.elf \$out/mvapp_eabi.debug.elf; arm-linux-gnueabi-strip \$out/mvapp_eabi.elf
ls -la \$out/mvapp_eabi.elf
"

if [ -n "${install_root}" ]; then
  libs="${top}/open_sources/out-eabi/libs"
  sudo mkdir -p "${install_root}/usr/lib/eabi"
  for l in libz.so.1 libjpeg.so.62 libpng12.so.0 libfreetype.so.6 libminigui-1.3.so.3 \
           libttf.so.2; do
    sudo cp -L "${libs}/${l}" "${install_root}/usr/lib/eabi/${l}"
  done
  sudo install -m 755 "${mv_dir}/out-eabi/mvapp_eabi.elf" \
    "${install_root}/usr/work0/app/mvapp_eabi.elf"
  echo "installed into ${install_root}"
fi
