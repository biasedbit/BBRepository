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

#pragma mark -

/**
 A repository item is an instance of a class that has a couple of properties:

 - Must have a unique identifier (primary key);
 - Needs be serializable to foundation type objects;
 - Needs to be deserializable from foundation objects.
 

 ## Serialization and de-serialization

 Conversion methods (`initWithRepositoryDictionary:` and `convertToRepositoryDictionary`) are optional since you may
 prefer to keep conversion logic outside of the object, such as when you want to store a class that has subclasses and
 it doesn't make sense for the superclass to know its subclasses and/or how to (de)serialize them.

 An example of such case would be storing `Product` instances. Assuming there are many `Product` subclasses,
 each with its own fields, rather than coding all the subclass serialization/deserialization logic into the `Product`
 superclass or overriding the conversion methods and repeating code in each subclass (harder to maintain if anything
 changes) you can leave this logic to the repository or some other external facility.
 
 @see BBRepository
 */
@protocol BBRepositoryItem <NSObject>


#pragma mark Identification

///---------------------
/// @name Identification
///---------------------

/**
 A unique key that identifies a repository object.

 All repository items must have some sort of primary key that serves as index.
 
 @return Repository item key.
 */
@required
- (NSString*)key;


#pragma mark (De-)Serialization

///-------------------------
/// @name (De-)Serialization
///-------------------------

/**
 Convert an `NSDictionary` representation of an instance of the class implementing this protocol to an actual instance.

 @param dictionary The dictionary to be converted.
 
 @return A newly initialized instance of the class implementing this protocol.
 */
@optional
- (id)initWithRepositoryDictionary:(NSDictionary*)dictionary;

/**
 Convert the instance of the class implementing this protocol to its `NSDictionary` representation.
 
 @return The `NSDictionary` representation of an instance of the class implementing this protocol.
 */
@optional
- (NSDictionary*)convertToRepositoryDictionary;

@end
