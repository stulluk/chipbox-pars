#!/bin/bash
# Build the CSAPI (libcs*) libraries for ARM EABI (gcc 12 / glibc 2.36, Debian cross)
# inside Docker, plus the read-only smoke test, without touching the old OABI build.
#
# Usage: csapi/build_eabi.sh [install_root]
#   install_root example: /srv/chipbox-nfsroot/v40-nfs
#   Installs libcs*.so + the EABI glibc runtime under <root>/usr/lib/eabi,
#   /lib/ld-linux.so.3 and <root>/usr/work0/app/eabi/csapi_smoke.
set -euo pipefail

csapi="$(cd "$(dirname "$0")" && pwd)"
img="${CHIPBOX_EABI_IMAGE:-chipbox-mainline-kernel-builder:bookworm}"
jobs=$(( $(nproc) * 8 / 10 ))
install_root="${1:-}"
modules="csevt cstvout csi2c cshdmi csgpio csaud csosd csvid cssqc csdemux cssi csspi csplayer_new"
tc="CC='arm-linux-gnueabi-gcc -fPIC' AR=arm-linux-gnueabi-ar LD=arm-linux-gnueabi-ld \
LINK=arm-linux-gnueabi-gcc RANLIB=arm-linux-gnueabi-ranlib STRIP=arm-linux-gnueabi-strip \
NM=arm-linux-gnueabi-nm"
out="${csapi}/out-eabi"

rm -rf "${out}"
mkdir -p "${out}/runtime"

docker run --rm --cpus="${jobs}" -u "$(id -u):$(id -g)" -v "${csapi}:/c" -w /c "${img}" bash -c "
set -e
for m in ${modules}; do
  (cd \$m && make clean > /dev/null 2>&1; make static dynamic ${tc} > /c/out-eabi/build_\$m.log 2>&1) \
    || { echo \"FAIL \$m\"; tail -5 /c/out-eabi/build_\$m.log; exit 1; }
  echo \"OK   \$m\"
done
arm-linux-gnueabi-gcc -O2 -Wall -march=armv5te -Iinclude -Icsi2c/include -Icsgpio/include \
  testtools/csapi_smoke_eabi.c -Llib -lcsi2c -lcsgpio \
  -Wl,--dynamic-linker=/lib/ld-linux.so.3 -Wl,-rpath,/usr/lib/eabi -o out-eabi/csapi_smoke
cd /usr/arm-linux-gnueabi/lib
for f in ld-linux.so.3 libc.so.6 libm.so.6 libpthread.so.0 libdl.so.2 librt.so.1 libgcc_s.so.1; do
  cp -L \$f /c/out-eabi/runtime/
done
"

cp "${csapi}"/lib/libcs*.so "${out}/"
echo "built: $(find "${out}" -maxdepth 1 -name 'libcs*.so' | wc -l) libraries"

if [ -n "${install_root}" ]; then
  sudo mkdir -p "${install_root}/usr/lib/eabi" "${install_root}/usr/work0/app/eabi"
  sudo cp "${out}"/runtime/* "${out}"/libcs*.so "${install_root}/usr/lib/eabi/"
  sudo ln -sfn /usr/lib/eabi/ld-linux.so.3 "${install_root}/lib/ld-linux.so.3"
  sudo install -m 755 "${out}/csapi_smoke" "${install_root}/usr/work0/app/eabi/csapi_smoke"
  echo "installed into ${install_root}"
fi
