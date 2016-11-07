// Type definitions for graphql
// Project: https://github.com/graphql/graphql-js
// Definitions by: Erich Ocean <https://github.com/erichocean>

/// <reference path="node.d.ts" />

declare module "graphql" {
    module GraphQL {
        export function graphql(schema, requestString, rootValue?, variableValues?, operationName?) : any

        export class GraphQLSchema {
          constructor(config: any)
        }

        export class GraphQLScalarType {
          constructor(config: any)
        }

        export class GraphQLObjectType {
          constructor(config: any)
        }

        export class GraphQLInterfaceType {
          constructor(config: any)
        }

        export class GraphQLUnionType {
          constructor(config: any)
        }

        export class GraphQLEnumType {
          constructor(config: any)
        }

        export class GraphQLInputObjectType {
          constructor(config: any)
        }

        export class GraphQLList {
          constructor(config: any)
        }

        export class GraphQLNonNull {
          constructor(config: any)
        }

        export class GraphQLInt {
          constructor(config: any)
        }

        export class GraphQLFloat {
          constructor(config: any)
        }

        export class GraphQLString {
          constructor(config: any)
        }

        export class GraphQLBoolean {
          constructor(config: any)
        }

        export class GraphQLID {
          constructor(config: any)
        }

        // export function formatError: [Getter]

        // export interface IServerOptions {
        //     host?: string;
        //     port?: number;
        //     server?: http.Server;
        //     verifyClient?: {
        //         (info: {origin: string; secure: boolean; req: http.ServerRequest}): boolean;
        //         (info: {origin: string; secure: boolean; req: http.ServerRequest},
        //                                          callback: (res: boolean) => void): void;
        //     };
        //     handleProtocols?: any;
        //     path?: string;
        //     noServer?: boolean;
        //     disableHixie?: boolean;
        //     clientTracking?: boolean;
        // }
    }

    export = GraphQL
}
