//
//  Utilities.m
//  Follow
//
//  Copyright © 2016 Follow-Mee, Incorporated. All rights reserved.
//

#import "Utilities.h"

// Taken from http://stackoverflow.com/a/9009321
@implementation NSData (Hex)

- (NSString *) hexString
{
    NSUInteger length = self.length;
    unichar* hexChars = (unichar*)malloc(sizeof(unichar) * (length*2));
    unsigned char* bytes = (unsigned char*)self.bytes;
    for (NSUInteger i = 0; i < length; i++) {
        unichar c = bytes[i] / 16;
        if (c < 10) c += '0';
        else c += 'A' - 10;
        hexChars[i*2] = c;
        c = bytes[i] % 16;
        if (c < 10) c += '0';
        else c += 'A' - 10;
        hexChars[i*2+1] = c;
    }
    NSString* retVal = [[NSString alloc] initWithCharactersNoCopy:hexChars length:length*2 freeWhenDone:YES];
    return [retVal lowercaseString];
}

// Taken from http://stackoverflow.com/a/17006367
+ (NSData *) dataFromHexString: (NSString * )bytesString
{
    if (!bytesString || !bytesString.length) return NULL;

    // Get the c string
    const char *scanner=[bytesString cStringUsingEncoding:NSUTF8StringEncoding];
    char twoChars[3]={0,0,0};
    long bytesBlockSize = bytesString.length/2;
    long counter = bytesBlockSize;
    Byte *bytesBlock = malloc(bytesBlockSize);
    if (!bytesBlock) return NULL;
    Byte *writer = bytesBlock;
    while (counter--) {
        twoChars[0]=*scanner++;
        twoChars[1]=*scanner++;
        *writer++ = strtol(twoChars, NULL, 16);
    }
    return [NSData dataWithBytesNoCopy:bytesBlock length:bytesBlockSize freeWhenDone:YES];
}

// Taken from http://stackoverflow.com/a/14194440
- (NSString *) encodeBase64
{
    static char* alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=";

    unsigned int length = (unsigned int)self.length;
    unsigned const char* rawData = self.bytes;

    //empty data = empty output
    if (length == 0) {
        return @"";
    }

    unsigned int outputLength = (((length + 2) / 3) * 4);

    //let's allocate buffer for the output
    char* rawOutput = malloc(outputLength + 1);

    //with each step we get 3 bytes from the input and write 4 bytes to the output
    for (unsigned int i = 0, outputIndex = 0; i < length; i += 3, outputIndex += 4) {
        BOOL triple = NO;
        BOOL quad = NO;

        //get 3 bytes (or only 1 or 2 when we have reached the end of input)
        unsigned int value = rawData[i];
        value <<= 8;

        if (i + 1 < length) {
            value |= rawData[i + 1];
            triple = YES;
        }

        value <<= 8;

        if (i + 2 < length) {
            value |= rawData[i + 2];
            quad = YES;
        }

        //3 * 8 bits written as 4 * 6 bits (indexing the 64 chars of the alphabet)
        //write = if end of input reached
        rawOutput[outputIndex + 3] = (quad) ? alphabet[value & 0x3F] : '=';
        value >>= 6;
        rawOutput[outputIndex + 2] = (triple) ? alphabet[value & 0x3F] : '=';
        value >>= 6;
        rawOutput[outputIndex + 1] = alphabet[value & 0x3F];
        value >>= 6;
        rawOutput[outputIndex] = alphabet[value & 0x3F];
    }

    rawOutput[outputLength] = 0;

//    NSString* output = [NSString stringWithCString:rawOutput encoding:NSASCIIStringEncoding];
//    free(rawOutput);

    // This is more efficient.
    NSString *output = [[NSString alloc] initWithBytesNoCopy: rawOutput
                                         length:              outputLength - 1 // don't include the trailing NULL
                                         encoding:            NSUTF8StringEncoding
                                         freeWhenDone:        YES                 ];


    return output;
}

@end