//
//  WebGZipDecoder.h
//  WebKit
//
//  Created by Darin Adler on Wed Dec 04 2002.
//  Copyright (c) 2002 Apple Computer, Inc. All rights reserved.
//

#import <Foundation/Foundation.h>

#import <zlib.h>

@protocol WebDownloadDecoder;

@interface WebGZipDecoder : NSObject <WebDownloadDecoder>
{
    z_stream _stream;
    BOOL _streamInitialized;
    
    BOOL _failed;
}
@end
