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

#import "BBCache.h"



#pragma mark - Constants

NSTimeInterval const kBBCacheDefaultItemDuration = 604800; // 1 week



#pragma mark -

@implementation BBCache


#pragma mark Creation

- (id)init
{
    return [self initWithIdentifier:kBBRepositoryDefaultIdentifier andItemDuration:kBBCacheDefaultItemDuration];
}

- (id)initWithIdentifier:(NSString*)identifier
{
    return [self initWithIdentifier:identifier andItemDuration:kBBCacheDefaultItemDuration];
}

- (id)initWithIdentifier:(NSString*)identifier andItemDuration:(NSTimeInterval)itemDuration
{
    self = [super initWithIdentifier:identifier];
    if (self != nil) _itemDuration = itemDuration;

    return self;
}


#pragma mark BBRepository overrides

- (id)itemForKey:(NSString*)key
{
    id<BBCacheItem> item = [super itemForKey:key];
    if (item == nil) return nil;

    [self touchItem:item];

    return item;
}

- (BOOL)addItem:(id<BBCacheItem>)item
{
    if (item == nil) return NO;

    // Only change the expiration date if it's not nil; allows us to set custom expiration date
    if ([item expirationDate] == nil) [self touchItem:item];

    return [super addItem:item];
}

- (NSString*)baseStoragePath
{
    return [NSSearchPathForDirectoriesInDomains(NSCachesDirectory, NSUserDomainMask, YES) objectAtIndex:0];
}


#pragma mark Interface

- (NSUInteger)compact
{
    NSDate* now = [NSDate date];

    LogTrace(@"[%@] Purging stale items (expiry date inferior to %@)...", [self repositoryName], now);

    NSMutableArray* keysToRemove = [NSMutableArray array];
    __block NSUInteger expiredItemCount = 0;
    [_entries enumerateKeysAndObjectsUsingBlock:^(NSString* key, id<BBCacheItem> item, BOOL* stop) {
        // If date is < to staleThreshold, then purge this item
        NSDate* itemExpirationDate = [item expirationDate];
        if ([[now earlierDate:itemExpirationDate] isEqualToDate:itemExpirationDate]) {
            expiredItemCount++;
            LogTrace(@"[%@] - Will remove expired item with key '%@'", [self repositoryName], key);

            [keysToRemove addObject:key];
        }
    }];

    for (NSString* key in keysToRemove) {
        [self removeItemWithKey:key];
    }

    return expiredItemCount;
}


#pragma mark Private helpers

- (void)touchItem:(id<BBCacheItem>)item
{
    NSDate* newExpiration = [NSDate dateWithTimeIntervalSinceNow:_itemDuration];
    [item setExpirationDate:newExpiration];
}

@end
