// Type definitions for keygrip
// Project: https://github.com/crypto-utils/keygrip
// Definitions by: Erich Ocean <https://github.com/erichocean>

/// <reference path="node.d.ts" />

declare module "keygrip" {

  class Keygrip {
    constructor(keys: string[])

    sign(data: any) : Buffer
    indexOf(data: any) : number
    verify(data: any, digest: any) : boolean
  }

  export = Keygrip
}
