// Type definitions for msgpack
// Project: https://github.com/pgriess/node-msgpack
// Definitions by: Erich Ocean <https://github.com/erichocean>

/// <reference path="node.d.ts" />

declare module "msgpack" {

  module MessagePack {
    export function pack(jsonValue: any) : Buffer    
    export function unpack(data: Buffer) : any    
  }

  export = MessagePack
}
