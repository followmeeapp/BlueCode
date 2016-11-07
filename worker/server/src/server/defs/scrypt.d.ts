// Type definitions for scrypt
// Project: https://github.com/barrysteyn/node-scrypt
// Definitions by: Erich Ocean <https://github.com/erichocean>

/// <reference path="node.d.ts" />

declare module "scrypt" {

  module Scrypt {
    export function paramsSync(maxtime: number) : any
    export function paramsSync(maxtime: number, maxmem: number, max_memfrac?: number) : any
    export function kdfSync(key: string|Buffer, paramsObject) : Buffer
    export function kdf(key: string|Buffer, paramsObject, callback: (err, result: Buffer) => void) : void
    export function verifyKdf(kdf: Buffer, key: string|Buffer, callback: (err, result: boolean) => void) : void
  }

  export = Scrypt
}
