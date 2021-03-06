## MagicalRecordEasyMapping

> test for MagicalRecord and EasyMapping, which allow to convert a JSON
> string to the corresponding data structure at the same time saving
> into the database.

> MagicalRecord will save/retrieve data via CoreData to sqlite.

### 1. Install EasyMapping and MagicalRecord

### 2. Add a Core Data

Add a new Core Data file from File/New... in Xcode, or can be added
when creating a new project.

### 3. Add MagicalRecord init/cleanup to AppDelegate.swift

The db filename is [package name].sqlite, "MagicalRecordEasyMapping.sqlite"

### 4. Design data model

Click "Model.xcdatamodelId" in Xcodde, the model designer shows up.  In the relationships, one-to-one, one-to-many, can be defined.   Also, it is important to define the Class name in the Entity pane in the right side.  Without the Class name, EasyMapping will fail.

Or, the contents file in Model.xcdatamodelId can be directly edited.

```xml
<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<model type="com.apple.IDECoreDataModeler.DataModel" documentVersion="1.0" lastSavedToolsVersion="11232" systemVersion="15F34" minimumToolsVersion="Automatic" sourceLanguage="Swift" userDefinedModelVersionIdentifier="">
    <entity name="Car" representedClassName="Car" syncable="YES">
        <attribute name="createdAt" optional="YES" attributeType="Date" usesScalarValueType="NO" syncable="YES"/>
        <attribute name="model" optional="YES" attributeType="String" syncable="YES"/>
        <attribute name="year" optional="YES" attributeType="String" syncable="YES"/>
    </entity>
    <entity name="Person" representedClassName="Person" syncable="YES">
        <attribute name="email" optional="YES" attributeType="String" syncable="YES"/>
        <attribute name="name" optional="YES" attributeType="String" syncable="YES"/>
        <relationship name="car" optional="YES" maxCount="1" deletionRule="Nullify" destinationEntity="Car" syncable="YES"/>
        <relationship name="phones" optional="YES" toMany="YES" deletionRule="Nullify" destinationEntity="Phone" syncable="YES"/>
    </entity>
    <entity name="Phone" representedClassName="Phone" syncable="YES">
        <attribute name="number" optional="YES" attributeType="String" syncable="YES"/>
    </entity>
    <elements>
        <element name="Person" positionX="-63" positionY="-18" width="128" height="103"/>
        <element name="Car" positionX="223" positionY="27" width="128" height="88"/>
        <element name="Phone" positionX="142" positionY="201" width="128" height="58"/>
    </elements>
</model>
```

representedClassName has to be specified for EasyMapping.

In the code,

```swift
@objc(Person)
class Person: EKManagedObjectModel {
    @NSManaged var name : String!
    @NSManaged var email: String!
    @NSManaged var car : Car?
    @NSManaged var phones: Set<Phone>?
}
```

@objc(Persion) tag is needed since EasyMapping is implemented in
Object-C.  Also, "has-many" has to be "Set", not Array.

### 5. Verify the json data can be read and written through EKManagedObjectMapping

### 6. Save and retrieve data from CoreData with MagicalRecord
