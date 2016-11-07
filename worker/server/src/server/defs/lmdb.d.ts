// Type definitions for lmdb
// Project: https://github.com/xygroup/node-lmdb
// Definitions by: Erich Ocean <https://github.com/erichocean>

/// <reference path="node.d.ts" />

declare module "lmdb" {
  interface EnvOptions {
    path: string,
    mapSize: number,
    maxDbs: number,
    maxReaders: number
  }

  interface DbiOptions {
    name: string,
    create?: boolean,
    keyIsUint32?: boolean,
    dupSort?: boolean,
    dupFixed?: boolean,
    integerDup?: boolean
  }

  interface TxnOptions {
    readOnly?: boolean
  }

  class Env {
    open(options: EnvOptions) : Env
    openDbi(options: DbiOptions) : Dbi
    beginTxn(options?: TxnOptions) : Txn
  }

  class Dbi {

  }

  class Txn {
    commit() : void
    abort() : void

    del(db: Dbi, key: string|number) : void

    getNumber(db: Dbi, key: string|number) : number
    putNumber(db: Dbi, key: string|number, val: number) : void

    getString(db: Dbi, key: string|number) : string
    putString(db: Dbi, key: string|number, val: string) : void

    getBinary(db: Dbi, key: string|number) : Buffer
    putBinary(db: Dbi, key: string|number, val: Buffer) : void
  }

  class Cursor {
    constructor(Txn, Dbi)

    goToFirst() : number|string
    goToNext() : number|string
    goToLast() : number|string
    close() : void

    getCurrentBinary(callback: (key: string|number, data: Buffer) => void) : void
  }
}
