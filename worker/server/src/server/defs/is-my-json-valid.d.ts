// Type definitions for is-my-json-valid
// Project: https://github.com/mafintosh/is-my-json-valid
// Definitions by: Erich Ocean <https://github.com/erichocean>

/// <reference path="node.d.ts" />

declare module "is-my-json-valid" {

  // Returns a "validate" function.
  function validator(jsonSchema: any, options?: any) : (jsonValue: any) => boolean

  export = validator
}
