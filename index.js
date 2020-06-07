import { devices, HID } from 'node-hid'

const VENDOR_ID = 0x046d
const PRODUCT_ID = 0xc068
const INTERFACE = 0

const device = devices().find(({ vendorId, productId, interface: interfaceId }) => {
  return VENDOR_ID === vendorId && PRODUCT_ID === productId && INTERFACE  === interfaceId
})

if (!device) process.exit()

const stream = new HID(device.path)

stream.on('data', (data) => console.log([...data]))

setTimeout(() => stream.close(), 5000)
