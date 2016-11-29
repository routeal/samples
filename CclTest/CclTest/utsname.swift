import Foundation

public struct Utsname {
    public init() {}
    fileprivate var systemInfo: utsname {
        var result: utsname = utsname()
        uname(&result)
        return result
    }
    public var sysname: String {
        let mirror = Mirror(reflecting: self.systemInfo.sysname)
        return self._stringFromMirror(mirror)
    }
    public var nodename: String {
        let mirror = Mirror(reflecting: self.systemInfo.nodename)
        return self._stringFromMirror(mirror)
    }
    public var release: String {
        let mirror = Mirror(reflecting: self.systemInfo.release)
        return self._stringFromMirror(mirror)
    }
    public var version: String {
        let mirror = Mirror(reflecting: self.systemInfo.version)
        return self._stringFromMirror(mirror)
    }
    public var machine: String {
        let mirror = Mirror(reflecting: self.systemInfo.machine)
        return self._stringFromMirror(mirror)
    }
    fileprivate func _stringFromMirror(_ mirror: Mirror) -> String {
        var result = ""
        for child in mirror.children {
            if let value = child.value as? Int8 , value != 0 {
                result.append(String(UnicodeScalar(UInt8(value))))
            }
        }
        return result
    }
}
