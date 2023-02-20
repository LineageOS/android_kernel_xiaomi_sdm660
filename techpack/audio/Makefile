# auto-detect subdirs
ifeq ($(CONFIG_ARCH_SDXPOORWILLS), y)
include $(srctree)/techpack/audio/config/sdxpoorwillsauto.conf
export $(shell sed 's/=.*//' $(srctree)/techpack/audio/config/sdxpoorwillsauto.conf)
endif
ifeq ($(CONFIG_ARCH_SM8150), y)
include $(srctree)/techpack/audio/config/sm8150auto.conf
export $(shell sed 's/=.*//' $(srctree)/techpack/audio/config/sm8150auto.conf)
endif
ifeq ($(CONFIG_ARCH_SDMSHRIKE), y)
include $(srctree)/techpack/audio/config/sm8150auto.conf
export $(shell sed 's/=.*//' $(srctree)/techpack/audio/config/sm8150auto.conf)
endif
ifeq ($(CONFIG_ARCH_KONA), y)
include $(srctree)/techpack/audio/config/konaauto.conf
export $(shell sed 's/=.*//' $(srctree)/techpack/audio/config/konaauto.conf)
endif
ifeq ($(CONFIG_ARCH_LITO), y)
include $(srctree)/techpack/audio/config/litoauto.conf
export $(shell sed 's/=.*//' $(srctree)/techpack/audio/config/litoauto.conf)
endif
ifeq ($(CONFIG_ARCH_BENGAL), y)
include $(srctree)/techpack/audio/config/bengalauto.conf
export $(shell sed 's/=.*//' $(srctree)/techpack/audio/config/bengalauto.conf)
endif
ifeq ($(CONFIG_ARCH_SDM660), y)
include $(srctree)/techpack/audio/config/sdm660auto.conf
export $(shell sed 's/=.*//' $(srctree)/techpack/audio/config/sdm660auto.conf)
endif
ifeq ($(CONFIG_MACH_XIAOMI_JASON), y)
include $(srctree)/techpack/audio/config/xiaomi/jasonauto.conf
export $(shell sed 's/=.*//' $(srctree)/techpack/audio/config/xiaomi/jasonauto.conf)
endif
ifeq ($(CONFIG_MACH_XIAOMI_LAVENDER), y)
include $(srctree)/techpack/audio/config/xiaomi/lavenderauto.conf
export $(shell sed 's/=.*//' $(srctree)/techpack/audio/config/xiaomi/lavenderauto.conf)
endif
ifeq ($(CONFIG_MACH_XIAOMI_PLATINA), y)
include $(srctree)/techpack/audio/config/xiaomi/platinaauto.conf
export $(shell sed 's/=.*//' $(srctree)/techpack/audio/config/xiaomi/platinaauto.conf)
endif
ifeq ($(CONFIG_MACH_XIAOMI_TULIP), y)
include $(srctree)/techpack/audio/config/xiaomi/tulipauto.conf
export $(shell sed 's/=.*//' $(srctree)/techpack/audio/config/xiaomi/tulipauto.conf)
endif
ifeq ($(CONFIG_MACH_XIAOMI_WAYNE), y)
include $(srctree)/techpack/audio/config/xiaomi/wayneauto.conf
export $(shell sed 's/=.*//' $(srctree)/techpack/audio/config/xiaomi/wayneauto.conf)
endif
ifeq ($(CONFIG_MACH_XIAOMI_WHYRED), y)
include $(srctree)/techpack/audio/config/xiaomi/whyredauto.conf
export $(shell sed 's/=.*//' $(srctree)/techpack/audio/config/xiaomi/whyredauto.conf)
endif

# Use USERINCLUDE when you must reference the UAPI directories only.
USERINCLUDE     += \
                -I$(srctree)/techpack/audio/include/uapi \
                -I$(srctree)/techpack/audio/include

# Use LINUXINCLUDE when you must reference the include/ directory.
# Needed to be compatible with the O= option
LINUXINCLUDE    += \
                -I$(srctree)/techpack/audio/include/uapi \
                -I$(srctree)/techpack/audio/include/elliptic \
                -I$(srctree)/techpack/audio/include

ifeq ($(CONFIG_ARCH_SDXPOORWILLS), y)
LINUXINCLUDE    += \
                -include $(srctree)/techpack/audio/config/sdxpoorwillsautoconf.h
endif
ifeq ($(CONFIG_ARCH_SM8150), y)
LINUXINCLUDE    += \
                -include $(srctree)/techpack/audio/config/sm8150autoconf.h
endif
ifeq ($(CONFIG_ARCH_SDMSHRIKE), y)
LINUXINCLUDE    += \
                -include $(srctree)/techpack/audio/config/sm8150autoconf.h
endif
ifeq ($(CONFIG_ARCH_KONA), y)
LINUXINCLUDE    += \
                -include $(srctree)/techpack/audio/config/konaautoconf.h
endif
ifeq ($(CONFIG_ARCH_LITO), y)
LINUXINCLUDE    += \
                -include $(srctree)/techpack/audio/config/litoautoconf.h
endif
ifeq ($(CONFIG_ARCH_BENGAL), y)
LINUXINCLUDE    += \
                -include $(srctree)/techpack/audio/config/bengalautoconf.h
endif
ifeq ($(CONFIG_ARCH_SDM660), y)
LINUXINCLUDE    += \
                -include $(srctree)/techpack/audio/config/sdm660autoconf.h
endif
ifeq ($(CONFIG_MACH_XIAOMI_JASON), y)
LINUXINCLUDE    += \
                -include $(srctree)/techpack/audio/config/xiaomi/jasonautoconf.h
endif
ifeq ($(CONFIG_MACH_XIAOMI_LAVENDER), y)
LINUXINCLUDE    += \
                -include $(srctree)/techpack/audio/config/xiaomi/lavenderautoconf.h
endif
ifeq ($(CONFIG_MACH_XIAOMI_PLATINA), y)
LINUXINCLUDE    += \
                -include $(srctree)/techpack/audio/config/xiaomi/platinaautoconf.h
endif
ifeq ($(CONFIG_MACH_XIAOMI_TULIP), y)
LINUXINCLUDE    += \
                -include $(srctree)/techpack/audio/config/xiaomi/tulipautoconf.h
endif
ifeq ($(CONFIG_MACH_XIAOMI_WAYNE), y)
LINUXINCLUDE    += \
                -include $(srctree)/techpack/audio/config/xiaomi/wayneautoconf.h
endif
ifeq ($(CONFIG_MACH_XIAOMI_WHYRED), y)
LINUXINCLUDE    += \
                -include $(srctree)/techpack/audio/config/xiaomi/whyredautoconf.h
endif
obj-y += soc/
obj-y += dsp/
obj-y += ipc/
obj-y += asoc/
