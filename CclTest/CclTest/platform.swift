import Foundation

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
    print("\(heartbeat)")
    return 0;
}

class TimerWrapper {
    var timer: Timer!
    var callback: TimerCallbackFunc?
    var data: Any?
    var delay: Int?
    var interval: Int?
    var task: DispatchWorkItem!

    init() {
        task = DispatchWorkItem {
            self.startTimer()
        }
    }

    func start() {
        if let d = delay {
            let delaySeconds = Double(d) / 1000.0;
            DispatchQueue.main.asyncAfter(deadline: DispatchTime.now() + delaySeconds, execute: task)
        } else {
            startTimer()
        }
    }

    func startTimer() {
        var defaultInterval: Double = 20.0
        if let i = interval {
            if i > 1000 {
                defaultInterval = Double(i) / 1000.0;
            }
        }
        timer = Timer.scheduledTimer(timeInterval: defaultInterval, target: self,
                                     selector: #selector(self.update), userInfo: nil, repeats: true)
        timer.fire()
    }

    func done() {
        print("timer done")
        task.cancel()
        if let t = timer {
            t.invalidate()
        }
    }

    @objc func update(tm: Timer) {
        if let f = callback {
            print("kita")
            f(data)
        }
    }
}

func createTimer(callback: @escaping TimerCallbackFunc, data:Any?, delay:Int, interval:Int) -> Any
{
    print("createTimer")
    let tw = TimerWrapper()
    tw.callback = callback
    tw.data = data
    tw.delay = delay
    tw.interval = interval
    tw.start()
    return tw as Any
}

func destroyTimer(handle: Any?) -> Void
{
    print("destroyTimer")
    if let tw = handle as? TimerWrapper {
        print("destroyTimer 2")
        tw.done()
    }
}
