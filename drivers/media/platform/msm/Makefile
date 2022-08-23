# SPDX-License-Identifier: GPL-2.0-only

#
# Makefile for the qti specific video device drivers
# based on V4L2.
#
obj-y += sde/
obj-$(CONFIG_MSM_VIDC_3X_V4L2) += vidc_3x/
obj-$(CONFIG_MSM_CVP_V4L2) += cvp/
obj-$(CONFIG_MSM_NPU) += npu/
obj-$(CONFIG_MSM_GLOBAL_SYNX) += synx/
obj-$(CONFIG_TSPP) += broadcast/
obj-$(CONFIG_DVB_MPQ) += dvb/

ifdef CONFIG_MSMB_CAMERA_LEGACY
obj-$(CONFIG_MSMB_CAMERA) += camera_v2-legacy/
else
obj-$(CONFIG_MSMB_CAMERA) += camera_v2/
endif
