// Type definitions for vows
// Project: https://github.com/vowsjs/vows
// Definitions by: Erich Ocean <https://github.com/erichocean>

/// <reference path="node.d.ts" />

// declare module "vows" {

//   class Suite {
//     addBatch(batch: Batch) : Suite
//     run()
//     export(module: any) // should be a Node.js module, but I don't know how to say that
//   }

//   interface Batch {
//     [key: string]: Context | SyncContext
//   }

//   // async context structure
//   interface Context {
//     topic?: (...args: any[]) => void
//     // TODO: I don't know if there can really be more than two arguments to an async Vow.
//     [key: string]: (err: any, ...topic: any[]) => void | Context | SyncContext
//   }

//   // sync context structure
//   interface SyncContext {
//     topic?: (...args: any[]) => any | any
//     [key: string]: (topic: any) => void | Context | SyncContext
//   }

//   module Vows {
//     export function describe(suiteName: string) : Suite
//   }

//   export = Vows
// }

declare module "vows" {

  class Suite {
    addBatch(batch: {}) : Suite
    run()
    export(module: any) // should be a Node.js module, but I don't know how to say that
  }

  module Vows {
    export function describe(suiteName: string) : Suite
  }

  export = Vows
}
