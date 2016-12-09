import Foundation

/**
 *  Platform Interface:  swift functions are called from the C side.
 */
var pif = ConvivaClientPlatformInterface()
pif.printLog = printLog;
pif.saveConfig = saveConfig;
pif.loadConfig = loadConfig;
pif.sendHeartbeat = sendHeartbeat;
pif.createTimer = createTimer;
pif.destroyTimer = destroyTimer;

/**
 * Global settings
 */
var settings = ConvivaClientSettings()
settings.customer_key = "b914e177d9f9d20a835cf84915c381affe24f163"
settings.gateway_url = "https://test.url"
settings.heartbeat_interval = 2 * 1000
settings.enable_logging = true

/**
 *  Platform metadata: device and hardware info
 */
let systemInfo = Utsname()
var pm = ConvivaClientPlatformMetadata()
pm.device_type = "Desktop"
pm.device_brand = "Macbook"
pm.device_manufacturer = "Apple"
pm.device_model = "Macbook Pro"
pm.device_version = "2015"
pm.framework_name = "sample_c_lib"
pm.framework_version = "0.1"
pm.operating_system_name = systemInfo.sysname
pm.operating_system_version = systemInfo.release

/**
 * Test player data structure
 */
struct TestPlayer {
    var session: ConvivaClientSession!
    var state: ConvivaClientPlayerState!
    var count: Int!
}

func getPlayheadTime(data: AnyObject?) -> Int
{
    var ret: Int = 0;
    if var player = data as? TestPlayer {
        if player.state == ConvivaClientPlayerState.CCL_PLAYING {
            ret = player.count
            player.count = ret + 200
        }
    }
    return ret
}

func getBufferedTime(data: AnyObject?) -> Int
{
    return 1000
}

func getFramerate(data: AnyObject?) -> Int
{
    return 500
}

var playerIf = ConvivaClientPlayerInterface()
playerIf.getPlayheadTime = getPlayheadTime
playerIf.getBufferedTime = getBufferedTime
playerIf.getFramerate = getFramerate

/**
 *  Initialize the CCL
 */
let ccl = ConvivaClientLibrary(settings: settings, pm: pm, pif: pif)

let group = DispatchGroup()

let NUM_SESSIONS = 1
let NUM_ITERATIONS = 10

for queue_index in 1...NUM_SESSIONS {
    var label: String = "com.conviva.ccl.queue\(queue_index)"
    let queue = DispatchQueue(label: label)
    queue.async(group: group) {
        var cci = ConvivaClientContentInformation()
        cci.asset_name = "Jason Borne"
        cci.player_name = "C Demo Player"
        cci.viewer_id = "1234"
        cci.cdn_name = "other"
        cci.media_resource = "http://tako.com/ika"
        cci.media_url = "http://tako.com/ika"
        cci.media_duration = 1000;
        cci.media_bitrate = 3000;
        cci.media_is_live = false;

        let session = ConvivaClientSession(content: cci)

        var player = TestPlayer()
        player.session = session
        player.state = ConvivaClientPlayerState.CCL_UNKNOWN
        player.count = 0

        session.attach(playerInterface: playerIf, playerData: player as AnyObject)

        let randomNumber = TimeInterval(arc4random_uniform(UInt32(3)))
        Thread.sleep(forTimeInterval: randomNumber)

        for i in 1...NUM_ITERATIONS {
            switch i % 6 {
            case 0: session.playback_state = ConvivaClientPlayerState.CCL_STOPPED
            case 1: session.playback_state = ConvivaClientPlayerState.CCL_PLAYING
            case 2: session.playback_state = ConvivaClientPlayerState.CCL_BUFFERING
            case 3: session.playback_state = ConvivaClientPlayerState.CCL_PAUSED
            case 4: session.playback_state = ConvivaClientPlayerState.CCL_NOTMONITORED
            case 5: session.playback_state = ConvivaClientPlayerState.CCL_UNKNOWN
            default: break
            }
            let randomNumber = TimeInterval(arc4random_uniform(UInt32(3)))
            Thread.sleep(forTimeInterval: randomNumber)
            print("A:\(i)")
        }

        session.detach()

        let randomNumber2 = TimeInterval(arc4random_uniform(UInt32(3)))
        Thread.sleep(forTimeInterval: randomNumber2)

        session.destroy()
    }
}

group.notify(queue: DispatchQueue.main) {
    print("ccl close.")
    ccl.close()
    let runloop = CFRunLoopGetCurrent()
    CFRunLoopStop(runloop)
}

CFRunLoopRun()
