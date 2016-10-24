## NoStoryBoard

* Create a new project with Xcode

    * Xcode: Build to verify first

    * Emulator: Command+Shift+H => Home i.e. exit app

* Delete Main.storyboad and ViewController.swift

* Delete Main.stroyboard from Info.plist

* Add the code to application() in AppDelegate:

        window = UIWindow(frame: UIScreen.main.bounds)
        window!.backgroundColor = UIColor.white
        let myFirstViewController: FirstViewController = FirstViewController()
        let myNavigationController: UINavigationController = UINavigationController(rootViewController: myFirstViewController)
        window!.rootViewController = myNavigationController
        window!.makeKeyAndVisible()

* Add new viewcontrollers:

    * Open "New File..." by select the right button on the target project in Xcode
    * Add FirstViewController.swift

* Verify that UINavigationController shows FirstViewController


* Now, add SecondViewController.swift, also add the button to the FirstViewController
