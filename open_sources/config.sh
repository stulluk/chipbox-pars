#!/bin/bash

CROSS_COMPILE=arm-linux
CC="$CROSS_COMPILE"-gcc
AR="$CROSS_COMPILE"-ar
AS="$CROSS_COMPILE"-as
CPP="$CROSS_COMPILE"-g++
LD="$CC"
LD1="$CROSS_COMPILE"-ld
RANLIB="$CROSS_COMPILE"-ranlib

WORK_DIR=$PWD
TARGET_INC="$WORK_DIR"/inc
TARGET_LIBS="$WORK_DIR"/libs
TARGET_BIN="$WORK_DIR"/bin
TARGET_SBIN="$WORK_DIR"/sbin
TARGET_ROOTFS="$WORK_DIR"/rootfs

mkdir -p $TARGET_ROOTFS
mkdir -p $TARGET_INC
mkdir -p $TARGET_LIBS

cp target_skeleton/* "$TARGET_ROOTFS" -fa
pushd $TARGET_ROOTFS
find -name .svn | xargs rm -rf
popd
cp $($CC --print-file-name=libc.so.6 | sed -e 's/[-.][a-z.0-9A-Z]*$//')*so*  $TARGET_ROOTFS/lib -fa
cp $($CC --print-file-name=libc-2.3.6.so | sed -e 's/[-.][a-z.0-9A-Z]*$//')*so*  $TARGET_ROOTFS/lib -fa
cp $($CC --print-file-name=ld-2.3.6.so | sed -e 's/[-.][a-z.0-9A-Z]*$//')*so*  $TARGET_ROOTFS/lib -fa
cp $($CC --print-file-name=libm-2.3.6.so | sed -e 's/[-.][a-z.0-9A-Z]*$//')*so*  $TARGET_ROOTFS/lib -fa
cp $($CC --print-file-name=libcrypt-2.3.6.so | sed -e 's/[-.][a-z.0-9A-Z]*$//')*so*  $TARGET_ROOTFS/lib -fa
cp $($CC --print-file-name=libdl-2.3.6.so | sed -e 's/[-.][a-z.0-9A-Z]*$//')*so*  $TARGET_ROOTFS/lib -fa
cp $($CC --print-file-name=libgcc_s.so.1 | sed -e 's/[-.][a-z.0-9A-Z]*$//')*so*  $TARGET_ROOTFS/lib -fa
cp $($CC --print-file-name=libnsl-2.3.6.so | sed -e 's/[-.][a-z.0-9A-Z]*$//')*so*  $TARGET_ROOTFS/lib -fa
cp $($CC --print-file-name=libresolv-2.3.6.so | sed -e 's/[-.][a-z.0-9A-Z]*$//')*so*  $TARGET_ROOTFS/lib -fa
cp $($CC --print-file-name=librt-2.3.6.so | sed -e 's/[-.][a-z.0-9A-Z]*$//')*so*  $TARGET_ROOTFS/lib -fa
cp $($CC --print-file-name=libstdc++.so.6.0.3 | sed -e 's/[-.][a-z.0-9A-Z]*$//')*so*  $TARGET_ROOTFS/lib -fa
cp $($CC --print-file-name=libpthread.so | sed -e 's/[-.][a-z.0-9A-Z]*$//')*so*  $TARGET_ROOTFS/lib -fa
cp $($CC --print-file-name=libthread_db-1.0.so | sed -e 's/[-.][a-z.0-9A-Z]*$//')*so*  $TARGET_ROOTFS/lib -fa
cp $($CC --print-file-name=libutil-2.3.6.so | sed -e 's/[-.][a-z.0-9A-Z]*$//')*so*  $TARGET_ROOTFS/lib -fa

while test $# -ge 1
do
case "$1" in

# Sertac switch to new version 4.Jan.2021
# busybox*)
# echo "---------------------------------------------------------"
# echo " busybox-1.4.1"
# echo "---------------------------------------------------------"
# pushd busybox-1.4.2
# if test ! -f .config; then
# 	cp scripts/defconfig_arm .config
# 	make oldconfig
# fi
# make ARCH=arm CROSS_COMPILE=$CROSS_COMPILE- CONFIG_PREFIX="$TARGET_ROOTFS" install
# popd
# shift;;

busybox*)
echo "---------------------------------------------------------"
echo " busybox-1.20.2"
echo "---------------------------------------------------------"
sleep 2
pushd busybox-1.20.2
if test ! -f .config; then
	cp chipbox-busybox-1.20.2-defconfig .config
	make ARCH=arm oldconfig # I will retry this
fi
make -j16 ARCH=arm CROSS_COMPILE=$CROSS_COMPILE- CONFIG_PREFIX="$TARGET_ROOTFS" install
popd
shift;;

zlib*)
echo "---------------------------------------------------------"
echo " zlib-1.2.3"
echo "---------------------------------------------------------"
sleep 2
pushd zlib-1.2.3
./configure \
	--shared \
	--prefix="$WORK_DIR" \
	--exec-prefix="$TARGET_BIN" \
	--libdir="$TARGET_LIBS" \
	--includedir="$TARGET_INC"
make -j16 CFLAGS=-fPIC CC="$CC" CPP="$CPP" AR="$AR rc" LDSHARED="$LD -shared -Wl,-soname,libz.so.1"
make install
popd
shift;;

bzip2*)
echo "---------------------------------------------------------"
echo " bzip2-1.0.4"
echo "---------------------------------------------------------"
sleep 2
pushd bzip2-1.0.4
make CC=$CC RANLIB=$RANLIB AR=$AR -f Makefile-libbz2_so 
make CC=$CC RANLIB=$RANLIB AR=$AR libbz2.a
cp bzlib.h $TARGET_INC
cp libbz2.so.1.0.4 $TARGET_LIBS
cp libbz2.a $TARGET_LIBS
pushd $TARGET_LIBS
ln -snf libbz2.so.1.0.4 libbz2.so
ln -snf libbz2.so.1.0.4 libbz2.so.1
popd
popd 
shift;;

jpeg*)
echo "---------------------------------------------------------"
echo " jpeg-6b"
echo "---------------------------------------------------------"
sleep 2
pushd jpeg-6b
./configure \
	--target="$CROSS_COMPILE" \
	--host="$CROSS_COMPILE" \
	--prefix=$WORK_DIR \
	--exec-prefix=$WORK_DIR \
	--bindir=$TARGET_BIN \
	--sbindir=$TARGET_SBIN \
	--libdir=$TARGET_LIBS \
	--libexecdir=$TARGET_LIBS \
	--includedir=$TARGET_INC \
	--enable-shared \
	--enable-static \
	--without-x
sleep 2
make -j16 CC=$CC AR=$AR RANLIB=$RANLIB LD=$LD1
make install-lib
popd
shift;;

png*)
echo "---------------------------------------------------------"
echo " libpng-1.2.16"
echo "---------------------------------------------------------"
sleep 2
pushd libpng-1.2.16
CFLAGS=-I$TARGET_INC \
LDFLAGS=-L$TARGET_LIBS \
ac_cv_have_decl_malloc=yes \
gl_cv_func_malloc_0_nonnull=yes \
ac_cv_func_malloc_0_nonnull=yes \
ac_cv_func_calloc_0_nonnull=yes \
ac_cv_func_realloc_0_nonnull=yes \
./configure \
	--target="$CROSS_COMPILE" \
	--host="$CROSS_COMPILE" \
	--prefix=$WORK_DIR \
	--exec-prefix=$WORK_DIR \
	--bindir=$TARGET_BIN \
	--sbindir=$TARGET_SBIN \
	--libdir=$TARGET_LIBS \
	--libexecdir=$TARGET_LIBS \
	--includedir=$TARGET_INC \
	--without-libpng-compat \
	--without-x 
sleep 2
make -j16
make install
popd
shift;;

freetype*)
echo "---------------------------------------------------------"
echo " freetype-2.2.1"
echo "---------------------------------------------------------"
sleep 2
pushd freetype-2.2.1
./configure \
	--target="$CROSS_COMPILE" \
	--host="$CROSS_COMPILE" \
	--prefix=$WORK_DIR \
	--exec-prefix=$WORK_DIR \
	--bindir=$TARGET_BIN \
	--sbindir=$TARGET_SBIN \
	--libdir=$TARGET_LIBS \
	--libexecdir=$TARGET_LIBS \
	--includedir=$TARGET_INC
sleep 2
make -j16 CCexe=gcc
make install
popd
shift;;

directfb*)
echo "---------------------------------------------------------"
echo " DirectFB-1.0.0"
echo "---------------------------------------------------------"
sleep 2
pushd DirectFB-1.0.0
FREETYPE_CFLAGS=-I$TARGET_INC/freetype2 \
FREETYPE_LIBS=-I$TARGET_LIBS \
CFLAGS=-I$TARGET_INC \
LDFLAGS=-L$TARGET_LIBS \
ac_cv_header_linux_wm97xx_h=no \
ac_cv_header_linux_sisfb_h=no \
./autogen.sh \
	--target="$CROSS_COMPILE" \
	--host="$CROSS_COMPILE" \
	--prefix=/ \
	--exec-prefix=/ \
	--bindir=/bin \
	--sbindir=/sbin \
	--libdir=/lib \
	--libexecdir=/lib \
	--sysconfdir=/etc \
	--datadir=/share \
	--localstatedir=/var \
	--includedir=/include \
	--mandir=/man \
	--infodir=/usr/info \
	--with-gfxdrivers=orion \
	--with-inputdrivers=linuxinput \
	--enable-jpeg \
	--enable-png \
	--enable-zlib \
	--disable-sysfs \
	--disable-sdl \
	--disable-video4linux \
	--disable-video4linux2 \
	--enable-freetype
sleep 2
make -j16
make DESTDIR=$WORK_DIR install
cp $WORK_DIR/lib/* $TARGET_LIBS/ -fa
cp $WORK_DIR/include/* $TARGET_INC/ -fa
cp $TARGET_LIBS/directfb-1.0-0 $TARGET_ROOTFS/lib/ -fa
rm -fr ../lib ../include 
popd
shift;;

minigui*)
echo "---------------------------------------------------------"
echo " libminigui-1.3.3"
echo "---------------------------------------------------------"
sleep 2
pushd libminigui-1.3.3
CFLAGS="-I$TARGET_INC -I$TARGET_INC/freetype2 -march=armv5te" \
LDFLAGS=-L$TARGET_LIBS \
./configure \
	--target="$CROSS_COMPILE" \
	--host="$CROSS_COMPILE" \
	--build="x86_64-unknown-linux-gnu" \
	--prefix=$WORK_DIR \
	--exec-prefix=$WORK_DIR \
	--bindir=$TARGET_BIN \
	--sbindir=$TARGET_SBIN \
	--libdir=$TARGET_LIBS \
	--libexecdir=$TARGET_LIBS \
	--includedir=$TARGET_INC \
	--enable-incoreres \
	--enable-jpgsupport \
	--enable-gifsupport \
	--enable-extfullgif \
	--disable-cursor \
	--disable-nativegal \
	--disable-videoqvfb \
	--disable-videoecoslcd \
	--disable-nativeimps2 \
	--disable-nativems \
	--disable-nativems3 \
	--disable-nativegpm \
	--disable-type1support \
	--enable-debug \
	--enable-kbdfrpc \
	--enable-extskin 
sleep 2
cp config_optimized.h config.h
make -j16 CC=$CC
make install
cp src/font/freetype-1.3.1/lib/libttf.so $TARGET_LIBS
mkdir -p $TARGET_ROOTFS/usr/local/etc
mkdir -p $TARGET_ROOTFS/usr/local/lib/minigui
cp etc/MiniGUI.cfg $TARGET_ROOTFS/usr/local/etc -fa
cp res $TARGET_ROOTFS/usr/local/lib/minigui -fa 
find $TARGET_ROOTFS/usr/local/lib/minigui -name ".svn"|xargs rm -fr
cp src/font/freetype-1.3.1/lib/libttf.so $TARGET_ROOTFS/lib -fa
popd
shift;;

mtdutils*)
echo "---------------------------------------------------------" 
echo "  mtd-utils-1.0.0"
echo "---------------------------------------------------------"
sleep 2
pushd  mtd-utils-1.0.0
make -j16 CC=$CC RANLIB=$RANLIB AR=$AR SBINDIR=$TARGET_ROOTFS/usr/bin INCLUDEDIR=$TARGET_INC LIBDIR=$TARGET_LIBS 
make CC=$CC RANLIB=$RANLIB AR=$AR SBINDIR=$TARGET_ROOTFS/usr/bin INCLUDEDIR=$TARGET_INC LIBDIR=$TARGET_LIBS  install
popd 
shift;;

hdparm*)
echo "---------------------------------------------------------" 
echo "  hdparm-7.7 "
echo "---------------------------------------------------------"
sleep 2
pushd hdparm-7.7
make -j16 CC=$CC
cp hdparm $TARGET_ROOTFS/usr/sbin -fa
popd 
shift;;

esac
done

rm -fr bin sbin home share man etc usr
cp -fa $TARGET_LIBS/*.so* $TARGET_ROOTFS/lib

for li in  $(ls $TARGET_ROOTFS/lib/*.so* )
do
  if [ "$(file $li | sed -e '/ASCII/d')" == "" ]; then
      rm -f $li
  fi
done

$CROSS_COMPILE-strip $TARGET_ROOTFS/lib/*.so*

