## PodTest

> install ios libraries from cocoapods

### 1. Install Pod

* sudo gem update --system
* sudo gem install cocoapods
* pod setup

### 2. Create a new project if not

### 3. Create a Podfile in the project

```
target 'PodTest' do
  pod 'EasyMapping'
end

use_frameworks!
```

use_frameworks! is required.


### 4. Close xcode and Run 'pod install'

### 5. Open xxx.xcworkspace instead of xcodeproj

### 6. Run 'pod update' when adding a new library in Podfile


NOTE: .gitignore should ignore the Pods directories or not.
