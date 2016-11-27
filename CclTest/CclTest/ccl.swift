import ccl

struct Settings {
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


struct PlatformMetadata {
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

struct PlatformInterface {

    var printLog: ((String, Int) -> (Int))?

    var saveConfig: ((String, Int) -> (Int))?

    var loadConfig: ((String, Int) -> (Int))?

    var sendHeartbeat: ((String, String, String, Int) -> (Int))?

    var createTimer: ((((UnsafeMutableRawPointer?)->Void), UnsafeMutableRawPointer?, Int, Int) -> AnyObject)?

    var destroyTimer: ((UnsafeMutableRawPointer?) -> Void)?
}

// FIXME: can be a member of a class???
var printLog: ((String, Int) -> (Int))?
var saveConfig: ((String, Int) -> (Int))?
var loadConfig: ((String, Int) -> (Int))?
var sendHeartbeat: ((String, String, String, Int) -> (Int))?
var createTimer: ((((UnsafeMutableRawPointer?)->Void), UnsafeMutableRawPointer?, Int, Int) -> AnyObject)?
var destroyTimer: ((UnsafeMutableRawPointer?) -> Void)?

// FIXME: can be a member of a class???
func print_log(str: UnsafePointer<CChar>?, size: UInt32 ) -> Int32
{
    if let printLogFunc = printLog {
        let s: String = String.init(cString: str!)
        return Int32(printLogFunc(s, Int(size)))
    }
    return 0
}

func save_config(buf: UnsafePointer<CChar>?, size: UInt32 ) -> Int32
{
    if let saveConfigFunc = saveConfig {
        let s: String = String.init(cString: buf!)
        return Int32(saveConfigFunc(s, Int(size)))
    }
    return 0
}

func load_config(buf: UnsafeMutablePointer<Int8>?, size: UInt32 ) -> Int32
{
    if let loadConfigFunc = loadConfig {
        let s: String = String.init(cString: buf!)
        return Int32(loadConfigFunc(s, Int(size)))
    }
    return 0
}

func send_heartbeat(url: UnsafePointer<CChar>?, content_type: UnsafePointer<CChar>?,
                    heartbeat: UnsafePointer<CChar>?, heartbeat_size: UInt32 ) -> Int32
{
    if let sendHeartbeatFunc = sendHeartbeat {

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
    if let createTimerFunc = createTimer {
        // FIXME: callback is optional
        let obj = createTimerFunc(callback!, data, Int(initial_time), Int(interval))
        return Unmanaged.passRetained(obj).toOpaque()
    }
    return nil
}

func destroy_timer(timer_handle: UnsafeMutableRawPointer?)
{
    if let destroyTimerFunc = destroyTimer {
        destroyTimerFunc(timer_handle)
    }
}

// has to be global since the C library keeps the reference
var native_pif = ccl_platform_interface()

class ConvivaClientLibrary {

    init(settings: Settings, pm: PlatformMetadata, pif: PlatformInterface) {

        printLog = pif.printLog
        saveConfig = pif.saveConfig
        loadConfig = pif.loadConfig
        sendHeartbeat = pif.sendHeartbeat
        createTimer = pif.createTimer
        destroyTimer = pif.destroyTimer

        native_pif.print_log = print_log
        native_pif.save_config = save_config
        native_pif.load_config = load_config
        native_pif.send_heartbeat = send_heartbeat
        native_pif.create_timer = create_timer
        native_pif.destroy_timer = destroy_timer

        ccl_init(settings.metadata, pm.metadata, &native_pif)
    }

}

class ConvivaClientSession {

    init() {
    }

}
