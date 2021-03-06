LINUX=/home/xm.chen/work/linuxdrv/linux-kernel-2.6
PREFIX=
KCC=arm-linux-gcc
UCC=arm-linux-gcc
LD=arm-linux-ld
KFLAGS=
UFLAGS=
PCDEBUG=
USE_PM=y
UNSAFE_TOOLS=y
# CONFIG_CARDBUS is not defined
# CONFIG_PNP_BIOS is not defined
UTS_RELEASE=2.6.12.5
UTS_VERSION=#244 Thu Feb 28 15:06:49 CST 2008
LINUX_VERSION_CODE=132620

MODDIR=/lib/modules/2.6.12.5

# Options from /home/xm.chen/work/linuxdrv/linux-kernel-2.6/.config
CONFIG_PCMCIA=y
# CONFIG_SMP is not defined
# CONFIG_PREEMPT is not defined
# CONFIG_RTHAL is not defined
# CONFIG_HIGHMEM is not defined
# CONFIG_PCI is not defined
# CONFIG_PM is not defined
CONFIG_SCSI=y
# CONFIG_IEEE1394 is not defined
CONFIG_INET=y
# CONFIG_NET_PCMCIA_RADIO is not defined
# CONFIG_TR is not defined
# CONFIG_NET_FASTROUTE is not defined
# CONFIG_NET_DIVERT is not defined
# CONFIG_MODVERSIONS is not defined
# CONFIG_DEBUG_KERNEL is not defined
CONFIG_PROC_FS=y
CONFIG_RWSEM_GENERIC_SPINLOCK=y
# CONFIG_RWSEM_XCHGADD_ALGORITHM is not defined
ARCH=arm
HOST_ARCH=i386
AFLAGS=
# CONFIG_ISA is not defined
CONFIG_UID16=y

CPPFLAGS=-I../include/static -I$(LINUX)/include -I../include
WFLAG=-Wa,--no-warn
HAS_PROC_BUS=y
DO_IDE=y
SYSV_INIT=y
RC_DIR=/etc/rc.d
HAS_WORDEXP=y
MANDIR=/usr/share/man/
XMANDIR=/usr/share/man/
