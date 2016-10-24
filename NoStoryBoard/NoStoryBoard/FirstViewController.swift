//
//  FirstViewController.swift
//  NoStoryBoard
//
//  Created by Hiroshi Watanabe on 10/23/16.
//  Copyright © 2016 Hiroshi Watanabe. All rights reserved.
//

import UIKit

class FirstViewController: UIViewController {

    override func viewDidLoad() {
        super.viewDidLoad()

        self.title = "First View"

        let buttonWidth: CGFloat = 100
        let buttonHeight: CGFloat = 50
        let posX: CGFloat = (self.view.bounds.width - buttonWidth) / 2
        let posY: CGFloat = (self.view.bounds.height - buttonHeight) / 2;

        let myButton = UIButton(frame: CGRect(x: posX, y: posY, width: buttonWidth, height: buttonHeight))
        myButton.backgroundColor = UIColor.red
        myButton.layer.masksToBounds = true
        myButton.layer.cornerRadius = 10.0
        myButton.setTitle("Click", for: .normal)
        myButton.addTarget(self, action: #selector(onClickMyButton(sender:)), for: .touchUpInside)

        self.view.addSubview(myButton);

        // Do any additional setup after loading the view.
    }

    override func didReceiveMemoryWarning() {
        super.didReceiveMemoryWarning()
        // Dispose of any resources that can be recreated.
    }


    internal func onClickMyButton(sender: UIButton){
        let secondViewController = SecondViewController()

        self.navigationController?.pushViewController(secondViewController, animated: true)
    }

}
