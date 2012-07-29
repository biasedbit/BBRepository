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
    return [self initWithIdentifier:kBBRepositoryDefaultIdentifier andItemDuration:kBBCacheDefaultItemDuration];
}

- (id)initWithIdentifier:(NSString*)identifier andItemDuration:(NSTimeInterval)itemDuration
{
    self = [super initWithIdentifier:identifier];
    if (self != nil) {
        _itemDuration = itemDuration;
    }

    return self;
}


#pragma mark BBRepository overrides

- (id)itemForKey:(NSString*)key
{
    id<BBCacheItem> item = [super itemForKey:key];
    if (item == nil) {
        return nil;
    }

    NSDate* newExpiration = [NSDate dateWithTimeIntervalSinceNow:_itemDuration];
    [item setExpirationDate:newExpiration];
    LogTrace(@"[%@] Extended expiration date for item with key '%@' to %@",
             [self repositoryName], key, newExpiration);

    return item;
}

- (BOOL)addItem:(id<BBCacheItem>)item
{
    if (item == nil) {
        return NO;
    }

    // Only change the expiration date if its nil
    // Allows users to set custom expiration date (only valid until item is touched by itemForKey:)
    if ([item expirationDate] == nil) {
        NSDate* newExpiration = [NSDate dateWithTimeIntervalSinceNow:_itemDuration];
        [item setExpirationDate:newExpiration];
    }

    return [super addItem:item];
}

- (NSString*)baseStoragePath
{
    return [NSSearchPathForDirectoriesInDomains(NSCachesDirectory, NSUserDomainMask, YES) objectAtIndex:0];
}


#pragma mark Public methods

- (NSUInteger)purgeStaleItems
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
            LogTrace(@"[%@] - Destroyed expired item with key '%@'", [self repositoryName], key);

            [keysToRemove addObject:key];
            [self destroyExpiredItem:item];
        }
    }];

    [_entries removeObjectsForKeys:keysToRemove];

    return expiredItemCount;
}

- (void)destroyExpiredItem:(id<BBCacheItem>)item
{
    // add your custom destruction behavior here
}

@end
