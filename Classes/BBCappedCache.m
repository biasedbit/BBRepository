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

#import "BBCappedCache.h"



#pragma mark -

@implementation BBCappedCache


#pragma mark Creation

- (instancetype)init
{
    NSAssert(NO, @"Please initialize with initWithIdentifier:itemDuration:resourceUsageLimit:");
    return nil;
}

- (instancetype)initWithIdentifier:(NSString*)identifier
{
    NSAssert(NO, @"Please initialize with initWithIdentifier:itemDuration:resourceUsageLimit:");
    return nil;
}

- (instancetype)initWithIdentifier:(NSString*)identifier itemDuration:(NSTimeInterval)itemDuration
{
    NSAssert(NO, @"Please initialize with initWithIdentifier:itemDuration:resourceUsageLimit:");
    return nil;
}

- (id)initWithIdentifier:(NSString*)identifier resourceUsageLimit:(double)resourceUsageLimit
{
    self = [super initWithIdentifier:identifier];
    if (self != nil) _resourceUsageLimit = resourceUsageLimit;

    return self;
}

- (instancetype)initWithIdentifier:(NSString*)identifier itemDuration:(NSTimeInterval)itemDuration
                resourceUsageLimit:(double)resourceUsageLimit
{
    self = [super initWithIdentifier:identifier itemDuration:itemDuration];
    if (self != nil) _resourceUsageLimit = resourceUsageLimit;

    return self;
}


#pragma mark BBRepository overrides

- (BOOL)addItem:(id<BBCappedCacheItem>)item
{
    double resourceUsageAfterAdding = [self totalResourceUsage] + [item resourceUsage];
    BOOL overCapacity = resourceUsageAfterAdding > _resourceUsageLimit;

    if (overCapacity) [self compact];

    return [super addItem:item];
}


#pragma mark BBCache overrides

- (NSUInteger)compact
{
    // Begin by ejecting stale items...
    NSUInteger deletedItems = [super compact];
    double currentResourceUsage = [self totalResourceUsage];

    // If we're under the resource usage limit, no need to compact further; bail out.
    if (currentResourceUsage <= _resourceUsageLimit) return deletedItems;

    // If we're over the limit, we need to remove items until we fit the limit again.
    // This means sorting items by their expiration date (older ones first) and removing them.
    NSComparator comparator = ^NSComparisonResult(id<BBCappedCacheItem> a, id<BBCappedCacheItem> b) {
        return [[a expirationDate] compare:[b expirationDate]];
    };

    NSArray* sortedItems = [[_entries allValues] sortedArrayUsingComparator:comparator];
    for (id<BBCappedCacheItem> item in sortedItems) {
        deletedItems++;
        currentResourceUsage -= [item resourceUsage];
        [self removeItemWithKey:[item key]];

        // Bail out if at any point we go under the file limit
        if (currentResourceUsage <= _resourceUsageLimit) return deletedItems;
    }

    return deletedItems;
}

#pragma mark ???

- (double)totalResourceUsage
{
    double total = 0;
    for (id<BBCappedCacheItem> item in [_entries allValues]) {
        total += [item resourceUsage];
    }

    return total;
}

@end
