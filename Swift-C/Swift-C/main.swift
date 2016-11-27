//
//  main.swift
//  Swift-C
//
//  Created by Hiroshi Watanabe on 10/28/16.
//  Copyright © 2016 Hiroshi Watanabe. All rights reserved.
//

import Foundation

////////////////////////////////////////////////////////////////////////////////
//
// C <-> Swift
//
////////////////////////////////////////////////////////////////////////////////


// print a string in the c side
let ss1: String = String.init(cString: UnsafePointer<CChar>(gstr))
print(ss1)

// print a string in the c side
let ss2: String = String.init(cString: UnsafePointer<CChar>(cgstr))
print(ss2)

let swiftstr = "Hi, all!"

// allocate the c struct
var c1 = CStruct()

// set the values
c1.i = 32
c1.str = strdup(swiftstr)

// print the values
print(c1.i)
print(c1.str) // print the binary
print(String.init(validatingUTF8: c1.str)!) // print in utf-8

// call in the c side
printstruct(&c1)

// free the string
free(c1.str)

// buffer size
let bufsize = MemoryLayout.size(ofValue: c1.buf)
print(bufsize)

// print str in swift
let ss3 = "swift string"
printswift(UnsafeMutablePointer(mutating: ss3))


////////////////////////////////////////////////////////////////////////////////
//
// http://www.swiftprogrammer.info/callback_void.html
//
////////////////////////////////////////////////////////////////////////////////

/** C code

typedef struct
{
    int32_t m_Int;
    int64_t m_Long;
    int16_t m_Array[3];
} APIStruct;

typedef void (*callback01)( void * );

int CUseCallback( callback01, int );

int CUseCallback(callback01 c, int n)
{
  APIStruct a;
  a.m_Int = 128;
  a.m_Long = 12;
  a.m_Array[0] = 100;
  c(&a);
  return 0;
}

 */


struct NaiveStruct
{
    var m_Int : Int32
    var m_Long : Int64
    var m_Array : (Int16, Int16, Int16)
}

func printNaive( s : NaiveStruct )
{
    print( "Printing NaiveStruct: " )
    print( "  m_Int: \(s.m_Int)" )
    print( "  m_Long: \(s.m_Long)" )
    print( "  m_Array: \(s.m_Array.0) \(s.m_Array.1) \(s.m_Array.2) " )
}

let NaiveCallback : callback01 = {( p : UnsafeMutableRawPointer? )->Void in
    print( "In NaiveCallback(), received a void pointer. " )
    let _ns : NaiveStruct = (UnsafeRawPointer(p)!.load(as: NaiveStruct.self))
    printNaive( s: _ns );
}

CUseCallback( NaiveCallback, 0 )

func printAPI( s : APIStruct )
{
    print( "Printing NaiveStruct: " )
    print( "  m_Int: \(s.m_Int)" )
    print( "  m_Long: \(s.m_Long)" )
    print( "  m_Array: \(s.m_Array.0) \(s.m_Array.1) \(s.m_Array.2) " )
}

let OneWayCallback : callback01 = {( p : UnsafeMutableRawPointer? )->Void in
    print( "In OneWayCallback(), received a void pointer. " )
    let _as : APIStruct = (UnsafeRawPointer(p)!.load(as: APIStruct.self))
    printAPI( s: _as );
}

CUseCallback( OneWayCallback, 0 )

struct WrapperStruct
{
    var m_Pointer : UnsafeMutablePointer<APIStruct>

    init( _p : UnsafeMutablePointer<APIStruct> )
    {
        m_Pointer = _p
    }

    var m_Int : Int32 {
        get { return m_Pointer.pointee.m_Int }
        set( val ) { m_Pointer.pointee.m_Int = val }
    }
    var m_Long : Int64 {
        get { return m_Pointer.pointee.m_Long }
        set( val ) { m_Pointer.pointee.m_Long = val }
    }
    var m_Array : (Int16, Int16, Int16)
    {
        get { return (m_Pointer.pointee.m_Array.0,
                      m_Pointer.pointee.m_Array.1,
                      m_Pointer.pointee.m_Array.2) }
        set ( val ) {
            m_Pointer.pointee.m_Array.0 = val.0
            m_Pointer.pointee.m_Array.1 = val.1
            m_Pointer.pointee.m_Array.2 = val.2
        }
    }
}

func printWrapper( s : WrapperStruct )
{
    print( "Printing APIStruct: " )
    print( "  m_Int: \(s.m_Int)" )
    print( "  m_Long: \(s.m_Long)" )
    print( "  m_Array: \(s.m_Array.0) \(s.m_Array.1) \(s.m_Array.2) " )
}

func TwoWayCallback( p : UnsafeMutableRawPointer? )->Void
{
    print( "In TwoWayCallback(), received a void pointer. " );
    var _wS : WrapperStruct = WrapperStruct(_p: (UnsafeMutableRawPointer(p)!.assumingMemoryBound(to: APIStruct.self)))
    printWrapper( s: _wS )
    print( "Setting m_Long in the structure to 98765432109 " )
    _wS.m_Long = 98765432109
    print( "Setting the array to 111, 222, 333" )
    _wS.m_Array.0 = 111
    _wS.m_Array.1 = 222
    _wS.m_Array.2 = 333
}

CUseCallback( TwoWayCallback, 1 )


////////////////////////////////////////////////////////////////////////////////
//
// passing string to c
//
////////////////////////////////////////////////////////////////////////////////


let StrCallback : callback02 = {( str : UnsafePointer<CChar>?, size: UInt32 ) -> Int32 in
    //let _as : APIStruct = (UnsafeRawPointer(p)!.load(as: APIStruct.self))
    //printAPI( s: _as );
    let s: String = String.init(cString: str!)

    print(s)
    print(size)

    return 0
}

CallStrCallback( StrCallback )


////////////////////////////////////////////////////////////////////////////////
//
// function pointer struct
//
////////////////////////////////////////////////////////////////////////////////


var cf1 = cfunc_struct();

func cf_cb1( str : UnsafePointer<CChar>?, size: UInt32 ) -> Int32
{
    let s: String = String.init(cString: str!)
    print(s)
    print(size)
    return 0
}

func cf_cb2( str : UnsafeMutablePointer<Int8>?, size: UInt32 ) -> Int32
{
    let s: String = String.init(cString: str!)
    print(s)
    print(size)
    return 0
}

func cf_cb3( f: func_callback?, data: UnsafeMutableRawPointer? ) -> Int32
{
    f!(data);
    return 0
}

struct MySwiftStruct
{
    var tako:Int32 = 32
}

var mp = MySwiftStruct()

func cf_cb4() -> UnsafeMutableRawPointer?
{
    print("func4 \(mp.tako)");
    let vp = withUnsafeMutablePointer(to: &mp) { (pp) -> UnsafeMutableRawPointer in
        return UnsafeMutableRawPointer(pp)
    }
    return UnsafeMutableRawPointer(vp)
}

func cf_cb5(ptr: UnsafeMutableRawPointer? ) -> Void
{
    if let pp = ptr {
        let p = UnsafeRawPointer(pp).load(as: MySwiftStruct.self)
        print("func5 \(p.tako)")
    }
}

cf1.func1 = cf_cb1
cf1.func2 = cf_cb2
cf1.func3 = cf_cb3
cf1.func4 = cf_cb4
cf1.func5 = cf_cb5

CallCFunc(&cf1)



////////////////////////////////////////////////////////////////////////////////
//
// timer function
//
////////////////////////////////////////////////////////////////////////////////


class TimerTest {
    var timer: Timer!

    var cb: func_callback?

    var data: UnsafeMutableRawPointer?

    init() {}

    init(cb: func_callback?, data: UnsafeMutableRawPointer?) {
        self.cb = cb
        self.data = data
    }

    func start() {
        timer = Timer.scheduledTimer(timeInterval: 1.0, target: self,
                                     selector: #selector(self.update), userInfo: nil, repeats: true)
        timer.fire()
    }

    func done() {
        timer.invalidate()
    }

    @objc func update(tm: Timer) {
        print("timer")
        if let f = cb {
            f(data!)
        }
    }
}

var ttt:TimerTest = TimerTest()

func create_timer(f: func_callback?, data: UnsafeMutableRawPointer?, delay: UInt32, interval: UInt32) -> UnsafeMutableRawPointer?
{
    ttt.cb = f
    ttt.data = data
    ttt.start()
    return Unmanaged.passRetained(ttt).toOpaque()
}

func destroy_timer(handle: UnsafeMutableRawPointer?) -> Void
{
    ttt.done()
}

var ft = cfunc_timer();
ft.create_timer = create_timer
ft.destroy_timer = destroy_timer
CreateTimerFunc(&ft)

CFRunLoopRun()
