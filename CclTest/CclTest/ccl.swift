import ccl

enum ConvivaClientPlayerState: Int {
    case CCL_STOPPED               = 1
    case CCL_PLAYING               = 3
    case CCL_BUFFERING             = 6
    case CCL_PAUSED                = 12
    case CCL_NOTMONITORED          = 98
    case CCL_UNKNOWN               = 100
}

struct ConvivaClientSettings {
    var metadata = ccl_create_metadata()

    var customer_key: String {
        get {
            if let cstr = ccl_get_metadata_string(metadata, CUSTOME_KEY) {
                return String.init(cString: UnsafePointer<CChar>(cstr))
            }
            return ""
        }
        set(key) {
            ccl_set_metadata_string(metadata, CUSTOME_KEY, UnsafeMutablePointer(mutating: key))
        }
    }

    var gateway_url: String {
        get {
            if let cstr = ccl_get_metadata_string(metadata, GATEWAY_URL) {
                return String.init(cString: UnsafePointer<CChar>(cstr))
            }
            return ""
        }
        set(key) {
            ccl_set_metadata_string(metadata, GATEWAY_URL, UnsafeMutablePointer(mutating: key))
        }
    }

    var heartbeat_interval: Int {
        get {
            return Int(ccl_get_metadata_int(metadata, HEARTBEAT_INTERVAL))
        }
        set(interval) {
            ccl_set_metadata_int(metadata, HEARTBEAT_INTERVAL, Int32(interval))
        }
    }

    var enable_logging: Bool {
        get {
            return ccl_get_metadata_int(metadata, ENABLE_LOGGING) > 0
        }
        set(logging) {
            let ival = logging ? 1 : 0
            ccl_set_metadata_int(metadata, ENABLE_LOGGING, Int32(ival))
        }
    }
}

struct ConvivaClientPlatformMetadata {
    var metadata = ccl_create_metadata()

    var device_type: String {
        get {
            if let cstr = ccl_get_metadata_string(metadata, DEVICE_TYPE) {
                return String.init(cString: UnsafePointer<CChar>(cstr))
            }
            return ""
        }
        set(key) {
            ccl_set_metadata_string(metadata, DEVICE_TYPE, UnsafeMutablePointer(mutating: key))
        }
    }

    var device_brand: String {
        get {
            if let cstr = ccl_get_metadata_string(metadata, DEVICE_BRAND) {
                return String.init(cString: UnsafePointer<CChar>(cstr))
            }
            return ""
        }
        set(key) {
            ccl_set_metadata_string(metadata, DEVICE_BRAND, UnsafeMutablePointer(mutating: key))
        }
    }

    var device_manufacturer: String {
        get {
            if let cstr = ccl_get_metadata_string(metadata, DEVICE_MANUFACTURER) {
                return String.init(cString: UnsafePointer<CChar>(cstr))
            }
            return ""
        }
        set(key) {
            ccl_set_metadata_string(metadata, DEVICE_MANUFACTURER, UnsafeMutablePointer(mutating: key))
        }
    }

    var device_model: String {
        get {
            if let cstr = ccl_get_metadata_string(metadata, DEVICE_MODEL) {
                return String.init(cString: UnsafePointer<CChar>(cstr))
            }
            return ""
        }
        set(key) {
            ccl_set_metadata_string(metadata, DEVICE_MODEL, UnsafeMutablePointer(mutating: key))
        }
    }

    var device_version: String {
        get {
            if let cstr = ccl_get_metadata_string(metadata, DEVICE_VERSION) {
                return String.init(cString: UnsafePointer<CChar>(cstr))
            }
            return ""
        }
        set(key) {
            ccl_set_metadata_string(metadata, DEVICE_VERSION, UnsafeMutablePointer(mutating: key))
        }
    }

    var framework_name: String {
        get {
            if let cstr = ccl_get_metadata_string(metadata, FRAMEWORK_NAME) {
                return String.init(cString: UnsafePointer<CChar>(cstr))
            }
            return ""
        }
        set(key) {
            ccl_set_metadata_string(metadata, FRAMEWORK_NAME, UnsafeMutablePointer(mutating: key))
        }
    }

    var framework_version: String {
        get {
            if let cstr = ccl_get_metadata_string(metadata, FRAMEWORK_VERSION) {
                return String.init(cString: UnsafePointer<CChar>(cstr))
            }
            return ""
        }
        set(key) {
            ccl_set_metadata_string(metadata, FRAMEWORK_VERSION, UnsafeMutablePointer(mutating: key))
        }
    }

    var operating_system_name: String {
        get {
            if let cstr = ccl_get_metadata_string(metadata, OPERATING_SYSTEM_NAME) {
                return String.init(cString: UnsafePointer<CChar>(cstr))
            }
            return ""
        }
        set(key) {
            ccl_set_metadata_string(metadata, OPERATING_SYSTEM_NAME, UnsafeMutablePointer(mutating: key))
        }
    }

    var operating_system_version: String {
        get {
            if let cstr = ccl_get_metadata_string(metadata, OPERATING_SYSTEM_VERSION) {
                return String.init(cString: UnsafePointer<CChar>(cstr))
            }
            return ""
        }
        set(key) {
            ccl_set_metadata_string(metadata, OPERATING_SYSTEM_VERSION, UnsafeMutablePointer(mutating: key))
        }
    }

}

typealias TimerCallbackFunc = (Any?) -> (Void)

struct ConvivaClientPlatformInterface {
    var printLog        : ((String, Int) -> (Int))?
    var saveConfig      : ((String, Int) -> (Int))?
    var loadConfig      : ((String, Int) -> (Int))?
    var sendHeartbeat   : ((String, String, String, Int) -> (Int))?
    var createTimer     : ((@escaping TimerCallbackFunc, Any?, Int, Int) -> (Any))?
    var destroyTimer    : ((Any?) -> Void)?
}

// FIXME: can be a member of a class???
var printLogImpl        : ((String, Int) -> (Int))?
var saveConfigImpl      : ((String, Int) -> (Int))?
var loadConfigImpl      : ((String, Int) -> (Int))?
var sendHeartbeatImpl   : ((String, String, String, Int) -> (Int))?
var createTimerImpl     : ((@escaping TimerCallbackFunc, Any, Int, Int) -> (Any))?
var destroyTimerImpl    : ((Any?) -> (Void))?

func print_log(str: UnsafePointer<CChar>?, size: UInt32 ) -> Int32
{
    if let printLogFunc = printLogImpl {
        let s: String = String.init(cString: str!)
        return Int32(printLogFunc(s, Int(size)))
    }
    return 0
}

func save_config(buf: UnsafePointer<CChar>?, size: UInt32 ) -> Int32
{
    if let saveConfigFunc = saveConfigImpl {
        let s: String = String.init(cString: buf!)
        return Int32(saveConfigFunc(s, Int(size)))
    }
    return 0
}

func load_config(buf: UnsafeMutablePointer<Int8>?, size: UInt32 ) -> Int32
{
    if let loadConfigFunc = loadConfigImpl {
        let s: String = String.init(cString: buf!)
        return Int32(loadConfigFunc(s, Int(size)))
    }
    return 0
}

func send_heartbeat(url: UnsafePointer<CChar>?, content_type: UnsafePointer<CChar>?,
                    heartbeat: UnsafePointer<CChar>?, heartbeat_size: UInt32 ) -> Int32
{
    if let sendHeartbeatFunc = sendHeartbeatImpl {

        // FIXME ////

        let surl: String = String.init(cString: url!)
        let stype: String = String.init(cString: content_type!)
        let sheartbet: String = String.init(cString: heartbeat!)
        return Int32(sendHeartbeatFunc(surl, stype, sheartbet, Int(heartbeat_size)))
    }
    return 0
}

func create_timer(callback: ccl_timer_callback?, data: UnsafeMutableRawPointer?,
                  initial_time: UInt32, interval: UInt32) -> UnsafeMutableRawPointer?
{
    if let createTimerFunc = createTimerImpl {

        // bridge ccl_timer_callback with TimerCallbackFunc
        let timerCallbackBridge: (Any?) -> Void = {
            (arg: Any?) -> Void in
            if let cb = callback {
                cb(arg as! UnsafeMutableRawPointer?);
            }
        }

        let obj = createTimerFunc(timerCallbackBridge, data as Any,
                                  Int(initial_time), Int(interval))

        return Unmanaged<AnyObject>.passRetained(obj as AnyObject).toOpaque()
    }
    return nil
}

func destroy_timer(timer_handle: UnsafeMutableRawPointer?)
{
    if let destroyTimerFunc = destroyTimerImpl {
        destroyTimerFunc(Unmanaged<AnyObject>.fromOpaque(timer_handle!).takeRetainedValue())
    }
}


struct ConvivaClientPlayerInterface {
    var getPlayheadTime : ((Any?) -> (Int))?
    var getBufferedTime : ((Any?) -> (Int))?
    var getFramerate    : ((Any?) -> (Int))?
}

var getPlayheadTimeImpl : ((Any?) -> (Int))?
var getBufferedTimeImpl : ((Any?) -> (Int))?
var getFramerateImpl    : ((Any?) -> (Int))?

func get_playhead_time(data: UnsafeMutableRawPointer?) -> Int32
{
    if let getPlayheadTimeFunc = getPlayheadTimeImpl {
        return Int32(getPlayheadTimeFunc(Unmanaged<AnyObject>.fromOpaque(data!).takeRetainedValue()))
    }
    return 0
}

func get_buffered_time(data: UnsafeMutableRawPointer?) -> Int32
{
    if let getBufferedTimeFunc = getBufferedTimeImpl {
        return Int32(getBufferedTimeFunc(Unmanaged<AnyObject>.fromOpaque(data!).takeRetainedValue()))
    }
    return 0
}

func get_framerate(data: UnsafeMutableRawPointer?) -> Int32
{
    if let getFramerateFunc =  getFramerateImpl{
        return Int32(getFramerateFunc(Unmanaged<AnyObject>.fromOpaque(data!).takeRetainedValue()))
    }
    return 0
}

class ConvivaClientLibrary {

    init(settings: ConvivaClientSettings, pm: ConvivaClientPlatformMetadata, pif: ConvivaClientPlatformInterface) {

        printLogImpl = pif.printLog
        saveConfigImpl = pif.saveConfig
        loadConfigImpl = pif.loadConfig
        sendHeartbeatImpl = pif.sendHeartbeat
        createTimerImpl = pif.createTimer
        destroyTimerImpl = pif.destroyTimer

        var native_pif = ccl_platform_interface()
        native_pif.print_log = print_log
        native_pif.save_config = save_config
        native_pif.load_config = load_config
        native_pif.send_heartbeat = send_heartbeat
        native_pif.create_timer = create_timer
        native_pif.destroy_timer = destroy_timer

        ccl_init(settings.metadata, pm.metadata, &native_pif)
    }

    func close() {
        ccl_deinit()
    }
}

struct ConvivaClientContentInformation {
    var metadata = ccl_create_metadata()

    var asset_name: String {
        get {
            if let cstr = ccl_get_metadata_string(metadata, ASSET_NAME) {
                return String.init(cString: UnsafePointer<CChar>(cstr))
            }
            return ""
        }
        set(key) {
            ccl_set_metadata_string(metadata, ASSET_NAME, UnsafeMutablePointer(mutating: key))
        }
    }

    var player_name: String {
        get {
            if let cstr = ccl_get_metadata_string(metadata, PLAYER_NAME) {
                return String.init(cString: UnsafePointer<CChar>(cstr))
            }
            return ""
        }
        set(key) {
            ccl_set_metadata_string(metadata, PLAYER_NAME, UnsafeMutablePointer(mutating: key))
        }
    }

    var viewer_id: String {
        get {
            if let cstr = ccl_get_metadata_string(metadata, VIEWER_ID) {
                return String.init(cString: UnsafePointer<CChar>(cstr))
            }
            return ""
        }
        set(key) {
            ccl_set_metadata_string(metadata, VIEWER_ID, UnsafeMutablePointer(mutating: key))
        }
    }

    var cdn_name: String {
        get {
            if let cstr = ccl_get_metadata_string(metadata, CDN_NAME) {
                return String.init(cString: UnsafePointer<CChar>(cstr))
            }
            return ""
        }
        set(key) {
            ccl_set_metadata_string(metadata, CDN_NAME, UnsafeMutablePointer(mutating: key))
        }
    }

    var media_resource: String {
        get {
            if let cstr = ccl_get_metadata_string(metadata, MEDIA_RESOURCE) {
                return String.init(cString: UnsafePointer<CChar>(cstr))
            }
            return ""
        }
        set(key) {
            ccl_set_metadata_string(metadata, MEDIA_RESOURCE, UnsafeMutablePointer(mutating: key))
        }
    }

    var media_url: String {
        get {
            if let cstr = ccl_get_metadata_string(metadata, MEDIA_URL) {
                return String.init(cString: UnsafePointer<CChar>(cstr))
            }
            return ""
        }
        set(key) {
            ccl_set_metadata_string(metadata, MEDIA_URL, UnsafeMutablePointer(mutating: key))
        }
    }

    var media_duration: Int {
        get {
            return Int(ccl_get_metadata_int(metadata, MEDIA_DURATION))
        }
        set(interval) {
            ccl_set_metadata_int(metadata, MEDIA_DURATION, Int32(interval))
        }
    }

    var media_bitrate: Int {
        get {
            return Int(ccl_get_metadata_int(metadata, MEDIA_BITRATE))
        }
        set(interval) {
            ccl_set_metadata_int(metadata, MEDIA_BITRATE, Int32(interval))
        }
    }

    var media_is_live: Bool {
        get {
            return ccl_get_metadata_int(metadata, MEDIA_IS_LIVE) > 0
        }
        set(islive) {
            let ival = islive ? 1 : 0
            ccl_set_metadata_int(metadata, MEDIA_IS_LIVE, Int32(ival))
        }
    }
}

class ConvivaClientSession {
    var session: OpaquePointer!

    init(content: ConvivaClientContentInformation) {
        session = ccl_session_create(content.metadata, nil)
    }

    func attach(pif: ConvivaClientPlayerInterface, player: Any) {
        getPlayheadTimeImpl = pif.getPlayheadTime
        getBufferedTimeImpl = pif.getBufferedTime
        getFramerateImpl = pif.getFramerate

        var pif = ccl_player()
        pif.get_playhead_time = get_playhead_time
        pif.get_buffered_time =  get_buffered_time
        pif.get_framerate = get_framerate

        ccl_session_attach(session, &pif,
                           Unmanaged<AnyObject>.passRetained(player as AnyObject).toOpaque())
    }

    func detach() {
        ccl_session_detach(session);
    }

    func destroy() {
        ccl_session_destroy(session);
    }
}
