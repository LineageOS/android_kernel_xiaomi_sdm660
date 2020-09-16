version_tr=1.0.1
# Kernel
KERNEL_DIR=$PWD
KERNEL="Image.gz-dtb"
KERN_IMG=$KERNEL_DIR/out/arch/arm64/boot/Image.gz-dtb
JASON_DTB=$KERNEL_DIR/out/arch/arm64/boot/dts/qcom/sdm660-mtp-jason.dtb
HOME=~/
# Build Start [needed for calculating time]
BUILD_START=$(date +"%s")

# AnyKernel2 Dir
ANYKERNEL_DIR=${HOME}Projects/Kernel/AnyKernel

# Export Zip Here
EXPORT_DIR=${HOME}Projects/Kernel/kernelreleases

ZIP_NAME="TurKernel-LTS-Manas-AOSP-"
ZIP_NAME+="Stable-"
ZIP_NAME+="v$version_tr-"
DASH_DATE=`echo $(date +'%d/%m/%Y/%H%M') | sed 's/\//-/g'`
ZIP_NAME+=$DASH_DATE

# Set User and Host
export KBUILD_BUILD_USER="onur"
export KBUILD_BUILD_HOST="ubuntu"

# Branding Stock Kernel
echo "-----------------------------------------------"
echo "  Initializing build to compile Ver: $ZIP_NAME "
echo "-----------------------------------------------"

echo -e "***********************************************"
echo     "         Creating Output Directory: out       "
echo -e "***********************************************"

# Create Out
mkdir -p out

echo -e "***********************************************"
echo    "         Initialising jason_defconfig          "
echo -e "***********************************************"

# Init Defconfig
make O=out ARCH=arm64 jason_defconfig

echo -e "***********************************************"
echo    "          Building TurKernel                    "
echo -e "***********************************************"


# make
PATH="/home/onur/Projects/Kernel/linux-x86_clang/bin:/home/onur/Projects/Kernel/aarch64-linux-android-4.9/bin:/home/onur/Projects/Kernel/arm-linux-androideabi-4.9/bin:${PATH}" \
make -j$(nproc --all) O=out \
                      ARCH=arm64 \
                      CC=clang \
                      CLANG_TRIPLE=aarch64-linux-gnu- \
                      CROSS_COMPILE_ARM32=arm-linux-androideabi- \
                      CROSS_COMPILE=aarch64-linux-android-

# If the above was successful
if [ -a $KERN_IMG ]; then
   BUILD_RESULT_STRING="BUILD SUCCESSFUL"

echo -e "***********************************************"
echo    "            Making Flashable Zip               "
echo -e "***********************************************"
   # AnyKernel2 Magic Begins!
   # Make the zip file
   echo "MAKING FLASHABLE ZIP"

   # Move the zImage to AnyKernel2 dir
   cp -vr ${KERN_IMG} ${ANYKERNEL_DIR}/zImage
   cp -vr ${JASON_DTB} ${ANYKERNEL_DIR}/zImage-dtb
   cd ${ANYKERNEL_DIR}
   zip -r9 ${ZIP_NAME}.zip * -x README ${ZIP_NAME}.zip

else
   BUILD_RESULT_STRING="BUILD FAILED"
fi

# Export Zip
NOW=$(date +"%m-%d")
ZIP_LOCATION=${ANYKERNEL_DIR}/${ZIP_NAME}.zip
ZIP_EXPORT=${EXPORT_DIR}/${NOW}
ZIP_EXPORT_LOCATION=${EXPORT_DIR}/${NOW}/${ZIP_NAME}.zip

rm -rf ${ZIP_EXPORT}
mkdir ${ZIP_EXPORT}
mv ${ZIP_LOCATION} ${ZIP_EXPORT}
cd ${HOME}

echo ""

if [[ -e $ZIP_EXPORT_LOCATION ]]; then
echo "------------------------------"
# add your uploader script
echo -e "\n------------------------------"
fi

# Deleting residual files
echo "Deleting unnecessary files..."
rm -rf ${ANYKERNEL_DIR}/zImage
rm -rf ${ANYKERNEL_DIR}/zImage-dtb
echo "Deleted unnecessary files!"

# End the script
echo "${BUILD_RESULT_STRING}!"

# End the Build and Print the Compilation Time
BUILD_END=$(date +"%s")
DIFF=$(($BUILD_END - $BUILD_START))
echo -e "Build completed in $(($DIFF / 60)) minute(s) and $(($DIFF % 60)) seconds."
