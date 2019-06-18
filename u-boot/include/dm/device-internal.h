/*
 * Copyright (C) 2013 Google, Inc
 *
 * (C) Copyright 2012
 * Pavel Herrmann <morpheus.ibis@gmail.com>
 * Marek Vasut <marex@denx.de>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _DM_DEVICE_INTERNAL_H
#define _DM_DEVICE_INTERNAL_H

struct udevice;

/**
 * device_bind() - Create a device and bind it to a driver
 *
 * Called to set up a new device attached to a driver. The device will either
 * have platdata, or a device tree node which can be used to create the
 * platdata.
 *
 * Once bound a device exists but is not yet active until device_probe() is
 * called.
 *
 * @parent: Pointer to device's parent, under which this driver will exist
 * @drv: Device's driver
 * @name: Name of device (e.g. device tree node name)
 * @platdata: Pointer to data for this device - the structure is device-
 * specific but may include the device's I/O address, etc.. This is NULL for
 * devices which use device tree.
 * @of_offset: Offset of device tree node for this device. This is -1 for
 * devices which don't use device tree.
 * @devp: Returns a pointer to the bound device
 * @return 0 if OK, -ve on error
 */
int device_bind(struct udevice *parent, const struct driver *drv,
		const char *name, void *platdata, int of_offset,
		struct udevice **devp);

/**
 * device_bind_by_name: Create a device and bind it to a driver
 *
 * This is a helper function used to bind devices which do not use device
 * tree.
 *
 * @parent: Pointer to device's parent
 * @pre_reloc_only: If true, bind the driver only if its DM_INIT_F flag is set.
 * If false bind the driver always.
 * @info: Name and platdata for this device
 * @devp: Returns a pointer to the bound device
 * @return 0 if OK, -ve on error
 */
int device_bind_by_name(struct udevice *parent, bool pre_reloc_only,
			const struct driver_info *info, struct udevice **devp);

/**
 * device_probe() - Probe a device, activating it
 *
 * Activate a device so that it is ready for use. All its parents are probed
 * first.
 *
 * @dev: Pointer to device to probe
 * @return 0 if OK, -ve on error
 */
int device_probe(struct udevice *dev);

/**
 * device_probe() - Probe a child device, activating it
 *
 * Activate a device so that it is ready for use. All its parents are probed
 * first. The child is provided with parent data if parent_priv is not NULL.
 *
 * @dev: Pointer to device to probe
 * @parent_priv: Pointer to parent data. If non-NULL then this is provided to
 * the child.
 * @return 0 if OK, -ve on error
 */
int device_probe_child(struct udevice *dev, void *parent_priv);

/**
 * device_remove() - Remove a device, de-activating it
 *
 * De-activate a device so that it is no longer ready for use. All its
 * children are deactivated first.
 *
 * @dev: Pointer to device to remove
 * @return 0 if OK, -ve on error (an error here is normally a very bad thing)
 */
#ifdef CONFIG_DM_DEVICE_REMOVE
int device_remove(struct udevice *dev);
#else
static inline int device_remove(struct udevice *dev) { return 0; }
#endif

/**
 * device_unbind() - Unbind a device, destroying it
 *
 * Unbind a device and remove all memory used by it
 *
 * @dev: Pointer to device to unbind
 * @return 0 if OK, -ve on error
 */
#ifdef CONFIG_DM_DEVICE_REMOVE
int device_unbind(struct udevice *dev);
#else
static inline int device_unbind(struct udevice *dev) { return 0; }
#endif

/**
 * device_remove_children() - Stop all device's children
 * @dev:	The device whose children are to be removed
 * @return 0 on success, -ve on error
 */
#ifdef CONFIG_DM_DEVICE_REMOVE
int device_remove_children(struct udevice *dev);
#else
static inline int device_remove_children(struct udevice *dev) { return 0; }
#endif

/**
 * device_unbind_children() - Unbind all device's children from the device
 *
 * On error, the function continues to unbind all children, and reports the
 * first error.
 *
 * @dev:	The device that is to be stripped of its children
 * @return 0 on success, -ve on error
 */
#ifdef CONFIG_DM_DEVICE_REMOVE
int device_unbind_children(struct udevice *dev);
#else
static inline int device_unbind_children(struct udevice *dev) { return 0; }
#endif

#ifdef CONFIG_DM_DEVICE_REMOVE
void device_free(struct udevice *dev);
#else
static inline void device_free(struct udevice *dev) {}
#endif

/* Cast away any volatile pointer */
#define DM_ROOT_NON_CONST		(((gd_t *)gd)->dm_root)
#define DM_UCLASS_ROOT_NON_CONST	(((gd_t *)gd)->uclass_root)

#endif