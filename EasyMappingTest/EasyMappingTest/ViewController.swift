//
//  ViewController.swift
//  EasyMappingTest
//
//  Created by Hiroshi Watanabe on 10/23/16.
//  Copyright © 2016 Hiroshi Watanabe. All rights reserved.
//

import UIKit
import EasyMapping

class Person: EKObjectModel {
    var name : String!
    var email: String!
    var car : Car?
    var phones: [Phone]!
}

class Car: EKObjectModel {
    var model : String!
    var year : String!
    var createdAt : NSDate!
}

class Phone: EKObjectModel {
    var number: String?
}

extension Phone {
    override class func objectMapping() -> EKObjectMapping {
        let mapping = EKObjectMapping(objectClass: self)
        mapping.mapProperties(from: ["number"])
        return mapping
    }
}

extension Person {
    override class func objectMapping() -> EKObjectMapping {
        let mapping = EKObjectMapping(objectClass: self)
        mapping.mapProperties(from: ["name"])
        mapping.mapProperties(from: ["email"])
        mapping.hasOne(Car.self, forKeyPath: "car")
        mapping.hasMany(Phone.self, forKeyPath: "phones")
        return mapping
    }
}

extension Car {
    override class func objectMapping() -> EKObjectMapping {
        let mapping = EKObjectMapping(objectClass: self)
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
        // Do any additional setup after loading the view, typically from a nib.

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

        let p: Person = Person(properties: person)

        print(p.name)
        print(p.email)

        print(p.car?.model)
        print(p.car?.year)
        print(p.car?.createdAt)

        print(p.phones[0].number)
        print(p.phones[1].number)
    }

    override func didReceiveMemoryWarning() {
        super.didReceiveMemoryWarning()
        // Dispose of any resources that can be recreated.
    }


}

