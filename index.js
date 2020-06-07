import { devices, HID } from 'node-hid'
import { exec as execChild } from 'child_process'
import { promisify } from 'util'
import EventEmitter from 'events'

const VENDOR_ID = 0xCB10
const PRODUCT_ID = 0x2257
const INTERFACE = 0

const ENUM = value => Object.freeze(value)
const STATES = ENUM({
  none: 0,
  normal: 1,
  insert: 2,
  visual: 3,
  operator: 4
})

const exec = promisify(execChild)

const getState = async () => {
  const { stdout } = await exec('emacs-mode')
  const value = stdout.trim().split('-')[0]
  const state = STATES[value]
  return state || STATES.none
}

const connect = async () => {
  const device = devices().find(({ vendorId, productId, interface: interfaceId }) => {
    return VENDOR_ID === vendorId && PRODUCT_ID === productId && INTERFACE  === interfaceId
  })
  if (!device) throw new Error(`
Device not found with VID '${VENDOR_ID}' and PID '${PRODUCT_ID}'.
Please ensure it is plugged in.
  `)
  return new HID(device.path)
}

const disconnect = device => device && device.close()

const poller = interval => {
  const emitter = new EventEmitter()
  const poll = () => setTimeout(() => emitter.emit('poll'), interval)
  const on = func => {
    const wrapper = async () => {
      await func()
      poll()
    }
    emitter.on('poll', wrapper)
    return self
  }
  const self = { poll, on }
  return self
}

connect().then(device => {
  poller(500).on(async () => {
    const state = await getState()
    device.write([1, state])
  }).poll()
}).catch(({ message }) => {
  console.error(message)
}).then(disconnect)
