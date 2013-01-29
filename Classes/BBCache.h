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

#import "BBRepository.h"
#import "BBCacheItem.h"



#pragma mark - Constants

/** Default item duration, set to one week. */
extern NSTimeInterval const kBBCacheDefaultItemDuration;



#pragma mark -

/**
 Special purpose implementation of a `BBRepository` that handles expiring items.
 
 When items are added to this cache, their expiration date is set to a date in the future. When that expiration date
 elapses, items are considered outdated and may be purged.

 Every time that an item is retrieved, this repository automatically "touches" its `expirationDate`, setting it farther
 into the future (exactly `itemDuration` seconds into the future) thus naturally keeping frequently used items from
 becoming outdated -- and eligible for purging.

 
 ## Purging items
 
 Purging expired items is **not** an automatic step. It is up to you to control when the items are purged (by calling
 `purgeStaleItems`). A good time to do this is when the app is sent to background or at any point in the app's
 lifecycle where the repository is no longer (or will become) unnecessary, such as when exiting an area of the app that
 makes use of it.

 @see BBCacheItem
 @see BBRepository
 */
@interface BBCache : BBRepository
{
@protected
    NSTimeInterval _itemDuration;
}


#pragma mark Creation

///---------------
/// @name Creation
///---------------

/**
 Creates a new repository with a given identifier and item duration.

 @param identifier The identifier for this cache.
 @param itemDuration The duration of items added to this cache.

 @return A newly initialized `BBCache` instance.

 @see [BBRepository identifier]
 @see itemDuration
 */
- (id)initWithIdentifier:(NSString*)identifier andItemDuration:(NSTimeInterval)itemDuration;


#pragma mark Cache properties

///-----------------------
/// @name Cache properties
///-----------------------

/**
 Duration, in seconds, of the items in this cache.
 
 When items are added to this repository, their expiration date will be set to the current instant plus `itemDuration`
 seconds.
 */
@property(assign, nonatomic, readonly) NSTimeInterval itemDuration;


#pragma mark BBRepository overrides

///-----------------------------
/// @name BBRepository overrides
///-----------------------------

/**
 Adds an item to the repository.
 
 If the object doesn't have its `expirationDate` property set, it will be set to the current instant plus `itemDuration`
 seconds.

 Subclasses should override this method and change its input type to help guarantee type safety.

 @param item Item to add to the repository.

 @return `YES` if the item was successfully added to the repository, `NO` otherwise.
 
 @see [BBRepository addItem:]
 */
- (BOOL)addItem:(id<BBCacheItem>)item;


#pragma mark Item expiration

///----------------------
/// @name Item expiration
///----------------------

/**
 Destroy all items in this cache whose `expirationDate` property is inferior to the moment when this method is called.
 
 Each expired item will be removed with the method `removeItemWithKey:`.
 
 @return Number of purged items.
 
 @see itemDeletedByCompaction:
 */
- (NSUInteger)compact;

@end
