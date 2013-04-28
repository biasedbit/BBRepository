//
// Copyright 2013 BiasedBit
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

//
//  Created by Bruno de Carvalho (@biasedbit, http://biasedbit.com)
//  Copyright (c) 2013 BiasedBit. All rights reserved.
//

#import "BBRepository+FileHandlingHelpers.h"



#pragma mark -

@implementation BBRepository (FileHandlingHelpers)


#pragma mark Interface

- (NSString*)relativePathForFile:(NSString*)file
{
    return [[self repositoryName] stringByAppendingPathComponent:file];
}

- (NSString*)fullPathForFile:(NSString*)file
{
    NSString* relativePath = [self relativePathForFile:file];
    return [self convertRelativeToFullPath:relativePath];
}

- (NSString*)convertRelativeToFullPath:(NSString*)relativePath
{
    return [[self baseStoragePath] stringByAppendingPathComponent:relativePath];
}

- (void)deleteFileInBackground:(NSString*)fullPathToFile
{
    if (fullPathToFile == nil) return;

    dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_BACKGROUND, 0), ^{
        NSError* error = nil;
        if ([[NSFileManager defaultManager] removeItemAtPath:fullPathToFile error:&error]) {
            LogTrace(@"[%@] Deleted file at '%@'.", self.repositoryName, fullPathToFile);
        } else {
            // Never fails unless file doesn't exist...
            LogError(@"[%@] Could not delete file at '%@': %@",
                     self.repositoryName, fullPathToFile, [error description]);
        }
    });
}

@end
