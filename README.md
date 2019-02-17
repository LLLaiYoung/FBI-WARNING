# FBI-WARNING

`FBI-WARNING`是一个使用`Clang`在编译的时候对`ObjC`代码进行静态分析的`.dylib`库。



目前已经支持以下一些通用的简单的静态代码检测规则：

#### @property

1. `NSString`和`Block`需要使用`copy`关键字修饰。
2. `delegate`需要使用`weak`关键字修饰。
3. 使用`atomic`原子性修饰时候的提醒。
4. `property`不应该使用大写开头命名。
5. `property`不应该使用内存管理语义命名。
6. 应该使用`@Property`方式定义属性。

#### Method

1. 方法名不应该使用大写开头命名。
2. 方法名不应该使用内存管理语义命名。
3. 方法参数名不应该使用大写开头命名。
4. 方法体超过 XX(50) 行的提醒。

#### Class

1. 类/分类名应该使用大写开头。
2. 类/分类名应该使用自定义大写前缀开头。
3. 类/分类名应该使用驼峰命名

#### Others

1. 应该使用`NS_ENUM/NS_OPTIONS`定义枚举，而不是使用`enum`。

未来也许会支持更多...

#### Custom

如果想要增加或者修改规则的小伙伴可以看看笔者写的另外两篇入门的文章（大牛请略过） [LLVM & Clang 入门](https://github.com/CYBoys/Blogs/blob/master/LLVM_Clang/LLVM%20%26%20Clang%20%E5%85%A5%E9%97%A8.md) 和 [Clang Plugin 之 Debug](https://github.com/CYBoys/Blogs/blob/master/LLVM_Clang/Clang%20Plugin%20%E4%B9%8B%20Debug.md) 这两篇文章讲了怎么搭建环境和使用 Xcode 进行调试。

当前代码中已经支持了可自定义过滤`Framework`或者文件，在`isVenderSourceCode()`函数中，将需要过滤的`Framework`或者`文件名`直接添加在`venders`中就可以了。

#### Notes

本仓库中会包含`FBI-WARNING`、`FBI-WARNING-Tooling`、`FWPluginDemo`和一个`.dylib`库。

**FBI-WARNING**：它会编译生成一个`.dylib`库，这个`.dylib`库会用于直接集成到 Xcode 中。集成部分请参考 [Xcode 集成 Plugin](https://github.com/CYBoys/Blogs/blob/master/LLVM_Clang/LLVM%20%26%20Clang%20%E5%85%A5%E9%97%A8.md#xcode_integrated_plugin)。

**FBI-WARNING-Tooling**：它是一个可直接使用适用于`Xcode`调试的代码。逻辑代码和`FBI-WARNING`是一样的（只是在加载方式和`CMakeLists`的配置文件上有一些差异），更准确的来说，`FBI-WARNING`是将`FBI-WARNING-Tooling`进行调试好的代码复制过去的。调试部分请参考 [Clang Plugin 之 Debug](https://github.com/CYBoys/Blogs/blob/master/LLVM_Clang/Clang%20Plugin%20%E4%B9%8B%20Debug.md)。

笔者也是刚开始学习，代码写得不够规范还请各位大佬见谅。其他自定义规则，请有需求的或者感兴趣的同学自己动动手。

欢迎对`clang`和`逆向`感兴趣的同学添加微信一起交流（添加时请注明）。

![](https://raw.githubusercontent.com/CYBoys/FBI-WARNING/master/Pic/WechatIMG39.jpeg)


#### Effects

![](https://raw.githubusercontent.com/CYBoys/FBI-WARNING/master/Pic/effect_0.png)

![](https://raw.githubusercontent.com/CYBoys/FBI-WARNING/master/Pic/effect_1.png)

#### Questions

Q: 更新了`.dylib`之后，在`Xcode`中重新运行怎么还是原来那样呀，没有任何变化？
A: 这时候需要对着对应的`Xcode`项目，按住`Shift`+`Command`+`K`，`Clean`一次就好了。

#### References

* [使用Xcode开发iOS语法检查的Clang插件](https://www.jianshu.com/p/581ef614a1c5)
* [CLANG技术分享系列二:代码风格检查(A CLANG PLUGIN APPROACH)](http://kangwang1988.github.io/tech/2016/10/31/check-code-style-using-clang-plugin.html)


如有内容错误，欢迎 [issue](https://github.com/CYBoys/FBI-WARNING/issues/new) 指正。

**转载请注明出处！**

