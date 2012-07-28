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

@protocol BBRepositoryItem <NSObject>

/* All repository items must have some sort of primary key that serves as index */
- (NSString*)key;

@optional
/*
 Conversion methods are optional since you may prefer to keep conversion logic outside of the object, such as when
 you want to store a class that has subclasses.

 Example: you want to store Product instances, but there are many Product subclasses, each with its own fields so
 rather than coding all the subclass serialization/deserialization logic into the Product class, or overriding the
 conversion methods and repeating code in each subclass (harder to maintain if anything changes) you can leave this
 logic to the repository.
 */
- (id)initWithRepositoryDictionary:(NSDictionary*)dictionary;
- (NSDictionary*)convertToRepositoryDictionary;

@end
