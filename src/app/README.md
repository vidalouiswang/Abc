### English

You could code from here, edit functions "setup" and "loop" in file "app.cpp" as arduino way.

The file /lib/mydb/mydb.h and mydb.cpp declared and defined 3 instances of class MyDB, they are:
    - db
    - dbUser
    - dbApp

The first one is main database for system use, you app could use this database to store data, but this is not recommended.

The second one stored other users information, you SHOULD NOT modify any content by your way in this database.

The last one is for user app, you could do any action to this instance.

### 中文

你可以从这里开始编写你的代码，编辑文件 "app.cpp" 中的 函数 "setup" 和 "loop"，和使用arduino的方式一样。

文件 /lib/mydb/mydb.h 和 mydb.cpp 中生命和定义了3个数据库实例，它们是：
    - db
    - dbUser
    - dbApp

第一个是主数据库，给系统用的，你的app可以使用这个数据库，但是不建议。

第二个存储了其他用户信息，你 **不** 应该对这个数据库用你的方式进行任何操作。

第三个是给你的app用的，你可以随意使用。