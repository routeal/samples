//
//  ViewController.swift
//  MagicalRecordEasyMapping
//
//  Created by Hiroshi Watanabe on 10/27/16.
//  Copyright © 2016 Hiroshi Watanabe. All rights reserved.
//

import UIKit
import CoreData
import EasyMapping

@objc(Person)
class Person: EKManagedObjectModel {
    @NSManaged var name : String!
    @NSManaged var email: String!
    @NSManaged var car : Car?

    /** NOTE:

     Has to be Set for hasMany relationship.

     **/
    @NSManaged var phones: Set<Phone>?
}

@objc(Car)
class Car: EKManagedObjectModel {
    @NSManaged var model : String!
    @NSManaged var year : String!
    @NSManaged var createdAt : NSDate!
}

@objc(Phone)
class Phone: EKManagedObjectModel {
    @NSManaged var number: String?
}

extension Phone {
    override class func objectMapping() -> EKManagedObjectMapping {
        let mapping = EKManagedObjectMapping(entityName: "Phone")
        mapping.mapProperties(from: ["number"])
        return mapping
    }
}

extension Person {
    override class func objectMapping() -> EKManagedObjectMapping {
        let mapping = EKManagedObjectMapping(entityName: "Person")
        mapping.mapProperties(from: ["name"])
        mapping.mapProperties(from: ["email"])
        mapping.hasOne(Car.self, forKeyPath: "car")
        mapping.hasMany(Phone.self, forKeyPath: "phones")
        return mapping
    }
}

extension Car {
    override class func objectMapping() -> EKManagedObjectMapping {
        let mapping = EKManagedObjectMapping(entityName: "Car")
        mapping.mapProperties(from: ["model"])
        mapping.mapProperties(from: ["year"])
        let formatter: DateFormatter = DateFormatter()
        formatter.timeZone = NSTimeZone(forSecondsFromGMT: 0) as TimeZone!
        formatter.dateFormat = "yyyy-MM-dd"
        mapping.mapKeyPath("createdAt", toProperty: "createdAt", with: formatter)
        return mapping
    }
}

class ViewController: UIViewController {

    override func viewDidLoad() {
        super.viewDidLoad()

        let phone1: [String:AnyObject] = [
          "number" : "408-123-8765" as AnyObject,
        ]

        let phone2: [String:AnyObject] = [
          "number" : "650-987-4321" as AnyObject,
        ]

        let phones = [phone1, phone2]

        let car: [String:AnyObject] = [
          "model" : "acura" as AnyObject,
          "year"  : "1990" as AnyObject,
          "createdAt" : "2007-01-12" as AnyObject
        ]

        let person: [String:AnyObject] = [
          "name" : "hiroshi" as AnyObject,
          "email" : "nabe@live.com" as AnyObject,
          "car" : car as AnyObject,
          "phones" : phones  as AnyObject
        ]

        // reading from the JSON array
        let p = Person.object(withProperties: person, in: NSManagedObjectContext.mr_default())

        print(p.name)
        print(p.email)

        print(p.car?.model)
        print(p.car?.year)
        print(p.car?.createdAt)

        for phone in p.phones! {
            print(phone.number!);
        }

        // saving to the CoreData
        p.managedObjectContext!.mr_saveToPersistentStoreAndWait()


        // reading from the CoreData
        if let obj = Person.mr_findAll() {
            if obj.count > 0 {
                let po = obj[0] as! Person
                print(po.name)
                print(po.email)
                print(po.car?.model)
                print(po.car?.year)
                print(po.car?.createdAt)
                for phone in po.phones! {
                    print(phone.number!);
                }
            }
        }
    }

    override func didReceiveMemoryWarning() {
        super.didReceiveMemoryWarning()
        // Dispose of any resources that can be recreated.
    }


}

