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
    let randomNumber = TimeInterval(arc4random_uniform(UInt32(3)))
    Thread.sleep(forTimeInterval: randomNumber)
    print("\(heartbeat)")
    return 0;
}

class TimerWrapper {
    var timer: Timer!
    var callback: TimerCallbackFunc?
    var data: Any? // session pointer from C
    var delay: Int? // milliseconds
    var interval: Int? // milliseconds
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
        var defaultInterval: Double = 20.0  // in second
        if let i = interval {
            if i > 1000 && i != 20000 {
                defaultInterval = Double(i) / 1000.0;
            }
        }
        timer = Timer.scheduledTimer(timeInterval: defaultInterval, target: self,
                                     selector: #selector(self.update), userInfo: nil, repeats: true)
        timer.fire()
    }

    func done() {
        task.cancel()
        if let t = timer {
            t.invalidate()
        }
    }

    @objc func update(tm: Timer) {
        if let _callback = callback, let _data = data {
            _callback(_data)
        }
    }
}

func createTimer(callback: @escaping TimerCallbackFunc, data:Any, delay:Int, interval:Int) -> Any
{
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
    if let tw = handle as? TimerWrapper {
        tw.done()
    }
}
