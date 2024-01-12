#!/bin/bash
# Script outline to install and build kernel.
# Author: Siddhant Jajoo.

set -e
set -u

OUTDIR=/tmp/aeld
KERNEL_REPO=git://git.kernel.org/pub/scm/linux/kernel/git/stable/linux-stable.git
KERNEL_VERSION=v5.1.10
BUSYBOX_VERSION=1_33_1
FINDER_APP_DIR=$(realpath $(dirname $0))
ARCH=arm64
CROSS_COMPILE=aarch64-none-linux-gnu-

if [ $# -lt 1 ]
then
	echo "Using default directory ${OUTDIR} for output"
else
	OUTDIR=$1
	echo "Using passed directory ${OUTDIR} for output"
fi

mkdir -p ${OUTDIR}

cd ${OUTDIR}
if [ ! -d "${OUTDIR}/linux-stable" ]; then
    #Clone only if the repository does not exist.
	echo "CLONING GIT LINUX STABLE VERSION ${KERNEL_VERSION} IN ${OUTDIR}"
	git clone ${KERNEL_REPO} --depth 1 --single-branch --branch ${KERNEL_VERSION}
fi
if [ ! -e ${OUTDIR}/linux-stable/arch/${ARCH}/boot/Image ]; then
    cd linux-stable
    echo "Checking out version ${KERNEL_VERSION}"
    git checkout ${KERNEL_VERSION}

    # TODO: Add your kernel build steps here
    # can use EXPORT?
    # deep clean kernel build tree
    make ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} mrproper
    # Use provided configuration for 'virt' arm dev board
    make ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} defconfig
    # build kernel image for QEMU
    make -j4 ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} all
    # build modules
    make ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} modules
    # build device tree
    make ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} dtbs
fi

echo "Adding the Image in outdir"
# Copy image file to ${OUTDIR}
cp ${OUTDIR}/linux-stable/arch/${ARCH}/boot/Image ${OUTDIR}

echo "Creating the staging directory for the root filesystem"
cd ${OUTDIR}
if [ -d "${OUTDIR}/rootfs" ]
then
	echo "Deleting rootfs directory at ${OUTDIR}/rootfs and starting over"
    sudo rm  -rf ${OUTDIR}/rootfs
fi

# TODO: Create necessary base directories
# Create root filesystem
mkdir -p ${OUTDIR}/rootfs
cd "${OUTDIR}/rootfs"
mkdir -p bin dev etc home lib lib64 proc sbin sys tmp usr var
mkdir -p usr/bin usr/lib usr/sbin
mkdir -p var/log

cd ${OUTDIR}
if [ ! -d "${OUTDIR}/busybox" ]
then
git clone git://busybox.net/busybox.git
    cd busybox
    git checkout ${BUSYBOX_VERSION}
    # TODO:  Configure busybox
    make distclean
    make defconfig
else
    cd busybox
fi

# TODO: Make and install busybox
make -j4 ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE}
#make
make CONFIG_PREFIX=${OUTDIR}/rootfs ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} install
#make CONFIG_PREFIX=${OUTDIR}/rootfs install
# change dir to rootfs in order to get dependency
cd "${OUTDIR}/rootfs"

echo "Library dependencies"
${CROSS_COMPILE}readelf -a bin/busybox | grep "program interpreter"
${CROSS_COMPILE}readelf -a bin/busybox | grep "Shared library"

# TODO: Add library dependencies to rootfs
SYS_ROOT=$(${CROSS_COMPILE}gcc -print-sysroot)
# if entire libc is copied, there will be "kernel panic - not syncing no working init found" fault, weird...
cp ${SYS_ROOT}/lib/ld-linux-aarch64.so.1 ${OUTDIR}/rootfs/lib
cp ${SYS_ROOT}/lib64/libm.so.6 ${SYS_ROOT}/lib64/libresolv.so.2 ${SYS_ROOT}/lib64/libc.so.6 ${OUTDIR}/rootfs/lib64
#ln -s bin/busybox init

# TODO: Make device nodes
# Create NUll device
sudo mknod -m 666 ${OUTDIR}/rootfs/dev/null c 1 3
# Create console device
sudo mknod -m 666 ${OUTDIR}/rootfs/dev/console c 5 1

# TODO: Clean and build the writer utility
cd "${FINDER_APP_DIR}"
make clean
make all
#rm -f  *.o writer *.elf *.map
#${CROSS_COMPILE}gcc -g -Wall -c -o writer.o writer.c
#${CROSS_COMPILE}gcc -g -Wall -I/ writer.o -o writer

# TODO: Copy the finder related scripts and executables to the /home directory
# on the target rootfs  
#cp -r -L ${FINDER_APP_DIR}/* ${OUTDIR}/rootfs/home
mkdir -p ${OUTDIR}/rootfs/home/conf
cp -r ${FINDER_APP_DIR}/conf/* ${OUTDIR}/rootfs/home/conf
cp ${FINDER_APP_DIR}/finder.sh ${FINDER_APP_DIR}/finder-test.sh ${FINDER_APP_DIR}/autorun-qemu.sh ${FINDER_APP_DIR}/writer ${OUTDIR}/rootfs/home

# TODO: Chown the root directory
# Recursively change all files under rootfs directory
sudo chown -R root:root ${OUTDIR}/rootfs

# TODO: Create initramfs.cpio.gz
# Bundle root filesystem into cpio file
cd "${OUTDIR}/rootfs"
find . | cpio -H newc -ov --owner root:root > ${OUTDIR}/initramfs.cpio
# create gz file
cd "${OUTDIR}"
gzip -f initramfs.cpio

# remember to modify Makefile to make it cross-compile
# also remember to modify #!/bin/bash into #!/bin/sh in finder.sh