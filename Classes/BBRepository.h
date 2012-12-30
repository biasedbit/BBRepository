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
        #define LogTrace(fmt, ...)  NSLog((@"TRACE: " fmt), ##__VA_ARGS__);
    #elif
        #define LogTrace(fmt, ...)
    #endif
#endif

#ifndef LogDebug
    #define LogDebug(fmt, ...)  NSLog((@"DEBUG: " fmt), ##__VA_ARGS__);
#endif
#ifndef LogInfo
    #define LogInfo(fmt, ...)   NSLog((@" INFO: " fmt), ##__VA_ARGS__);
#endif
#ifndef LogError
    #define LogError(fmt, ...)  NSLog((@"ERROR: " fmt), ##__VA_ARGS__);
#endif



#pragma mark - Constants

/** Default repository identifier */
extern NSString* const kBBRepositoryDefaultIdentifier;



#pragma mark -

/**
 Blueprint for a simple object repository that stores objects to disk using binary property lists.

 You can read all about the perks of this method [in this article](http://biasedbit.com/caf-vs-custom-serialization/).
 
 This is a (way) faster alternative to Core Data for those cases where you don't need all the querying power of Core 
 Data and are more interested in the raw speed of operations.
 
 By default, a repository only permits querying by its primary key, `[BBRepositoryItem key]` but you are free
 (and welcome) to extend it to support indexing/searching/removing by other attributes.
 
 
 ## Reading and persisting data
 
 Unlike Core Data, this repository does not automatically load and/or persist content. It is up to you to
 determine the most appropriate time to load contents from disk (`reload`) and/or persist them (`flush`).
 
 If the repository will be used throughout the app, you should probably do a call to `reload` when the app finishes
 launching and perform a `flush` when it's sent to background or terminated.


 ## Repository location and identification
 
 This class will store the index file, a binary property list, in the `NSApplicationSupportDirectory`. The index file
 will contain `NSDictionary` representations of the managed objects
 

 ## Subclassing notes
 
 Assuming the items that are to be managed by this repository contain back and forth conversion logic in them
 (i.e. they properly implement `[BBRepositoryItem initWithRepositoryDictionary:]` and
 `[BBRepositoryItem convertToRepositoryDictionary]`) the only method you need to override is
 `createItemFromDictionary:`.
 
 All other methods are optional, but you will likely need to add further logic (i.e. override calling `super`). For an
 example, check out the BBCache class and other examples provided.
 
 
 ## Indexing other fields
 
 If you wish to index your records based on other fields, the best way to do it is to override `reloadComplete` and
 populate your own `NSDictionary` there.
 
 For instance, if you were to index a given record based on a combination of `name` and `source` fields, you could do:
 
    - (void)reloadComplete
    {
        // Assumes existence of _otherIndex NSMutableDictionary
        _otherIndex = [NSMutableDictionary dictionary];

        for (id record in _entries) {
            NSString* key = [NSString stringWithFormat:@"%@:%@", record.name, record.source];
            [_otherIndex setObject:record forKey:key];
        }
     }
 
 Of course that in doing so, you'd also have to update this secondary index on record insertion and removal.


 ## Performance considerations
 
 This class (and subclasses) are not meant to handle very large data sets. Use it only if you're sure that you will be
 handling at most a couple thousand entries.
 
 ### Properties of managed objects

 Whatever the properties of your objects are, they will inevitably be converted to basic foundation types:
 
 - `NSString`
 - `NSNumber`
 - `NSArray`
 - `NSDictionary`

 As long as your objects can be converted into these primitives, they can be as complex as you'd like them to be.
 
 ### Circular references

 Circular references are **not** supported. If you really need to store an object graph, please use the `NSCoding`
 protocol instead. While it is slightly slower, it's a richer serialization framework.

 ### Size of managed objects

 Since all the entries in the index file will be loaded to memory, you should strive to keep your managed objects as
 small and simple as possible.
 
 You should never store binary data or very large fields in these objects. What you should do instead is subclass this
 class and make sure the very large fields are stored to their own files and lazily loaded whenever needed. A good
 point to write these files to disk would be when adding objects to the repository (i.e. override `addItem:` and roll in
 the added functionality).

 Assuming you want to store an object that holds a `UIImage` instance, instead of storing the `NSData` for the PNG
 representation of the image in the managed object, you could instead hold the path for a file where that `NSData` is
 stored and lazily load it whenever appropriate. When adding this object to the repository, you'd immediately save the
 image data to disk and when removing it from the repository, you'd delete its companion file.
 
 An image cache is a perfect example of this use case.

 @see BBRepositoryItem
 @see BBCache
 */
@interface BBRepository : NSObject
{
@protected
    NSString* _identifier;
    NSString* _repositoryDirectory;
    NSString* _repositoryIndex;
    NSMutableDictionary* _entries;
}


#pragma mark Creation

///---------------
/// @name Creation
///---------------

/**
 Creates a new repository with a given identifier.

 @param identifier The identifier for this repository.

 @return A newly initialized `BBRepository` instance.
 
 @see identifier
 */
- (id)initWithIdentifier:(NSString*)identifier;

/**
 Creates a new repository, passing "Default" as the identifier.
 
 @return A newly initialized `BBRepository` instance.
 
 @see initWithIdentifier:
 */
- (id)init;


#pragma mark Repository properties

///----------------------------
/// @name Repository properties
///----------------------------

/**
 The identifier for this repository, as provided on creation.
 
 Identifiers serve to distinguish among different versions of the same repository. Two repositories with two different
 identifiers will have a different storage path and index, thus resulting in two completely independent repositories.
 
 @see baseStoragePath
 @see repositoryName
 */
@property(strong, nonatomic, readonly) NSString* identifier;

/**
 Returns the base storage path.

 The index file will be stored to this directory. By default, this returns the first path under the user's
 `NSApplicationSupportDirectory`.

 Subclasses should override this method and return another path if (for instance, caches should use `NSCachesDirectory`.
 This path can also be used to store additional files managed by this repository.

 @return The base storage path for this repository.
 */
- (NSString*)baseStoragePath;

/**
 Name of the repository, used to generate the target repository directory and index filename.

 This method returns the a string with the following format:

     <repository name>-<<identifier>>

 That string will be used to build the repository folder:
 
     <base storage path>/<repository name>/

 It will also be used to create the index file:
 
     <base storage path>/<repository name>/<repository name>-<<identifier>>-Index.plist
 
  Subclasses may override this method to provide a different return value.
 
 @return The name of the repository.
 */
- (NSString*)repositoryName;


#pragma mark Repository lifecycle management

///--------------------------------------
/// @name Repository lifecycle management
///--------------------------------------

/**
 Completely purges all data managed by this repository.

 This method will delete index file on disk and entries in memory.

 @return `YES` if destruction was successful, `NO` otherwise. When this method returns `NO` the app probably needs to
 be reinstalled.
 */
- (BOOL)destroy;

/**
 Reload data from disk

 @return `YES` if entries were successfully reloaded from disk, `NO` otherwise.
 */
- (BOOL)reload;

/**
 Called after reload succeeds, right before returning `YES` on `reload`.

 This method is meant to be overridden by subclasses (no need to call `super` since it's a no-op implementation) that
 need to perform some post reload actions.
 */
- (void)reloadComplete;

/**
 Serialize and flush all entries in memory to disk.

 @return `YES` if entries were successfully flushed to disk, `NO` otherwise.
 */
- (BOOL)flush;


#pragma mark Querying

///---------------
/// @name Querying
///---------------

/**
 Number of managed entries (in-memory) by this repository.

 @return Number of entries.
 */
- (NSUInteger)itemCount;

/**
 Returns a snapshot of all the items present in the repository at the time of calling of this method.

 @return The current repository items.
 */
- (NSArray*)allItems;

/**
 Test whether the repository contains an item with the given key.
 
 @param key The key to test.

 @return `YES` if an item with the given key is present in this repository, `NO` otherwise.
 */
- (BOOL)hasItemWithKey:(NSString*)key;

/**
 Retrieve an item based on its index key
 
 @param key The key of the item to retrieve.
 
 @return The item for the given key, if there is one.
 */
- (id)itemForKey:(NSString*)key;


#pragma mark Modifications

///--------------------
/// @name Modifications
///--------------------

/**
 Adds an item to the repository.
 
 Subclasses should override this method and change its input type to help guarantee type safety.
 
 @param item Item to add to the repository.
 
 @return `YES` if the item was successfully added to the repository, `NO` otherwise. Note that by adding an object to
 the repository there is no guarantee that the object will be persisted to disk. You must explicitly call `flush` when
 deemed appropriate in order to persist this item.
 */
- (BOOL)addItem:(id<BBRepositoryItem>)item;

/**
 Removes an item from the repository.

 Subclasses should override this method and change its input type to help guarantee type safety.

 @param key Key for the item to remove from the repository.
 */
- (void)removeItemWithKey:(NSString*)key;


#pragma mark Item (de-)serialization

///------------------------------
/// @name Item (de-)serialization
///------------------------------

/**
 Creates an instance of a class that implements the `BBRepositoryItem` protocol from an `NSDictionary` containing the
 serialized version of the object.

 This method **must** be overridden by subclasses and is the place to call the item's class
 `initWithRepositoryDictionary:` method.
 
     // assuming CustomItem implements BBRepositoryItem...
     - (CustomItem*)createItemFromDictionary:(NSDictionary*)dictionary
     {
         return [CustomItem alloc] initWithRepositoryDictionary:dictionary];
     }
 
 When subclassing, make sure you change the return type, to improve type safety.
 
 @param dictionary `NSDictionary` instance that contains the serialized fields for an object.
 
 @return A deserialized instance of a class that implements the `BBRepositoryItem` protocol.
 */
- (id<BBRepositoryItem>)createItemFromDictionary:(NSDictionary*)dictionary;

/**
 Convert an item to its `NSDictionary` representation, so that it can be stored to a binary property list (plist).
 
 If this method returns `nil`, the item cannot be converted to a `NSDictionary` representation and thus it will not be
 serialized.

 This method may not need to be overridden, as the default implementation will test the whether the item responds to the
 selector `convertToDictionary` and call it if it does.
 
 Beware that if the item does not respond to `convertToDictionary` selector, this method will return `nil` and cause
 the item **not to be serialized**.
 
 If you subclass this method, make sure you change the input parameter type, in order to add type safety.

 @param item The item to convert.

 @return the `NSDictionary` representation of the item.
 */
- (NSDictionary*)convertItemToDictionary:(id<BBRepositoryItem>)item;


#pragma mark Hooks

///------------
/// @name Hooks
///------------

/**
 Called right before adding a **new** item to the repository index.
 
 Override this method to implement your custom logic (e.g. write a file to disk).
 
 If you return `NO` on this method, the addition will be interrupted.
 
 @param item The item to add.

 @return NO to interrupt addition, YES to proceed.

 @see willReplaceItem:withNewItem:
 */
- (BOOL)willAddNewItem:(id<BBRepositoryItem>)item;

/**
 Called right after adding a new item to the repository index.
 
 @param item The item that was just added.

 @see didReplaceItem:withNewItem:
 */
- (void)didAddNewItem:(id<BBRepositoryItem>)item;

/**
 Called right before replacing an item with a new item.

 If your managed objects contain references to files on disk, you can use `item` and `newItem` to compare the paths
 and, in case they're different, delete the old item.

 If you return `NO` on this method, the replacement will be interrupted.

 @param item The item that will be replaced.
 @param newItem The new item.

 @return NO to interrupt addition, YES to proceed.
 */
- (BOOL)willReplaceItem:(id<BBRepositoryItem>)item withNewItem:(id<BBRepositoryItem>)newItem;

/**
 Called right after replacing an item.
 
 @param item The item that was replaced.
 @param newItem The new item.
 */
- (void)didReplaceItem:(id<BBRepositoryItem>)item withNewItem:(id<BBRepositoryItem>)newItem;

/**
 Called right before removing an item from the repository index.
 
 Unlike adding or replacing, removing cannot be stopped.

 @param item The item that will be removed.
 */
- (void)willRemoveItem:(id<BBRepositoryItem>)item;

/**
 Called right after removing an item from the repository index.

 @param item The item that was just removed.
 */
- (void)didRemoveItem:(id<BBRepositoryItem>)item;

@end
