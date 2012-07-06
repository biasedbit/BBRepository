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

#import "BBRepository.h"



#pragma mark - Constants

NSString* const kBBRepositoryDefaultIdentifier = @"Default";



#pragma mark -

@implementation BBRepository
{
    dispatch_once_t _repositoryNameOnceToken;
    __strong NSString* _repositoryName;
}


#pragma mark Creation

- (id)initWithIdentifier:(NSString*)identifier
{
    self = [super init];
    if (self != nil) {
        _identifier = identifier;

        NSString* basePath = [self baseStoragePath];
        NSString* repositoryName = [self repositoryName];
        NSString* indexFileName = [NSString stringWithFormat:@"%@-%@-Index.plist", _identifier, repositoryName];

        _repositoryDirectory = [basePath stringByAppendingPathComponent:repositoryName];
        _repositoryIndex = [_repositoryDirectory stringByAppendingPathComponent:indexFileName];
    }

    return self;
}

- (id)init
{
    return [self initWithIdentifier:kBBRepositoryDefaultIdentifier];
}


#pragma mark Public methods

- (BOOL)reset
{
    _entries = [NSMutableDictionary dictionary];
    [[NSFileManager defaultManager] removeItemAtPath:_repositoryDirectory error:nil];

    return YES;
}

- (BOOL)reload
{
    NSError* error = nil;

    // Make sure the directory exists
    if (![[NSFileManager defaultManager]
          createDirectoryAtPath:_repositoryDirectory withIntermediateDirectories:YES attributes:nil error:&error]) {
        LogError(@"[%@] Failed to ensure repository directory exists: %@",
                 [self repositoryName], [error localizedDescription]);
        if (_entries == nil) {
            _entries = [NSMutableDictionary dictionary];
        }
        return NO;
    }

    // Load the file as NSData
    NSData* dictionaryData = [NSData dataWithContentsOfFile:_repositoryIndex];
    if (dictionaryData == nil) {
        LogTrace(@"[%@] Could not read index file; creating empty repository.", [self repositoryName]);
        if (_entries == nil) {
            _entries = [NSMutableDictionary dictionary];
        }
        return NO;
    }

    // Deserialize the contents of the file to an NSDictionary
    NSString* errorDescription = nil;
    NSDictionary* entriesAsDictionaries = [NSPropertyListSerialization
                                           propertyListFromData:dictionaryData
                                           mutabilityOption:NSPropertyListImmutable
                                           format:NULL errorDescription:&errorDescription];

    if (errorDescription != nil) {
        LogError(@"[%@] Data read from index file but de-serialization failed: %@", [self repositoryName], error);
        if (_entries == nil) {
            _entries = [NSMutableDictionary dictionary];
        }
        return NO;
    }

    NSMutableDictionary* entries = [NSMutableDictionary dictionaryWithCapacity:[entriesAsDictionaries count]];
    // Convert each key-value pair (NSString, NSDictionary) into entries
    [entriesAsDictionaries enumerateKeysAndObjectsUsingBlock:^(NSString* key, NSDictionary* dictionary, BOOL* stop) {
        id<BBRepositoryItem> item = [self createItemFromDictionary:dictionary];
        if (item != nil) {
            [entries setObject:item forKey:[item key]];
        }
    }];

    // "atomic" change
    _entries = entries;

    // Allow subclasses to perform some logic right after we've finished reloading data from disk
    [self reloadComplete];

    LogDebug(@"[%@] Deserialized %u items from %u entries index file.",
             [self repositoryName], [_entries count], [entriesAsDictionaries count]);

    return YES;
}

- (BOOL)flush
{
    // Grab a snapshot to avoid the need for synchronization
    NSDictionary* snapshot = [NSDictionary dictionaryWithDictionary:_entries];

    NSError* error = nil;
    NSMutableDictionary* itemsAsDictionaries = [NSMutableDictionary dictionaryWithCapacity:[snapshot count]];
    [snapshot enumerateKeysAndObjectsUsingBlock:^(NSString* key, id<BBRepositoryItem> item, BOOL* stop) {
        NSDictionary* itemAsDictionary = [self convertItemToDictionary:item];
        if (itemAsDictionary != nil) {
            [itemsAsDictionaries setObject:itemAsDictionary forKey:key];
        }
    }];

    // Create NSData from the dictionary created above, by serializing using binary property lists.
    NSData* dictionaryData = [NSPropertyListSerialization
                              dataWithPropertyList:itemsAsDictionaries
                              format:NSPropertyListBinaryFormat_v1_0
                              options:0 error:&error];
    if (error != nil) {
        LogError(@"[%@] Failed to serialize index to binary format: %@",
                 [self repositoryName], [error localizedDescription]);
        return NO;
    }

    if (![dictionaryData writeToFile:_repositoryIndex options:NSDataWritingAtomic error:&error]) {
        LogError(@"[%@] Failed to write index file to disk while flushing: %@",
                 [self repositoryName], [error localizedDescription]);
        return NO;
    }

    LogDebug(@"[%@] Serialized %u entries to %u binary format and wrote to disk.",
             [self repositoryName], [snapshot count], [itemsAsDictionaries count]);

    return YES;
}

- (NSUInteger)itemCount
{
    return [_entries count];
}

- (id)itemForKey:(NSString*)key
{
    return [_entries objectForKey:key];
}

- (BOOL)addItem:(id<BBRepositoryItem>)item
{
    [_entries setObject:item forKey:[item key]];

    return YES;
}

- (void)removeItemWithKey:(NSString*)key
{
    [_entries removeObjectForKey:key];
}

- (id<BBRepositoryItem>)createItemFromDictionary:(NSDictionary*)dictionary
{
    return nil;
}

- (NSDictionary*)convertItemToDictionary:(id<BBRepositoryItem>)item
{
    if ([item respondsToSelector:@selector(convertToDictionary)]) {
        return [item convertToDictionary];
    }

    return nil;
}

- (NSString*)repositoryName
{
    // Rather than computing the name every time, just do it once...
    dispatch_once(&_repositoryNameOnceToken, ^{
        _repositoryName = [NSString stringWithFormat:@"%@-%@", NSStringFromClass([self class]), _identifier];
    });

    return _repositoryName;
}

- (NSString*)baseStoragePath
{
    return [NSSearchPathForDirectoriesInDomains(NSApplicationSupportDirectory, NSUserDomainMask, YES) objectAtIndex:0];
}

- (void)reloadComplete
{
    // to be overridden by subclasses and add custom behavior
}

@end
