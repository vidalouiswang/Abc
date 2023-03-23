The class Element is the core of class MyDB and whole system, it allows you to store data in it. And you could use it for manu purpose.
The class ArrayBuffer is a tool that generate buffer using Element vector, or decode buffer to Element vector.
If you are a beginner, do not modify any codes. If you are master-hand of C++ and you have enhancement method, please submit issue or send email, thanks.
There are full comments in header file, check arraybuffer.h for more information.

类Element是数据库和整个系统的核心，他可以用来存储数据。你可以把它用作许多用途。
类ArrayBuffer是一个可以将Element容器转换为buffer的工具，或者可以用来将buffer转换为Element容器。
如果你是新手，请不要改动任何代码。如果你是C++高手觉得某些地方可以改进，可以提交issue或发邮件，谢谢。
头文件中有完整的注释，更多信息请参阅arraybuffer.h。

2023/03/23
Rebuilt class Element, now it supports all types of data.
Change file structure from .h and .cpp to single .hpp.
Almost all operators had been overloaded, it's very useful.
More usage refer to /test/test.cpp.


重构了类Element，现在它支持所有数据类型。
修改了文件结构，由.hpp替代.h和.cpp。
几乎所有的符号都已被重载，这很实用。
更多用法参见/test/test.cpp。