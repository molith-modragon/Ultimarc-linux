/*
 ============================================================================
 Name        : common.c
 Author      : Robert Abram, Katie Snow
 Version     :
 Copyright   : Copyright 2014 Robert Abram, Katie Snow
 Description : Common functions between device libraries
 ============================================================================
 */

/* C */
#include <stdio.h>
#include <json.h>

#include "common.h"
#include "dbg.h"

struct libusb_device_handle*
openUSB(libusb_context *ctx, uint16_t vendor, uint16_t product, int interface, int autoconnect)
{
  int ret = 0;

  struct libusb_device_handle *handle = NULL;

  /* Open USB communication */
  ret = libusb_init(&ctx);
  if (ret < 0)
  {
    log_err("libusb_init failed: %i.", ret);
    goto error;
  }
  libusb_set_debug(ctx, 3);

  handle = libusb_open_device_with_vid_pid(ctx, vendor, product);
  if (!handle)
  {
    log_err("Unable to open device.");
    goto error;
  }

  if (autoconnect == 0)
  {
    /* detach the kernel driver */
    if(libusb_kernel_driver_active(handle, interface) == 1)
    {
      log_info ("Kernel Driver Active.");
      if(libusb_detach_kernel_driver(handle, interface) == 0) //detach it
        log_info ("Kernel Driver Detached.");
    }
  }
  else
  {
    libusb_set_auto_detach_kernel_driver(handle, 1);
  }

  ret = libusb_claim_interface(handle, interface);
  if (ret < 0)
  {
    log_err ("Unable to claim interface.");
    goto error;
  }

  return handle;

  error:
  closeUSB(ctx, handle, interface);
  return NULL;
}

void
closeUSB(libusb_context *ctx, struct libusb_device_handle *handle, int interface)
{
  if (handle)
  {
    libusb_release_interface(handle, interface);
    libusb_close(handle);
  }

  if (ctx)
    libusb_exit(ctx);

}

bool checkBoardID(json_object* jobj, const char* entry)
{
  bool valid = false;
  int id = -1;

  json_object* tmp = NULL;

  if (json_object_object_get_ex(jobj, entry, &tmp))
  {
    if (json_object_get_type(tmp) == json_type_int)
    {
      id = json_object_get_int(tmp);
      if (id > 0 && id < 5)
      {
        valid = true;
      }
      else
      {
        log_err ("'%s' is not a valid entry.  Valid entries are 1 - 4", entry);
      }
    }
    else
    {
      log_err ("'%s' is not defined as an integer", entry);
    }
  }

  return valid;
}

uint8_t convertDecimalToHex(int decimal)
{
  if (decimal < 0 || decimal > 0xFF) return 0;
  return decimal & 0xFF;
}
