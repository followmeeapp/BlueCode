// Type definitions for cookies
// Project: https://github.com/pgriess/node-msgpack
// Definitions by: Erich Ocean <https://github.com/erichocean>

/// <reference path="node.d.ts" />
/// <reference path="keygrip.d.ts" />

declare module "cookies" {
  import * as http from 'http'
  import Keygrip = require('keygrip')

  class Cookies {
    constructor(req: http.ServerRequest, res: http.ServerResponse, keys?: string[]|Keygrip)

    get(name: string, options?: {
      secure: boolean
    }) : string

    set(name: string, value?: string, options?: {
      maxAge?: number
      expires?: Date
      path?: string
      domain?: string
      secure?: boolean
      secureProxy?: boolean
      httpOnly?: boolean
      signed?: boolean
      overwrite?: boolean
    }) : void
 }

  export = Cookies
}
