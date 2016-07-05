//
//  randombytes.c
//  Follow
//
//  Created by Erich Ocean on 6/19/16.
//  Copyright © 2016 Xy Group Ltd. All rights reserved.
//

#include "randombytes.h"

#include <CommonCrypto/CommonCrypto.h>
#include <CommonCrypto/CommonRandom.h>

void randombytes(unsigned char *buf, unsigned long long len)
{
    CCRandomGenerateBytes((void *)buf, len);
}
