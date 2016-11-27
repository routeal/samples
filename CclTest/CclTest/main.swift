//
//  main.swift
//  CclTest
//
//  Created by Hiroshi Watanabe on 11/8/16.
//  Copyright © 2016 Hiroshi Watanabe. All rights reserved.
//

import Foundation

print("Hello, World!")

var settings = Settings()
settings.customer_key = "b914e177d9f9d20a835cf84915c381affe24f163"
settings.gateway_url = ""
settings.heartbeat_interval = 20 * 1000
settings.enable_logging = true

var pm = PlatformMetadata()
pm.device_type = "Desktop"
pm.device_brand = "Macbook"
pm.device_manufacturer = "Apple"
pm.device_model = "Macbook Pro"
pm.device_version = "123"
pm.framework_name = "tbd"
pm.framework_version = "1.0"
pm.operating_system_name = "macos"
pm.operating_system_version = "10.1"

func printLog(str: String, size: Int) -> Int
{
    print("\(str)")
    return 0
}

func saveConfig(str: String, size: Int) -> Int
{
    return 0
}

func loadConfig(str:String, size: Int) -> Int
{
    return 0
}

func sendHeartbeat(url:String, type:String, heartbeat:String, size:Int) -> Int
{
    return 0;
}

class TimerClass {
    var timer: Timer!
    var cb: ((UnsafeMutableRawPointer?)->Void)?
    var data: UnsafeMutableRawPointer?
    var delay: Int?
    var interval: Int?

    init() {}

    func start() {
        timer = Timer.scheduledTimer(timeInterval: 1.0, target: self,
                                     selector: #selector(self.update), userInfo: nil, repeats: true)
        timer.fire()
    }

    func done() {
        timer.invalidate()
    }

    @objc func update(tm: Timer) {
        if let f = cb {
            print("kita")
            f(data!)
        }
    }
}

var tc = TimerClass()

func createTimer(cb: @escaping ((UnsafeMutableRawPointer?)->Void), arg:UnsafeMutableRawPointer?, delay:Int, interval:Int) -> AnyObject
{
    tc.cb = cb
    tc.data = arg
    tc.delay = delay
    tc.interval = interval
    tc.start()
    return tc as AnyObject
}

func destroyTimer(handle: UnsafeMutableRawPointer?) -> Void
{
    tc.done()
}

var pif = PlatformInterface()
pif.printLog = printLog;
pif.saveConfig = saveConfig;
pif.loadConfig = loadConfig;
pif.sendHeartbeat = sendHeartbeat;
pif.createTimer = createTimer;
pif.destroyTimer = destroyTimer;

var ccl = ConvivaClientLibrary(settings: settings, pm: pm, pif: pif)

print(settings.customer_key)

//ccl.set(settings);

CFRunLoopRun()
