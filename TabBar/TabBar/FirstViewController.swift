//
//  FirstViewController.swift
//  TabBar
//
//  Created by Hiroshi Watanabe on 10/23/16.
//  Copyright © 2016 Hiroshi Watanabe. All rights reserved.
//

import UIKit

class FirstViewController: UIViewController {

    init() {
        super.init(nibName: nil, bundle: nil)

        self.tabBarItem = UITabBarItem(tabBarSystemItem: .search, tag: 1)
    }

    required init?(coder aDecoder: NSCoder) {
        super.init(coder: aDecoder)
    }

    required override init(nibName nibNameOrNil: String!, bundle nibBundleOrNil: Bundle!) {
        super.init(nibName: nibNameOrNil, bundle: nibBundleOrNil)
    }

    override func viewDidLoad() {
        super.viewDidLoad()

        // Do any additional setup after loading the view.
    }

    override func didReceiveMemoryWarning() {
        super.didReceiveMemoryWarning()
        // Dispose of any resources that can be recreated.
    }
    

    /*
    // MARK: - Navigation

    // In a storyboard-based application, you will often want to do a little preparation before navigation
    override func prepare(for segue: UIStoryboardSegue, sender: Any?) {
        // Get the new view controller using segue.destinationViewController.
        // Pass the selected object to the new view controller.
    }
    */

}
