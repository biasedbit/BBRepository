//
// Copyright 2012 BiasedBit
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
//  Copyright (c) 2012 BiasedBit. All rights reserved.
//

#import "BBRepositoryItem.h"



#pragma mark - Macros

#ifndef LogTrace
    #if DEBUG
        #define LogTrace(fmt, ...)  NSLog((@"(TRACE) " fmt), ##__VA_ARGS__);
    #elif
        #define LogTrace(...)
    #endif
#endif

#ifndef LogDebug
    #define LogDebug(fmt, ...)  NSLog((@"(DEBUG) " fmt), ##__VA_ARGS__);
#endif

#ifndef LogInfo
    #define LogInfo(fmt, ...)   NSLog((@"(INFO) " fmt), ##__VA_ARGS__);
#endif

#ifndef LogError
    #define LogError(fmt, ...)  NSLog((@"(ERROR) " fmt), ##__VA_ARGS__);
#endif



#pragma mark - Constants

extern NSString* const kBBRepositoryDefaultIdentifier;



#pragma mark -

@interface BBRepository : NSObject
{
@protected
    __strong NSString* _identifier;
    __strong NSString* _repositoryDirectory;
    __strong NSString* _repositoryIndex;
    __strong NSMutableDictionary* _entries;
}


#pragma mark Public properties

@property(strong, nonatomic, readonly) NSString* identifier;


#pragma mark Creation

- (id)initWithIdentifier:(NSString*)identifier;


#pragma mark Public methods

/* Completely purges all data managed by this repository (deletes file on disk and entries in memory) */
- (BOOL)reset;

/* Reload data from disk */
- (BOOL)reload;

/* Flush data in memory to disk */
- (BOOL)flush;

/* Number of managed entries (in-memory) by this repository */
- (NSUInteger)itemCount;

- (NSArray*)allItems;

- (BOOL)hasItemWithKey:(NSString*)key;

/* Retrieve an item based on its index key */
- (id)itemForKey:(NSString*)key;

- (BOOL)addItem:(id<BBRepositoryItem>)item;

- (void)removeItemWithKey:(NSString*)key;

/* This one MUST be overridden by subclasses */
- (id<BBRepositoryItem>)createItemFromDictionary:(NSDictionary*)dictionary;

/* Defaults to calling -convertToDictionary on the item if the item responds to the optional method */
- (NSDictionary*)convertItemToDictionary:(id<BBRepositoryItem>)item;

/* Defaults to the name of the class; this is merely to build the full path where the files will reside on disk */
- (NSString*)repositoryName;

/* Returns the base storage path. The final path will be built depending on the instance. */
- (NSString*)baseStoragePath;

/* Called after reload succeeds (but before returning YES on -reload). Add your post-reload logic here */
- (void)reloadComplete;

@end
