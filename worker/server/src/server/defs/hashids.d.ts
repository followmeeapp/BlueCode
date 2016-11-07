// Type definitions for hashids
// Project: https://github.com/pgriess/node-msgpack
// Definitions by: Erich Ocean <https://github.com/erichocean>

/// <reference path="node.d.ts" />

declare module "hashids" {

  class Hashids {
    constructor(salt: string, minimumLength?: number, alphabet?: string)

    encode(...numbers: number[]) : string
    decode(hashid: string) : number[]
  }

  export = Hashids
}
