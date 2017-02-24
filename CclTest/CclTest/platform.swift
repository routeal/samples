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

// convert json string to dictionary
func convertToDictionary(text: String) -> [String: Any]? {
    if let data = text.data(using: .utf8) {
        do {
            return try JSONSerialization.jsonObject(with: data, options: []) as? [String: Any]
        } catch {
            print(error.localizedDescription)
        }
    }
    return nil
}

func receiveResponse(id: Int)
{
    //print("receiveResponse: \(id)")

    let dic: [String: Any] = [
      "sid": id,
      "t": "CwsServerResponse",
      "err": "ok",
      "clid": "1976252.6667240.1322884.12345678",
    ]

    do {
        let jsonData = try JSONSerialization.data(withJSONObject: dic, options: [])
        if let json = String(bytes: jsonData, encoding: String.Encoding.utf8) {
            parseResponse(response: json)
        }
    } catch {
        print(error.localizedDescription)
    }
}

func sendHeartbeat(url:String, type:String, heartbeat:String, size:Int) -> Int
{
    print("\(heartbeat)")

    if let dict = convertToDictionary(text: heartbeat), let sid = dict["sid"] {
        let randomNumber2 = TimeInterval(arc4random_uniform(UInt32(777)))

        DispatchQueue.main.asyncAfter(deadline: DispatchTime.now() + (randomNumber2 / 1000)) {
            receiveResponse(id: sid as! Int)
        }
    }

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
        if let t = timer {
            t.invalidate()
        }
        task.cancel()
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
